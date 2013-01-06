/* GstSwitch
 * Copyright (C) 2013 Duzy Chan <code@duzy.info>
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

#include <stdlib.h>
#include <string.h>
#include "gstswitchserver.h"
#include "gstrecorder.h"

enum
{
  PROP_0,
  PROP_PORT,
};

enum
{
  SIGNAL_END_RECORDER,
  SIGNAL__LAST,
};

static guint gst_recorder_signals[SIGNAL__LAST] = { 0 };
extern gboolean verbose;

#define parent_class gst_recorder_parent_class

G_DEFINE_TYPE (GstRecorder, gst_recorder, GST_TYPE_WORKER);

static void
gst_recorder_init (GstRecorder * rec)
{
  INFO ("Recorder initialized");
}

static void
gst_recorder_finalize (GstRecorder * rec)
{
  if (G_OBJECT_CLASS (parent_class)->finalize)
    (*G_OBJECT_CLASS (parent_class)->finalize) (G_OBJECT (rec));

  INFO ("Recorder finalized");
}

static void
gst_recorder_get_property (GstRecorder *rec, guint property_id,
    GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  case PROP_PORT:
    g_value_set_uint (value, rec->sink_port);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (rec, property_id, pspec);
    break;
  }
}

static void
gst_recorder_set_property (GstRecorder *rec, guint property_id,
    const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  case PROP_PORT:
    rec->sink_port = g_value_get_uint (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (rec), property_id, pspec);
    break;
  }
}

static GstElement *
gst_recorder_create_pipeline (GstRecorder * rec)
{
  GstElement *pipeline;
  GError *error = NULL;
  GString *desc;

  desc = g_string_new ("");

  g_string_append_printf (desc, "intervideosrc name=source_video "
      "channel=composite_video ");
  g_string_append_printf (desc, "interaudiosrc name=source_audio "
      "channel=composite_audio ");
  g_string_append_printf (desc, "source_video. ! queue ! autovideosink ");
  g_string_append_printf (desc, "source_audio. ! queue ! autoaudiosink ");

  if (verbose)
    g_print ("pipeline: %s\n", desc->str);

  pipeline = (GstElement *) gst_parse_launch (desc->str, &error);
  g_string_free (desc, FALSE);

  if (error) {
    ERROR ("pipeline parsing error: %s", error->message);
    gst_object_unref (pipeline);
    return NULL;
  }

  return pipeline;
}

static gboolean
gst_recorder_prepare (GstRecorder *rec)
{
  GstWorker *worker = GST_WORKER (rec);

  (void) worker;

  return TRUE;
}

static void
gst_recorder_null (GstRecorder *rec)
{
  GstWorker *worker = GST_WORKER (rec);

  INFO ("null: %s (%p)", worker->name, rec);

  g_signal_emit (rec, gst_recorder_signals[SIGNAL_END_RECORDER], 0);
}

static void
gst_recorder_class_init (GstRecorderClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstWorkerClass *worker_class = GST_WORKER_CLASS (klass);

  object_class->finalize = (GObjectFinalizeFunc) gst_recorder_finalize;
  object_class->set_property = (GObjectSetPropertyFunc) gst_recorder_set_property;
  object_class->get_property = (GObjectGetPropertyFunc) gst_recorder_get_property;

  gst_recorder_signals[SIGNAL_END_RECORDER] =
    g_signal_new ("end-recorder", G_TYPE_FROM_CLASS (klass),
	G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (GstRecorderClass, end_recorder),
	NULL, NULL, g_cclosure_marshal_generic, G_TYPE_NONE, 0);

  g_object_class_install_property (object_class, PROP_PORT,
      g_param_spec_uint ("port", "Port", "Sink port",
          GST_SWITCH_MIN_SINK_PORT, GST_SWITCH_MAX_SINK_PORT,
	  GST_SWITCH_MIN_SINK_PORT,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  worker_class->null_state = (GstWorkerNullStateFunc) gst_recorder_null;
  worker_class->prepare = (GstWorkerPrepareFunc) gst_recorder_prepare;
  worker_class->create_pipeline = (GstWorkerCreatePipelineFunc)
    gst_recorder_create_pipeline;
}
