/* GstSwitchSrv
 * Copyright (C) 2012 Duzy Chan <code@duzy.info>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstworker.h"
#include "gstswitchserver.h"

#define GST_WORKER_LOCK_PIPELINE(srv) (g_mutex_lock (&(srv)->pipeline_lock))
#define GST_WORKER_UNLOCK_PIPELINE(srv) (g_mutex_unlock (&(srv)->pipeline_lock))

enum
{
  PROP_0,
  PROP_NAME,
};

enum
{
  SIGNAL_PREPARE_WORKER,
  SIGNAL_START_WORKER,
  SIGNAL_END_WORKER,
  SIGNAL__LAST,
};

static guint gst_worker_signals[SIGNAL__LAST] = { 0 };
extern gboolean verbose;

#define gst_worker_parent_class parent_class
G_DEFINE_TYPE (GstWorker, gst_worker, G_TYPE_OBJECT);

static void
gst_worker_init (GstWorker *worker)
{
  worker->name = NULL;
  worker->server = NULL;
  worker->bus = NULL;
  worker->pipeline = NULL;
  worker->pipeline_func = NULL;
  worker->pipeline_func_data = NULL;
  worker->pipeline_string = NULL;
  worker->paused_for_buffering = FALSE;
  worker->watch = 0;

  g_mutex_init (&worker->pipeline_lock);

  //INFO ("init (%p)", worker);
}

static void
gst_worker_dispose (GstWorker *worker)
{
  if (worker->server) {
    g_object_unref (worker->server);
    worker->server = NULL;
  }

  if (worker->watch) {
    g_source_remove (worker->watch);
  }

  if (worker->bus) {
    gst_object_unref (worker->bus);
    worker->bus = NULL;
  }

  if (worker->pipeline) {
    gst_element_set_state (worker->pipeline, GST_STATE_NULL);
    gst_object_unref (worker->pipeline);
    worker->pipeline = NULL;
  }

  if (worker->pipeline_string) {
    g_string_free (worker->pipeline_string, FALSE);
    worker->pipeline_string = NULL;
  }

  //INFO ("dispose %p", worker);
  G_OBJECT_CLASS (parent_class)->dispose (G_OBJECT (worker));
}

static void
gst_worker_finalize (GstWorker *worker)
{
  g_mutex_clear (&worker->pipeline_lock);

  g_free (worker->name);
  worker->name = NULL;

  if (G_OBJECT_CLASS (parent_class)->finalize)
    (*G_OBJECT_CLASS (parent_class)->finalize) (G_OBJECT (worker));
}

static void
gst_worker_set_property (GstWorker *worker, guint property_id,
    const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  case PROP_NAME: {
    if (worker->name) g_free (worker->name);
    worker->name = g_strdup (g_value_get_string (value));
    break;
  }
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (worker), property_id, pspec);
    break;
  }
}

static void
gst_worker_get_property (GstWorker *worker, guint property_id,
    GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  case PROP_NAME: {
    worker->name = g_strdup (g_value_get_string (value));
    break;
  }
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (worker), property_id, pspec);
    break;
  }
}

static GString *
gst_worker_get_pipeline_string (GstWorker *worker)
{
  GString *desc = NULL;
  if (worker->pipeline_func)
    desc = worker->pipeline_func (worker, worker->pipeline_func_data);
  if (!desc && worker->pipeline_string)
    desc = g_string_new (worker->pipeline_string->str);
  if (!desc)
    desc = g_string_new ("");
  return desc;
}

static GstElement *
gst_worker_create_pipeline (GstWorker *worker)
{
  GstWorkerClass *workerclass = GST_WORKER_CLASS (
      G_OBJECT_GET_CLASS (worker));

  GString *desc = NULL;
  GstElement *pipeline = NULL;
  GError *error = NULL;
  GstParseContext *context = NULL;

 create_pipeline:
  desc = workerclass->get_pipeline_string (worker);
  context = gst_parse_context_new ();

  if (verbose) {
    g_print ("%s: %s\n", worker->name, desc->str);
  }

  pipeline = (GstElement *) gst_parse_launch_full (desc->str, context,
      GST_PARSE_FLAG_NONE, &error);
  g_string_free (desc, FALSE);

  if (error == NULL) {
    goto end;
  }

  if (g_error_matches (error, GST_PARSE_ERROR,
	  GST_PARSE_ERROR_NO_SUCH_ELEMENT)) {
    gchar **name = NULL;
    gchar **names = gst_parse_context_get_missing_elements (context);
    gboolean retry = workerclass->missing &&
      (*workerclass->missing) (worker, names);
    for (name = names; *name; ++name) ERROR ("missing: %s", *name);
    g_strfreev (names);

    if (retry) {
      gst_parse_context_free (context);
      gst_object_unref (pipeline);
      pipeline = NULL;
      context = NULL;
      goto create_pipeline;
    }
  } else {
    ERROR ("%s: pipeline parsing error: %s", worker->name, error->message);
  }

  if (pipeline) {
    gst_object_unref (pipeline);
    pipeline = NULL;
  }

 end:
  gst_parse_context_free (context);
  return pipeline;
}

static GstWorkerNullReturn
gst_worker_null (GstWorker *worker)
{
  return GST_WORKER_NR_END;
}

static gboolean gst_worker_prepare (GstWorker *);

gboolean
gst_worker_start (GstWorker *worker)
{
  GstStateChangeReturn ret = GST_STATE_CHANGE_FAILURE;

  g_return_val_if_fail (GST_IS_WORKER (worker), FALSE);

  if (gst_worker_prepare (worker)) {
    GST_WORKER_LOCK_PIPELINE (worker);
    ret = gst_element_set_state (worker->pipeline, GST_STATE_READY);
    GST_WORKER_UNLOCK_PIPELINE (worker);
  }

  return ret == GST_STATE_CHANGE_SUCCESS ? TRUE : FALSE;
}

static gboolean
gst_worker_replay (GstWorker *worker)
{
  GstStateChangeReturn ret = GST_STATE_CHANGE_FAILURE;

  g_return_val_if_fail (GST_IS_WORKER (worker), FALSE);

  GST_WORKER_LOCK_PIPELINE (worker);
 
  if (worker->pipeline) {
    GstState state;

    ret = gst_element_get_state (worker->pipeline, &state, NULL,
	GST_CLOCK_TIME_NONE);

    if (state != GST_STATE_PLAYING) {
      ret = gst_element_set_state (worker->pipeline, GST_STATE_READY);
    }
  }

  GST_WORKER_UNLOCK_PIPELINE (worker);

  return ret == GST_STATE_CHANGE_SUCCESS ? TRUE : FALSE;
}

gboolean
gst_worker_stop_force (GstWorker *worker, gboolean force)
{
  GstStateChangeReturn ret = GST_STATE_CHANGE_FAILURE;

  g_return_val_if_fail (GST_IS_WORKER (worker), FALSE);

  GST_WORKER_LOCK_PIPELINE (worker);

  if (worker->pipeline) {
#if 1
    GstState state;

    ret = gst_element_get_state (worker->pipeline, &state, NULL,
	GST_CLOCK_TIME_NONE);

    if (state == GST_STATE_PLAYING || force) {
      ret = gst_element_set_state (worker->pipeline, GST_STATE_NULL);
    }
#else
    ret = gst_element_set_state (worker->pipeline, GST_STATE_NULL);
#endif
  }

  GST_WORKER_UNLOCK_PIPELINE (worker);

  return ret == GST_STATE_CHANGE_SUCCESS ? TRUE : FALSE;
}

GstElement *
gst_worker_get_element_unsafe (GstWorker *worker, const gchar *name)
{
  g_return_val_if_fail (GST_IS_WORKER (worker), NULL);

  return gst_bin_get_by_name (GST_BIN (worker->pipeline), name);
}

GstElement *
gst_worker_get_element (GstWorker *worker, const gchar *name)
{
  GstElement *element = NULL;

  GST_WORKER_LOCK_PIPELINE (worker);
  element = gst_worker_get_element_unsafe (worker, name);
  GST_WORKER_UNLOCK_PIPELINE (worker);
  return element;
}

/*
static void
gst_worker_missing_plugin (GstWorker *worker, GstStructure *structure)
{
  GstWorkerClass *workerclass = GST_WORKER_CLASS (
      G_OBJECT_GET_CLASS (worker));

  ERROR ("missing plugin");

  if (workerclass->missing_plugin)
    (*workerclass->missing_plugin) (worker, structure);
}
*/

