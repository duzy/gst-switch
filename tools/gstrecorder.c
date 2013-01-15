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
#include "gstcomposite.h"
#include "gstrecorder.h"

enum
{
  PROP_0,
  PROP_PORT,
  PROP_WIDTH,
  PROP_HEIGHT,
};

enum
{
  SIGNAL__LAST,
};

//static guint gst_recorder_signals[SIGNAL__LAST] = { 0 };
extern gboolean verbose;

#define parent_class gst_recorder_parent_class

G_DEFINE_TYPE (GstRecorder, gst_recorder, GST_TYPE_WORKER);

static void
gst_recorder_init (GstRecorder * rec)
{
  INFO ("Recorder initialized");
  rec->sink_port = 0;
  rec->width = GST_SWITCH_COMPOSITE_DEFAULT_A_WIDTH;
  rec->height = GST_SWITCH_COMPOSITE_DEFAULT_A_HEIGHT;
  rec->write_disk = GST_WORKER (g_object_new (GST_TYPE_WORKER, "name", "writedisk", NULL));
  rec->write_tcp = GST_WORKER (g_object_new (GST_TYPE_WORKER, "name", "writetcp", NULL));
}

static void
gst_recorder_finalize (GstRecorder * rec)
{
  if (G_OBJECT_CLASS (parent_class)->finalize)
    (*G_OBJECT_CLASS (parent_class)->finalize) (G_OBJECT (rec));

  if (rec->write_disk) {
    g_object_unref (rec->write_disk);
    rec->write_disk = NULL;
  }

  if (rec->write_tcp) {
    g_object_unref (rec->write_tcp);
    rec->write_tcp = NULL;
  }

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
  case PROP_WIDTH:
    g_value_set_uint (value, rec->width);
    break;
  case PROP_HEIGHT:
    g_value_set_uint (value, rec->height);
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
  case PROP_WIDTH:
    rec->width = g_value_get_uint (value);
    break;
  case PROP_HEIGHT:
    rec->height = g_value_get_uint (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (rec), property_id, pspec);
    break;
  }
}

static GString *
gst_recorder_get_write_disk_string (GstRecorder *rec)
{
  GString *desc = g_string_new ("");
  return desc;
}

static GString *
gst_recorder_get_write_tcp_string (GstRecorder *rec)
{
  GString *desc = g_string_new ("");
  return desc;
}

static GString *
gst_recorder_get_pipeline_string (GstRecorder * rec)
{
  GString *desc;
  const gchar *filename = opts.record_filename;
  if (!filename) {
    filename = "/dev/null";
  }

  INFO ("Recording to %s and port %d", filename, rec->sink_port);

  desc = g_string_new ("");

  g_string_append_printf (desc, "intervideosrc name=source_video "
      "channel=composite_video ");
  g_string_append_printf (desc, "interaudiosrc name=source_audio "
      "channel=composite_audio ");
  g_string_append_printf (desc, "source_video. "
      "! video/x-raw,width=%d,height=%d "
      "! queue ! vp8enc " //! mpeg2enc
      "! mux. ", rec->width, rec->height);
  g_string_append_printf (desc, "source_audio. "
      "! queue ! faac "
      "! mux. ");
  g_string_append_printf (desc, "avimux name=mux ! tee name=result ");
  g_string_append_printf (desc, "filesink name=disk_sink sync=false "
      "location=%s ", filename);
  g_string_append_printf (desc, "tcpserversink name=tcp_sink sync=false "
      "port=%d ", rec->sink_port);
  g_string_append_printf (desc, "result. ! queue2 ! disk_sink. ");
  g_string_append_printf (desc, "result. ! queue2 ! gdppay ! tcp_sink. ");

  return desc;
}

static gboolean
gst_recorder_prepare (GstRecorder *rec)
{
#if 0
  if (rec->write_disk->pipeline_string)
    g_string_free (rec->write_disk->pipeline_string, FALSE);
  if (rec->write_tcp->pipeline_string)
    g_string_free (rec->write_tcp->pipeline_string, FALSE);

  rec->write_disk->pipeline_string = gst_recorder_get_write_disk_string (rec);
  rec->write_tcp->pipeline_string = gst_recorder_get_write_tcp_string (rec);

  if (!gst_worker_prepare (rec->write_disk))
    goto error_prepare_write_disk;

  gst_worker_start (rec->write_disk);

  if (!gst_worker_prepare (rec->write_tcp))
    goto error_prepare_write_tcp;

  gst_worker_start (rec->write_tcp);

  return TRUE;

 error_prepare_write_disk:
  {
    ERROR ("prepare writing to disk");
    g_string_free (rec->write_disk->pipeline_string, FALSE);
    rec->write_disk->pipeline_string = NULL;
    return FALSE;
  }

 error_prepare_write_tcp:
  {
    ERROR ("prepare writing to TCP");
    g_string_free (rec->write_tcp->pipeline_string, FALSE);
    rec->write_tcp->pipeline_string = NULL;
    return FALSE;
  }
#else
  return TRUE;
#endif
}

static void
gst_recorder_null (GstRecorder *rec)
{
  GstWorker *worker = GST_WORKER (rec);

  INFO ("null: %s (%p)", worker->name, rec);
}

static void
gst_recorder_class_init (GstRecorderClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstWorkerClass *worker_class = GST_WORKER_CLASS (klass);

  object_class->finalize = (GObjectFinalizeFunc) gst_recorder_finalize;
  object_class->set_property = (GObjectSetPropertyFunc) gst_recorder_set_property;
  object_class->get_property = (GObjectGetPropertyFunc) gst_recorder_get_property;

  g_object_class_install_property (object_class, PROP_PORT,
      g_param_spec_uint ("port", "Port", "Sink port",
          GST_SWITCH_MIN_SINK_PORT, GST_SWITCH_MAX_SINK_PORT,
	  GST_SWITCH_MIN_SINK_PORT,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_WIDTH,
      g_param_spec_uint ("width", "Input Width", "Input video frame width",
          1, G_MAXINT, GST_SWITCH_COMPOSITE_DEFAULT_A_WIDTH,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_HEIGHT,
      g_param_spec_uint ("height", "Input Height", "Input video frame height",
          1, G_MAXINT, GST_SWITCH_COMPOSITE_DEFAULT_A_HEIGHT,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  worker_class->null_state = (GstWorkerNullStateFunc) gst_recorder_null;
  worker_class->prepare = (GstWorkerPrepareFunc) gst_recorder_prepare;
  worker_class->get_pipeline_string = (GstWorkerGetPipelineString)
    gst_recorder_get_pipeline_string;
}
