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
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "gstswitchserver.h"
#include "gstcomposite.h"
#include "gstrecorder.h"

enum
{
  PROP_0,
  PROP_MODE,
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
  rec->sink_port = 0;
  rec->mode = 0;
  rec->width = 0;
  rec->height = 0;

  //INFO ("init %p", rec);
}

static void
gst_recorder_dispose (GstRecorder * rec)
{
  INFO ("dispose %p", rec);
  G_OBJECT_CLASS (parent_class)->dispose (G_OBJECT (rec));
}

static void
gst_recorder_finalize (GstRecorder * rec)
{
  if (G_OBJECT_CLASS (parent_class)->finalize)
    (*G_OBJECT_CLASS (parent_class)->finalize) (G_OBJECT (rec));
}

static void
gst_recorder_get_property (GstRecorder *rec, guint property_id,
    GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  case PROP_MODE:
    g_value_set_uint (value, rec->mode);
    break;
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
  case PROP_MODE:
    rec->mode = (GstCompositeMode) (g_value_get_uint (value));
    break;
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

static const gchar *
gst_recorder_new_filename (GstRecorder * rec)
{
  time_t t;
  struct tm *tm;
  gchar stamp[128];
  const gchar *dot = NULL;
  const gchar *filename = opts.record_filename;
  if (!filename) {
    return NULL;
  }

  t = time (NULL);
  tm = localtime (&t);

  if (tm == NULL) {
    static gint num = 0; num += 1;
    snprintf (stamp, sizeof (stamp), "%d", num);
  } else {
    strftime (stamp, sizeof (stamp), "%F %H%M%S", tm);
  }

  if ((dot = g_strrstr (filename, "."))) {
    const gchar *s = g_strndup (filename, dot-filename);
    filename = g_strdup_printf ("%s %s%s", s, stamp, dot);
    g_free ((gpointer) s);
  } else {
    filename = g_strdup_printf ("%s %s.dat", filename, stamp);
  }

  return filename;
}

static GString *
gst_recorder_get_pipeline_string (GstRecorder * rec)
{
  const gchar *filename = gst_recorder_new_filename (rec);
  GString *desc;

  //INFO ("Recording to %s and port %d", filename, rec->sink_port);

  desc = g_string_new ("");

  g_string_append_printf (desc, "intervideosrc name=source_video "
      "channel=composite_video ");
  g_string_append_printf (desc, "interaudiosrc name=source_audio "
      "channel=composite_audio ");

  g_string_append_printf (desc,
      "source_video. ! video/x-raw,width=%d,height=%d ",
      rec->width, rec->height);
  ASSESS ("assess-record-video-source");
  g_string_append_printf (desc, "! queue2 ");
  ASSESS ("assess-record-video-encode-queued");
  g_string_append_printf (desc, "! vp8enc ");
  ASSESS ("assess-record-video-encoded");
  g_string_append_printf (desc, "! mux. ");

  g_string_append_printf (desc, "source_audio. ");
  ASSESS ("assess-record-audio-source");
  g_string_append_printf (desc, "! queue2 ");
  ASSESS ("assess-record-audio-queued");
  g_string_append_printf (desc, "! faac ");
  ASSESS ("assess-record-audio-encoded");
  g_string_append_printf (desc, "! mux. ");

  g_string_append_printf (desc, "avimux name=mux ");
  ASSESS ("assess-record-mux-result");
  g_string_append_printf (desc, "! tee name=result ");

  if (filename) {
    g_string_append_printf (desc, "filesink name=disk_sink sync=false "
	"location=\"%s\" ", filename);
    g_free ((gpointer) filename);
    g_string_append_printf (desc, "result. ");
    ASSESS ("assess-record-file-to-queue");
    g_string_append_printf (desc, "! queue2 ");
    ASSESS ("assess-record-file-to-sink");
    g_string_append_printf (desc, "! disk_sink. ");
  }

  g_string_append_printf (desc, "tcpserversink name=tcp_sink sync=false "
      "port=%d ", rec->sink_port);
  g_string_append_printf (desc, "result. ");
  ASSESS ("assess-record-tcp-to-queue");
  g_string_append_printf (desc, "! queue2 ");
  ASSESS ("assess-record-tcp-to-sink");
  g_string_append_printf (desc, "! gdppay ! tcp_sink. ");
  return desc;
}

static void
gst_recorder_client_socket_added (GstElement *element,
    GSocket *socket, GstRecorder *rec)
{
  g_return_if_fail (G_IS_SOCKET (socket));

  INFO ("client-socket-added: %d", g_socket_get_fd (socket));
}

static void
gst_recorder_client_socket_removed (GstElement *element,
    GSocket *socket, GstRecorder *rec)
{
  g_return_if_fail (G_IS_SOCKET (socket));

  INFO ("client-socket-removed: %d", g_socket_get_fd (socket));

  g_socket_close (socket, NULL);
}

static gboolean
gst_recorder_prepare (GstRecorder *rec)
{
  GstElement *tcp_sink = NULL;

  g_return_val_if_fail (GST_IS_RECORDER (rec), FALSE);

  tcp_sink = gst_worker_get_element_unlocked (GST_WORKER (rec), "tcp_sink");

  g_return_val_if_fail (GST_IS_ELEMENT (tcp_sink), FALSE);

  g_signal_connect (tcp_sink, "client-added",
      G_CALLBACK (gst_recorder_client_socket_added), rec);

  g_signal_connect (tcp_sink, "client-socket-removed",
      G_CALLBACK (gst_recorder_client_socket_removed), rec);

  gst_object_unref (tcp_sink);
  return TRUE;
}

static void
gst_recorder_class_init (GstRecorderClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstWorkerClass *worker_class = GST_WORKER_CLASS (klass);

  object_class->dispose = (GObjectFinalizeFunc) gst_recorder_dispose;
  object_class->finalize = (GObjectFinalizeFunc) gst_recorder_finalize;
  object_class->set_property = (GObjectSetPropertyFunc) gst_recorder_set_property;
  object_class->get_property = (GObjectGetPropertyFunc) gst_recorder_get_property;

  g_object_class_install_property (object_class, PROP_MODE,
      g_param_spec_uint ("mode", "Mode", "Composite Mode",
          COMPOSE_MODE_0, COMPOSE_MODE__LAST, COMPOSE_MODE_0,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_PORT,
      g_param_spec_uint ("port", "Port", "Sink port",
          GST_SWITCH_MIN_SINK_PORT, GST_SWITCH_MAX_SINK_PORT,
	  GST_SWITCH_MIN_SINK_PORT,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_WIDTH,
      g_param_spec_uint ("width", "Input Width", "Input video frame width",
          1, G_MAXINT, GST_SWITCH_COMPOSITE_DEFAULT_WIDTH,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_HEIGHT,
      g_param_spec_uint ("height", "Input Height", "Input video frame height",
          1, G_MAXINT, GST_SWITCH_COMPOSITE_DEFAULT_HEIGHT,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  worker_class->prepare = (GstWorkerPrepareFunc) gst_recorder_prepare;
  worker_class->get_pipeline_string = (GstWorkerGetPipelineStringFunc)
    gst_recorder_get_pipeline_string;
}