static void
gst_worker_handle_eos (GstWorker *worker)
{
  gst_worker_stop (worker);
}

static void
gst_worker_handle_error (GstWorker *worker, GError * error,
    const char *debug)
{
  //ERROR ("%s: %s", worker->name, error->message);

  if (error->domain == GST_CORE_ERROR) {
    ERROR ("%s: (CORE: %d) %s", worker->name, error->code, error->message);
    switch (error->code) {
    case GST_CORE_ERROR_MISSING_PLUGIN:
      ERROR ("missing plugin..");
      break;
    case GST_CORE_ERROR_NEGOTIATION:
      ERROR ("%s: negotiation: %s", worker->name, error->message);
      break;
    }
  }

  if (error->domain == GST_LIBRARY_ERROR) {
    ERROR ("%s: (LIBRARY: %d) %s", worker->name, error->code, error->message);
  }

  if (error->domain == GST_RESOURCE_ERROR) {
    ERROR ("%s: (RESOURCE: %d) %s", worker->name, error->code, error->message);
  }

  if (error->domain == GST_STREAM_ERROR) {
    ERROR ("%s: (STREAM: %d) %s", worker->name, error->code, error->message);
  }

#if 0
  ERROR ("DEBUG INFO:\n%s\n", debug);
#endif

#if 0
  gst_worker_stop (worker);
#endif
}

