/* GstSwitch
 * Copyright (C) 2012,2013 Duzy Chan <code@duzy.info>
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
#include "gstcase.h"

enum
{
  PROP_0,
  PROP_TYPE,
  PROP_SERVE,
  PROP_STREAM,
  PROP_PORT,
  PROP_A_WIDTH,
  PROP_A_HEIGHT,
  PROP_B_WIDTH,
  PROP_B_HEIGHT,
};

enum
{
  SIGNAL_END_CASE,
  SIGNAL__LAST,
};

static guint gst_case_signals[SIGNAL__LAST] = { 0 };
extern gboolean verbose;

G_DEFINE_TYPE (GstCase, gst_case, GST_TYPE_WORKER);

static void
gst_case_init (GstCase * cas)
{
  cas->type = GST_CASE_UNKNOWN;
  cas->stream = NULL;
  cas->serve_type = GST_SERVE_NOTHING;
  cas->sink_port = 0;
  cas->a_width = 0;
  cas->a_height = 0;
  cas->b_width = 0;
  cas->b_height = 0;

  INFO ("Case initialized");
}

static void
gst_case_finalize (GstCase * cas)
{
  if (cas->stream) {
    g_object_unref (cas->stream);
    cas->stream = NULL;
  }

  if (G_OBJECT_CLASS (gst_case_parent_class)->finalize)
    (*G_OBJECT_CLASS (gst_case_parent_class)->finalize) (G_OBJECT (cas));

  INFO ("Case finalized");
}

static void
gst_case_get_property (GstCase *cas, guint property_id,
    GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  case PROP_TYPE:
    g_value_set_uint (value, cas->type);
    break;
  case PROP_SERVE:
    g_value_set_uint (value, cas->serve_type);
    break;
  case PROP_STREAM:
    g_value_set_object (value, cas->stream);
    break;
  case PROP_PORT:
    g_value_set_uint (value, cas->sink_port);
    break;
  case PROP_A_WIDTH:
    g_value_set_uint (value, cas->a_width);
    break;
  case PROP_A_HEIGHT:
    g_value_set_uint (value, cas->a_height);
    break;
  case PROP_B_WIDTH:
    g_value_set_uint (value, cas->b_width);
    break;
  case PROP_B_HEIGHT:
    g_value_set_uint (value, cas->b_height);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (cas, property_id, pspec);
    break;
  }
}

static void
gst_case_set_property (GstCase *cas, guint property_id,
    const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  case PROP_TYPE:
    cas->type = (GstCaseType) g_value_get_uint (value);
    break;
  case PROP_SERVE:
    cas->serve_type = (GstSwitchServeStreamType) g_value_get_uint (value);
    break;
  case PROP_STREAM: {
    GObject *stream = g_value_dup_object (value);
    if (cas->stream)
      g_object_unref (cas->stream);
    cas->stream = G_INPUT_STREAM (stream);
  } break;
  case PROP_PORT:
    cas->sink_port = g_value_get_uint (value);
    break;
  case PROP_A_WIDTH:
    cas->a_width = g_value_get_uint (value);
    break;
  case PROP_A_HEIGHT:
    cas->a_height = g_value_get_uint (value);
    break;
  case PROP_B_WIDTH:
    cas->b_width = g_value_get_uint (value);
    break;
  case PROP_B_HEIGHT:
    cas->b_height = g_value_get_uint (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (cas), property_id, pspec);
    break;
  }
}

static GstElement *
gst_case_create_pipeline (GstCase * cas)
{
  GstElement *pipeline;
  GError *error = NULL;
  GString *desc;
  gchar *channel = NULL;
  gchar *convert = NULL;

  desc = g_string_new ("");

  g_string_append_printf (desc, "giostreamsrc name=source ");

  switch (cas->type) {
  case GST_CASE_COMPOSITE_A:
    channel = "composite_a";
    convert = g_strdup_printf ("identity "
	"! videoscale ! video/x-raw,width=%d,height=%d "
	//"! textoverlay text=A shaded-background=true "
	//"! timeoverlay "
	, cas->a_width, cas->a_height);
  case GST_CASE_COMPOSITE_B:
    if (channel == NULL) {
      channel = "composite_b";
      convert = g_strdup_printf ("identity "
	  "! videoscale ! video/x-raw,width=%d,height=%d "
	  //"! textoverlay text=B shaded-background=true "
	  //"! videobox border-alpha=0 left=50 top=50 right=150 bottom=230 "
	  , cas->b_width, cas->b_height);
    }
    g_string_append_printf (desc, "intervideosink name=sink channel=%s ",
	channel);
    g_string_append_printf (desc, "source. ! gdpdepay ! %s ! sink. ",
	convert);
    break;
  case GST_CASE_COMPOSITE_a:
    g_string_append_printf (desc, "source. ! tee name=as ");
    g_string_append_printf (desc, "as. ! queue2 ! gdpdepay ! gdppay "
	"tcpserversink name=sink1 port=%d ", cas->sink_port);
    g_string_append_printf (desc, "as. ! queue2 ! "
	"interaudiosink name=sink2 channel=composite_audio ");
    break;
  case GST_CASE_PREVIEW:
    g_string_append_printf (desc, "tcpserversink name=sink sync=false "
	"port=%d ", cas->sink_port);
    g_string_append_printf (desc, "source. ! gdpdepay ! gdppay ! sink. ");
    break;
  case GST_CASE_UNKNOWN:
    ERROR ("unknown case");
    break;
  }

  g_free (convert);
  convert = NULL;

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
gst_case_prepare (GstCase *cas)
{
  GstWorker *worker = GST_WORKER (cas);

  if (!cas->stream) {
    ERROR ("no stream for new case");
    return FALSE;
  }

  if (!worker->source) {
    ERROR ("no source");
    return FALSE;
  }

  g_object_set (worker->source, "stream", cas->stream, NULL);

  return TRUE;
}

static void
gst_case_null (GstCase *cas)
{
  GstWorker *worker = GST_WORKER (cas);

  INFO ("null: %s (%p)", worker->name, cas);

  g_signal_emit (cas, gst_case_signals[SIGNAL_END_CASE], 0);
}

static void
gst_case_class_init (GstCaseClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstWorkerClass *worker_class = GST_WORKER_CLASS (klass);

  object_class->finalize = (GObjectFinalizeFunc) gst_case_finalize;
  object_class->set_property = (GObjectSetPropertyFunc) gst_case_set_property;
  object_class->get_property = (GObjectGetPropertyFunc) gst_case_get_property;

  gst_case_signals[SIGNAL_END_CASE] =
    g_signal_new ("end-case", G_TYPE_FROM_CLASS (klass),
	G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (GstCaseClass, end_case),
	NULL, NULL, g_cclosure_marshal_generic, G_TYPE_NONE, 0);

  g_object_class_install_property (object_class, PROP_TYPE,
      g_param_spec_uint ("type", "Type", "Case type",
          GST_CASE_UNKNOWN, GST_CASE_PREVIEW, GST_CASE_UNKNOWN,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_SERVE,
      g_param_spec_uint ("serve", "Serve", "Serve type",
          GST_SERVE_NOTHING, GST_SERVE_AUDIO_STREAM, GST_SERVE_NOTHING,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_STREAM,
      g_param_spec_object ("stream", "Stream", "Stream to read from",
          G_TYPE_INPUT_STREAM, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_PORT,
      g_param_spec_uint ("port", "Port", "Sink port",
          GST_SWITCH_MIN_SINK_PORT, GST_SWITCH_MAX_SINK_PORT,
	  GST_SWITCH_MIN_SINK_PORT,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_A_WIDTH,
      g_param_spec_uint ("awidth", "A Width", "Channel A width",
          1, G_MAXINT, GST_SWITCH_COMPOSITE_DEFAULT_A_WIDTH,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_A_HEIGHT,
      g_param_spec_uint ("aheight", "A Height", "Channel A height",
          1, G_MAXINT, GST_SWITCH_COMPOSITE_DEFAULT_A_HEIGHT,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_B_WIDTH,
      g_param_spec_uint ("bwidth", "B Width", "Channel B width",
          1, G_MAXINT, GST_SWITCH_COMPOSITE_DEFAULT_B_WIDTH,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_B_HEIGHT,
      g_param_spec_uint ("bheight", "B Height", "Channel B height",
          1, G_MAXINT, GST_SWITCH_COMPOSITE_DEFAULT_B_HEIGHT,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  worker_class->null_state = (GstWorkerNullStateFunc) gst_case_null;
  worker_class->prepare = (GstWorkerPrepareFunc) gst_case_prepare;
  worker_class->create_pipeline = (GstWorkerCreatePipelineFunc)
    gst_case_create_pipeline;
}
