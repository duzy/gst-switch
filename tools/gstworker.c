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

enum
{
  PROP_0,
  PROP_NAME,
};

extern gboolean verbose;

G_DEFINE_TYPE (GstWorker, gst_worker, G_TYPE_OBJECT);

static void
gst_worker_init (GstWorker *worker)
{
  worker->name = NULL;
  worker->server = NULL;
  worker->bus = NULL;
  worker->pipeline = NULL;
  worker->source = NULL;
  worker->sink = NULL;
  worker->paused_for_buffering = FALSE;
  worker->timer_id = -1;
}

static void
gst_worker_finalize (GstWorker *worker)
{
  g_free (worker->name);
  worker->name = NULL;

  if (worker->server) {
    g_object_unref (worker->server);
    worker->server = NULL;
  }

  if (worker->source) {
    gst_object_unref (worker->source);
    worker->source = NULL;
  }

  if (worker->sink) {
    gst_object_unref (worker->sink);
    worker->sink = NULL;
  }

  if (worker->pipeline) {
    gst_element_set_state (worker->pipeline, GST_STATE_NULL);
    gst_object_unref (worker->pipeline);
    worker->pipeline = NULL;
  }

  INFO ("Worker finalized");
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

static void
gst_worker_class_init (GstWorkerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = (GObjectFinalizeFunc) gst_worker_finalize;
  object_class->set_property = (GObjectSetPropertyFunc) gst_worker_set_property;
  object_class->get_property = (GObjectGetPropertyFunc) gst_worker_get_property;

  g_object_class_install_property (object_class, PROP_NAME,
      g_param_spec_string ("name", "Name", "Name of the case",
          "", G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  INFO ("Worker initialized");
}

static gboolean gst_worker_onesecond_timer (gpointer priv);

void
gst_worker_start (GstWorker *worker)
{
  gst_element_set_state (worker->pipeline, GST_STATE_READY);

  worker->timer_id = g_timeout_add (1000, gst_worker_onesecond_timer, worker);
}

void
gst_worker_stop (GstWorker *worker)
{
  gst_element_set_state (worker->pipeline, GST_STATE_NULL);

  g_source_remove (worker->timer_id);
}

static void
gst_worker_handle_eos (GstWorker *worker)
{
  gst_worker_stop (worker);
}

static void
gst_worker_handle_error (GstWorker *worker, GError * error,
    const char *debug)
{
  ERROR ("%s: %s", worker->name, error->message);
  gst_worker_stop (worker);
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
gst_worker_handle_null_to_ready (GstWorker *worker)
{
  gst_element_set_state (worker->pipeline, GST_STATE_PAUSED);
}

static void
gst_worker_handle_ready_to_paused (GstWorker *worker)
{
  if (!worker->paused_for_buffering) {
    gst_element_set_state (worker->pipeline, GST_STATE_PLAYING);
  }
}

static void
gst_worker_handle_paused_to_playing (GstWorker *worker)
{
}

static void
gst_worker_handle_playing_to_paused (GstWorker *worker)
{
}

static void
gst_worker_handle_paused_to_ready (GstWorker *worker)
{
}

static void
gst_worker_handle_ready_to_null (GstWorker *worker)
{
  GstWorkerClass *workerclass = GST_WORKER_CLASS (
      G_OBJECT_GET_CLASS (worker));

  if (workerclass->null_state)
    (*workerclass->null_state) (worker);
}

static gboolean
gst_worker_handle_pipeline_state_changed (GstWorker *worker,
    GstStateChange statechange)
{
  switch (statechange) {
  case GST_STATE_CHANGE_NULL_TO_READY:
    gst_worker_handle_null_to_ready (worker);
    break;
  case GST_STATE_CHANGE_READY_TO_PAUSED:
    gst_worker_handle_ready_to_paused (worker);
    break;
  case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
    gst_worker_handle_paused_to_playing (worker);
    break;
  case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
    gst_worker_handle_playing_to_paused (worker);
    break;
  case GST_STATE_CHANGE_PAUSED_TO_READY:
    gst_worker_handle_paused_to_ready (worker);
    break;
  case GST_STATE_CHANGE_READY_TO_NULL:
    gst_worker_handle_ready_to_null (worker);
    break;
  default:
    return FALSE;
  }
  return TRUE;
}

static gboolean
gst_worker_handle_message (GstBus * bus, GstMessage * message, gpointer data)
{
  GstWorker *worker = GST_WORKER (data);

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
      GstTagList *tag_list;

      gst_message_parse_tag (message, &tag_list);
      if (verbose)
        g_print ("tag\n");
    } break;
    case GST_MESSAGE_STATE_CHANGED:
    {
      gboolean ret;
      GstState oldstate, newstate, pending;
      gst_message_parse_state_changed (message, &oldstate, &newstate, &pending);
      if (GST_ELEMENT (message->src) == worker->pipeline) {
        if (verbose)
          g_print ("%s: state change from %s to %s\n", worker->name,
              gst_element_state_get_name (oldstate),
              gst_element_state_get_name (newstate));

	ret = gst_worker_handle_pipeline_state_changed (worker,
	    GST_STATE_TRANSITION (oldstate, newstate));

	if (!ret && verbose)
	  g_print ("unknown state change from %s to %s\n",
	      gst_element_state_get_name (oldstate),
	      gst_element_state_get_name (newstate));
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
    case GST_MESSAGE_STATE_DIRTY:
    case GST_MESSAGE_CLOCK_PROVIDE:
    case GST_MESSAGE_CLOCK_LOST:
    case GST_MESSAGE_NEW_CLOCK:
    case GST_MESSAGE_STRUCTURE_CHANGE:
    case GST_MESSAGE_STREAM_STATUS:
      break;
    case GST_MESSAGE_STEP_DONE:
    case GST_MESSAGE_APPLICATION:
    case GST_MESSAGE_ELEMENT:
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

  return TRUE;
}

static gboolean
gst_worker_onesecond_timer (gpointer priv)
{
  GstWorker *worker = (GstWorker *) priv;

  (void) worker;

  return TRUE;
}

gboolean
gst_worker_prepare (GstWorker *worker)
{
  GstWorkerClass *workerclass = GST_WORKER_CLASS (
      G_OBJECT_GET_CLASS (worker));

  if (worker->pipeline) {
    return TRUE;
  }

  if (!workerclass->create_pipeline)
    goto error_create_pipeline_not_installed;

  worker->pipeline = workerclass->create_pipeline (worker);
  if (!worker->pipeline)
    goto error_create_pipeline;

  gst_pipeline_set_auto_flush_bus (GST_PIPELINE (worker->pipeline), FALSE);

  worker->bus = gst_pipeline_get_bus (GST_PIPELINE (worker->pipeline));
  gst_bus_add_watch (worker->bus, gst_worker_handle_message, worker);

  worker->source = gst_bin_get_by_name (GST_BIN (worker->pipeline), "source");
  worker->sink = gst_bin_get_by_name (GST_BIN (worker->pipeline), "sink");

  if (workerclass->prepare) {
    if (!workerclass->prepare (worker)) {
      WARN ("failed to prepare worker");
      return FALSE;
    }
  }

  return TRUE;

  /* Errors Handling */

 error_create_pipeline_not_installed:
  {
    ERROR ("create_pipeline was not installed");
    return FALSE;
  }

 error_create_pipeline:
  {
    ERROR ("failed to create new pipeline");
    return FALSE;
  }
}