static void
gst_worker_handle_warning (GstWorker *worker, GError * error,
    const char *debug)
{
  WARN ("%s: %s", worker->name, error->message);
}

static void
gst_worker_handle_info (GstWorker *worker, GError * error,
    const char *debug)
{
  INFO ("%s: %s", worker->name, error->message);
}

static void
gst_worker_state_null_to_ready (GstWorker *worker)
{
  gst_element_set_state (worker->pipeline, GST_STATE_PAUSED);
}

static void
gst_worker_state_ready_to_paused (GstWorker *worker)
{
  if (!worker->paused_for_buffering) {
    gst_element_set_state (worker->pipeline, GST_STATE_PLAYING);
  }
}

static void
gst_worker_state_paused_to_playing (GstWorker *worker)
{
  GstWorkerClass *workerclass;

  g_return_if_fail (GST_IS_WORKER (worker));

  workerclass = GST_WORKER_CLASS (G_OBJECT_GET_CLASS (worker));
  if (workerclass->alive) {
    (*workerclass->alive) (worker);
  }

  g_signal_emit (worker, gst_worker_signals[SIGNAL_START_WORKER], 0);
}

static void
gst_worker_state_playing_to_paused (GstWorker *worker)
{
}

static void
gst_worker_state_paused_to_ready (GstWorker *worker)
{
}

static void
gst_worker_state_ready_to_null (GstWorker *worker)
{
  GstWorkerClass *workerclass;
  GstWorkerNullReturn ret = GST_WORKER_NR_END;

  g_return_if_fail (GST_IS_WORKER (worker));

  workerclass = GST_WORKER_CLASS (G_OBJECT_GET_CLASS (worker));
  if (workerclass->null) {
    switch ((ret = (*workerclass->null) (worker))) {
    case GST_WORKER_NR_REPLAY:
      gst_worker_replay (worker);
      break;
    case GST_WORKER_NR_END:
      break;
    }
  }

  if (ret == GST_WORKER_NR_END)
    g_signal_emit (worker, gst_worker_signals[SIGNAL_END_WORKER], 0);
}

static gboolean
gst_worker_pipeline_state_changed (GstWorker *worker,
    GstStateChange statechange)
{
  switch (statechange) {
  case GST_STATE_CHANGE_NULL_TO_READY:
    gst_worker_state_null_to_ready (worker);
    break;
  case GST_STATE_CHANGE_READY_TO_PAUSED:
    gst_worker_state_ready_to_paused (worker);
    break;
  case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
    gst_worker_state_paused_to_playing (worker);
    break;
  case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
    gst_worker_state_playing_to_paused (worker);
    break;
  case GST_STATE_CHANGE_PAUSED_TO_READY:
    gst_worker_state_paused_to_ready (worker);
    break;
  case GST_STATE_CHANGE_READY_TO_NULL:
    gst_worker_state_ready_to_null (worker);
    break;
  default:
    return FALSE;
  }
  return TRUE;
}

