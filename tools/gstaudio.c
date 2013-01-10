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

#include "gstaudio.h"
#include "gstswitchserver.h"
#include <gst/video/videooverlay.h>

#define parent_class gst_audio_parent_class

enum
{
  PROP_0,
  PROP_PORT,
  PROP_HANDLE,
};

extern gboolean verbose;

G_DEFINE_TYPE (GstAudio, gst_audio, GST_TYPE_WORKER);

static void
gst_audio_init (GstAudio *audio)
{
  INFO ("Audio play initialized");
}

static void
gst_audio_finalize (GstAudio *audio)
{
  if (G_OBJECT_CLASS (parent_class)->finalize)
    (*G_OBJECT_CLASS (parent_class)->finalize) (G_OBJECT (audio));

  INFO ("Audia play finalized");
}

static void
gst_audio_set_property (GstAudio *audio, guint property_id,
    const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  case PROP_PORT:
    audio->port = g_value_get_uint (value);
    break;
  case PROP_HANDLE:
    audio->handle = g_value_get_ulong (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (audio), property_id, pspec);
    break;
  }
}

static void
gst_audio_get_property (GstAudio *audio, guint property_id,
    GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  case PROP_PORT:
    g_value_set_uint (value, audio->port);
    break;
  case PROP_HANDLE:
    g_value_set_ulong (value, audio->handle);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (audio), property_id, pspec);
    break;
  }
}

static GstElement *
gst_audio_create_pipeline (GstAudio *audio)
{
  GstElement *pipeline;
  GError *error = NULL;
  GString *desc;

  INFO ("play audio %d", audio->port);

  desc = g_string_new ("");

  g_string_append_printf (desc, "tcpclientsrc name=source "
      "port=%d ", audio->port);
  g_string_append_printf (desc, "! gdpdepay ");
  g_string_append_printf (desc, "! audioconvert ");
  g_string_append_printf (desc, "! autoaudiosink name=sink ");

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

static void
gst_audio_null (GstAudio *audio)
{
}

static void
gst_audio_prepare (GstAudio *audio)
{
  GstWorker *worker = GST_WORKER (audio);
  (void) worker;
}

static void
gst_audio_class_init (GstAudioClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstWorkerClass *worker_class = GST_WORKER_CLASS (klass);

  object_class->finalize = (GObjectFinalizeFunc) gst_audio_finalize;
  object_class->set_property = (GObjectSetPropertyFunc) gst_audio_set_property;
  object_class->get_property = (GObjectGetPropertyFunc) gst_audio_get_property;

  g_object_class_install_property (object_class, PROP_PORT,
      g_param_spec_uint ("port", "Port", "Sink port",
          GST_SWITCH_MIN_SINK_PORT, GST_SWITCH_MAX_SINK_PORT,
	  GST_SWITCH_MIN_SINK_PORT,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_HANDLE,
      g_param_spec_ulong ("handle", "Handle", "Window Handle",
          0, ((gulong)-1), 0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  worker_class->null_state = (GstWorkerNullStateFunc) gst_audio_null;
  worker_class->prepare = (GstWorkerPrepareFunc) gst_audio_prepare;
  worker_class->create_pipeline = (GstWorkerCreatePipelineFunc)
    gst_audio_create_pipeline;
}
