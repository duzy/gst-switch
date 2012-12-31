/* GstComposite
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

#include <stdlib.h>
#include <string.h>
#include "gstswitchsrv.h"

enum
{
  PROP_0,
  PROP_PORT,
};

enum
{
  SIGNAL_END_COMPOSITE,
  SIGNAL__LAST,
};

static guint gst_composite_signals[SIGNAL__LAST] = { 0 };

G_DEFINE_TYPE (GstComposite, gst_composite, GST_TYPE_WORKER);

static void
gst_composite_init (GstComposite * composite)
{
}

static void
gst_composite_finalize (GstComposite * composite)
{
  if (G_OBJECT_CLASS (gst_composite_parent_class)->finalize)
    (*G_OBJECT_CLASS (gst_composite_parent_class)->finalize) (G_OBJECT (composite));

  INFO ("Composite finalized");
}

static void
gst_composite_set_property (GstComposite * composite, guint property_id,
    const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  case PROP_PORT:
    composite->sink_port = g_value_get_uint (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (composite), property_id,
	pspec);
    break;
  }
}

static void
gst_composite_get_property (GstComposite * composite, guint property_id,
    GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  case PROP_PORT:
    g_value_set_uint (value, composite->sink_port);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (composite), property_id,
	pspec);
    break;
  }
}

static GstElement *
gst_composite_create_pipeline (GstComposite * composite)
{
  GString *desc;
  GstElement *pipeline;
  GError *error = NULL;

  INFO ("Listenning on port %d", opts.port);

  desc = g_string_new ("");

  /*
  g_string_append_printf (desc, "videotestsrc name=src0 pattern=snow ");
  g_string_append_printf (desc, "videotestsrc name=src1 pattern=1 ");

  g_string_append_printf (desc, "videomixer name=compose "
      "sink_0::alpha=0.6 sink_1::alpha=0.5 ");
  g_string_append_printf (desc, "identity name=compose_a ");
  g_string_append_printf (desc, "identity name=compose_b ");
  g_string_append_printf (desc, "compose_a. ! compose.sink_0 ");
  g_string_append_printf (desc, "compose_b. ! compose.sink_1 ");
  g_string_append_printf (desc, "compose. ! compose_sink. ");

  g_string_append_printf (desc, "src0. ! video/x-raw-yuv, framerate=10/1, width=200, height=150 ! compose_a. ");
  g_string_append_printf (desc, "src1. ! video/x-raw-yuv, framerate=10/1 ! compose_b. ");
  //g_string_append_printf (desc, "src0. ! compose_a. ");
  //g_string_append_printf (desc, "src1. ! compose_b. ");

  g_string_append_printf (desc, "identity name=compose_sink ");
  g_string_append_printf (desc, "compose_sink. ! videoconvert ! xvimagesink ");
  */
#if 0
  g_string_append_printf (desc, "videotestsrc name=src0 pattern=0 "
      //"! video/x-raw-yuv,framerate=10/1,width=100,height=100 "
      //"! videobox border-alpha=0 left=0 top=0 "
      "! compose. ");
  g_string_append_printf (desc, "videotestsrc name=src1 pattern=snow "
      //"! video/x-raw-yuv,framerate=10/1,width=300,height=250 "
      "! videobox border-alpha=0 left=100 top=100 "
      "! compose. ");
  g_string_append_printf (desc, "videomixer name=compose "
      //"sink_0::alpha=0.6 "
      "sink_1::alpha=0.5 "
      "! videoconvert "
      "! xvimagesink ");
#else
  g_string_append_printf (desc, "intervideosrc name=source_a "
      "channel=composite_a ");
  g_string_append_printf (desc, "intervideosrc name=source_b "
      "channel=composite_b ");
  g_string_append_printf (desc, "videomixer name=compose "
      "sink_0::alpha=0.8 "
      "sink_0::zorder=0 "
      "sink_1::alpha=0.5 "
      "sink_1::xpos=20 sink_1::ypos=20 "
      "sink_1::zorder=1 ");
  g_string_append_printf (desc, "source_b. "
      "! video/x-raw,framerate=10/1,width=100,height=80 "
      //"! videobox border-alpha=0 left=50 top=50 right=150 bottom=230 "
      "! compose.sink_1 ");
  g_string_append_printf (desc, "source_a. "
      "! video/x-raw,framerate=10/1,width=300,height=250 "
      "! compose.sink_0 ");
  g_string_append_printf (desc, "compose. ! gdppay ! sink. ");
  g_string_append_printf (desc, "tcpserversink name=sink "
      "port=%d ", composite->sink_port);
#endif

  if (opts.verbose)
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
on_source_pad_added (GstElement * element, GstPad * pad,
    GstComposite * composite)
{
  INFO ("source-pad-added: %s.%s", GST_ELEMENT_NAME (element),
      GST_PAD_NAME (pad));
}

static gboolean
gst_composite_prepare (GstComposite *composite)
{
  GstWorker *worker = GST_WORKER (composite);
  if (worker->source) {
    g_signal_connect (worker->source, "pad-added",
	G_CALLBACK (on_source_pad_added), worker);
  }
  return TRUE;
}

static void
gst_composite_null (GstComposite *composite)
{
  GstWorker *worker = GST_WORKER (composite);

  INFO ("null composite: %s (%p)", worker->name, composite);

  g_signal_emit (composite, gst_composite_signals[SIGNAL_END_COMPOSITE], 0);
}

static void
gst_composite_class_init (GstCompositeClass * klass)
{
  GObjectClass * object_class = G_OBJECT_CLASS (klass);
  GstWorkerClass * worker_class = GST_WORKER_CLASS (klass);

  object_class->finalize = (GObjectFinalizeFunc) gst_composite_finalize;
  object_class->set_property = (GObjectSetPropertyFunc) gst_composite_set_property;
  object_class->get_property = (GObjectGetPropertyFunc) gst_composite_get_property;

  gst_composite_signals[SIGNAL_END_COMPOSITE] =
    g_signal_new ("end-composite", G_TYPE_FROM_CLASS (klass),
	G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (GstCompositeClass, end_composite),
	NULL, NULL, g_cclosure_marshal_generic, G_TYPE_NONE, 0);

  g_object_class_install_property (object_class, PROP_PORT,
      g_param_spec_uint ("port", "Port", "Sink port",
          GST_SWITCH_MIN_SINK_PORT, GST_SWITCH_MAX_SINK_PORT,
	  GST_SWITCH_MIN_SINK_PORT,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  worker_class->null_state = (GstWorkerNullStateFunc) gst_composite_null;
  worker_class->prepare = (GstWorkerPrepareFunc) gst_composite_prepare;
  worker_class->create_pipeline = (GstWorkerCreatePipelineFunc)
    gst_composite_create_pipeline;
}