static gboolean
gst_worker_message (GstBus * bus, GstMessage * message, GstWorker *worker)
{
  GstWorkerClass *workerclass;

  /* The event source should be removed if not worker ! */
  g_return_val_if_fail (GST_IS_WORKER (worker), FALSE);

  workerclass = GST_WORKER_CLASS (G_OBJECT_GET_CLASS (worker));

  switch (GST_MESSAGE_TYPE (message)) {
  case GST_MESSAGE_EOS:
    gst_worker_handle_eos (worker);
    break;
  case GST_MESSAGE_ERROR:
  {
    GError *error = NULL;
    gchar *debug;
    gst_message_parse_error (message, &error, &debug);
    gst_worker_handle_error (worker, error, debug);
  } break;
  case GST_MESSAGE_WARNING:
  {
    GError *error = NULL;
    gchar *debug;
    gst_message_parse_warning (message, &error, &debug);
    gst_worker_handle_warning (worker, error, debug);
  } break;
  case GST_MESSAGE_INFO:
  {
    GError *error = NULL;
    gchar *debug;
    gst_message_parse_info (message, &error, &debug);
    gst_worker_handle_info (worker, error, debug);
  } break;
  case GST_MESSAGE_TAG:
  {
    /*
    GstTagList *tag_list;

    gst_message_parse_tag (message, &tag_list);

    //if (verbose)
    //  g_print ("tag\n");

    gst_tag_list_unref (tag_list);
    */
  } break;
  case GST_MESSAGE_STATE_CHANGED:
  {
    gboolean ret;
    GstState oldstate, newstate, pending;
    gst_message_parse_state_changed (message, &oldstate, &newstate, &pending);
    if (GST_ELEMENT (message->src) == worker->pipeline) {
      /*
      if (verbose) {
	g_print ("%s: state change from %s to %s\n", worker->name,
	    gst_element_state_get_name (oldstate),
	    gst_element_state_get_name (newstate));
      }
      */

      ret = gst_worker_pipeline_state_changed (worker,
	  GST_STATE_TRANSITION (oldstate, newstate));

      if (!ret /*&& verbose*/) {
	WARN ("%s: UNKNOWN state change from %s to %s\n", worker->name,
	    gst_element_state_get_name (oldstate),
	    gst_element_state_get_name (newstate));
      }
    }
  } break;
  case GST_MESSAGE_BUFFERING:
  {
    int percent;
    gst_message_parse_buffering (message, &percent);
    //g_print("buffering %d\n", percent);
    if (!worker->paused_for_buffering && percent < 100) {
      g_print ("pausing for buffing\n");
      worker->paused_for_buffering = TRUE;
      gst_element_set_state (worker->pipeline, GST_STATE_PAUSED);
    } else if (worker->paused_for_buffering && percent == 100) {
      g_print ("unpausing for buffing\n");
      worker->paused_for_buffering = FALSE;
      gst_element_set_state (worker->pipeline, GST_STATE_PLAYING);
    }
  } break;
  case GST_MESSAGE_ELEMENT:
  case GST_MESSAGE_STATE_DIRTY:
  case GST_MESSAGE_CLOCK_PROVIDE:
  case GST_MESSAGE_CLOCK_LOST:
  case GST_MESSAGE_NEW_CLOCK:
  case GST_MESSAGE_STRUCTURE_CHANGE:
  case GST_MESSAGE_STREAM_STATUS:
    break;
  case GST_MESSAGE_STEP_DONE:
  case GST_MESSAGE_APPLICATION:
  case GST_MESSAGE_SEGMENT_START:
  case GST_MESSAGE_SEGMENT_DONE:
  case GST_MESSAGE_LATENCY:
  case GST_MESSAGE_ASYNC_START:
  case GST_MESSAGE_ASYNC_DONE:
  case GST_MESSAGE_REQUEST_STATE:
  case GST_MESSAGE_STEP_START:
  case GST_MESSAGE_QOS:
  default:
    if (verbose) {
      //g_print ("message: %s\n", GST_MESSAGE_TYPE_NAME (message));
    }
    break;
  }

  return workerclass->message ?
    workerclass->message (worker, message) : TRUE;
}

static gboolean
gst_worker_prepare_unsafe (GstWorker *worker)
{
  GstWorkerClass *workerclass;

  g_return_val_if_fail (worker, FALSE);

  workerclass = GST_WORKER_CLASS (G_OBJECT_GET_CLASS (worker));
  if (!workerclass->create_pipeline)
    goto error_create_pipeline_not_installed;

  //GST_WORKER_LOCK_PIPELINE (worker);
  if (worker->pipeline)
    goto end;

  worker->pipeline = workerclass->create_pipeline (worker);
  if (!worker->pipeline)
    goto error_create_pipeline;

  gst_pipeline_set_auto_flush_bus (GST_PIPELINE (worker->pipeline), FALSE);

  worker->bus = gst_pipeline_get_bus (GST_PIPELINE (worker->pipeline));
  if (!worker->bus)
    goto error_get_bus;

  worker->watch = gst_bus_add_watch (worker->bus,
      (GstBusFunc) gst_worker_message, worker);
  if (!worker->watch)
    goto error_add_watch;

  if (workerclass->prepare && !workerclass->prepare (worker))
    goto error_prepare;

  g_signal_emit (worker, gst_worker_signals[SIGNAL_PREPARE_WORKER], 0);

 end:
  //GST_WORKER_UNLOCK_PIPELINE (worker);
  return TRUE;

  /* Errors Handling */

 error_create_pipeline_not_installed:
  {
    ERROR ("%s: create_pipeline was not installed", worker->name);
    return FALSE;
  }

 error_create_pipeline:
  {
    ERROR ("%s: failed to create new pipeline", worker->name);
    //GST_WORKER_UNLOCK_PIPELINE (worker);
    return FALSE;
  }

 error_prepare:
  {
    g_source_remove (worker->watch);
  error_add_watch:
    gst_object_unref (worker->bus);
  error_get_bus:
    gst_object_unref (worker->pipeline);
    worker->pipeline = NULL;
    worker->bus = NULL;
    worker->watch = 0;
    ERROR ("%s: failed to prepare", worker->name);
    //GST_WORKER_UNLOCK_PIPELINE (worker);
    return FALSE;
  }
}

static gboolean
gst_worker_prepare (GstWorker *worker)
{
  gboolean ok = FALSE;
  if (worker->pipeline == NULL) {
    GST_WORKER_LOCK_PIPELINE (worker);
    ok = gst_worker_prepare_unsafe (worker);
    GST_WORKER_UNLOCK_PIPELINE (worker);
  } else {
    ok = TRUE;
  }
  return ok;
}

static gboolean
gst_worker_reset (GstWorker *worker)
{
  gboolean ok = FALSE;

  g_return_val_if_fail (GST_IS_WORKER (worker), FALSE);

#if 1
  if (worker->pipeline) {
    GST_WORKER_LOCK_PIPELINE (worker);
    if (worker->pipeline) {
      if (worker->watch) {
	g_source_remove (worker->watch);
      }
      if (worker->bus) {
	gst_object_unref (worker->bus);
      }
      if (worker->pipeline) {
	gst_element_set_state (worker->pipeline, GST_STATE_NULL);
	gst_object_unref (worker->pipeline);
      }
      worker->pipeline = NULL;
      worker->bus = NULL;
      worker->watch = 0;

      ok = gst_worker_prepare_unsafe (worker);
    }
    GST_WORKER_UNLOCK_PIPELINE (worker);
  }
#else
  ok = TRUE;
#endif

  return ok;
}

static void
gst_worker_class_init (GstWorkerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = (GObjectFinalizeFunc) gst_worker_dispose;
  object_class->finalize = (GObjectFinalizeFunc) gst_worker_finalize;
  object_class->set_property = (GObjectSetPropertyFunc) gst_worker_set_property;
  object_class->get_property = (GObjectGetPropertyFunc) gst_worker_get_property;

  gst_worker_signals[SIGNAL_PREPARE_WORKER] =
    g_signal_new ("prepare-worker", G_TYPE_FROM_CLASS (klass),
	G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (GstWorkerClass, prepare_worker),
	NULL, NULL, g_cclosure_marshal_generic, G_TYPE_NONE, 0);

  gst_worker_signals[SIGNAL_START_WORKER] =
    g_signal_new ("start-worker", G_TYPE_FROM_CLASS (klass),
	G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (GstWorkerClass, start_worker),
	NULL, NULL, g_cclosure_marshal_generic, G_TYPE_NONE, 0);

  gst_worker_signals[SIGNAL_END_WORKER] =
    g_signal_new ("end-worker", G_TYPE_FROM_CLASS (klass),
	G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (GstWorkerClass, end_worker),
	NULL, NULL, g_cclosure_marshal_generic, G_TYPE_NONE, 0);

  g_object_class_install_property (object_class, PROP_NAME,
      g_param_spec_string ("name", "Name", "Name of the case",
          "", G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  klass->get_pipeline_string = gst_worker_get_pipeline_string;
  klass->create_pipeline = gst_worker_create_pipeline;
  klass->null = gst_worker_null;
  klass->reset = gst_worker_reset;
}
