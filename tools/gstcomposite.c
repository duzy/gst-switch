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

enum
{
  PROP_0,
  PROP_PORT,
  PROP_A_X,
  PROP_A_Y,
  PROP_A_WIDTH,
  PROP_A_HEIGHT,
  PROP_B_X,
  PROP_B_Y,
  PROP_B_WIDTH,
  PROP_B_HEIGHT,
  PROP_WIDTH,
  PROP_HEIGHT,
};

enum
{
  SIGNAL_END_COMPOSITE,
  SIGNAL__LAST,
};

static guint gst_composite_signals[SIGNAL__LAST] = { 0 };
extern gboolean verbose;

G_DEFINE_TYPE (GstComposite, gst_composite, GST_TYPE_WORKER);

static void
gst_composite_init (GstComposite * composite)
{
  composite->a_x = GST_SWITCH_COMPOSITE_DEFAULT_A_X;
  composite->a_y = GST_SWITCH_COMPOSITE_DEFAULT_A_Y;
  composite->a_width = GST_SWITCH_COMPOSITE_DEFAULT_A_WIDTH;
  composite->a_height = GST_SWITCH_COMPOSITE_DEFAULT_A_HEIGHT;
  composite->b_x = GST_SWITCH_COMPOSITE_DEFAULT_B_X;
  composite->b_y = GST_SWITCH_COMPOSITE_DEFAULT_B_Y;
  composite->b_width = GST_SWITCH_COMPOSITE_DEFAULT_B_WIDTH;
  composite->b_height = GST_SWITCH_COMPOSITE_DEFAULT_B_HEIGHT;
  composite->width = GST_SWITCH_COMPOSITE_DEFAULT_A_WIDTH;
  composite->height = GST_SWITCH_COMPOSITE_DEFAULT_A_HEIGHT;

  INFO ("Composite initialized");
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
  case PROP_A_X:
    composite->a_x = g_value_get_uint (value);
    break;
  case PROP_A_Y:
    composite->a_y = g_value_get_uint (value);
    break;
  case PROP_A_WIDTH:
    composite->a_width = g_value_get_uint (value);
    break;
  case PROP_A_HEIGHT:
    composite->a_height = g_value_get_uint (value);
    break;
  case PROP_B_X:
    composite->b_x = g_value_get_uint (value);
    break;
  case PROP_B_Y:
    composite->b_y = g_value_get_uint (value);
    break;
  case PROP_B_WIDTH:
    composite->b_width = g_value_get_uint (value);
    break;
  case PROP_B_HEIGHT:
    composite->b_height = g_value_get_uint (value);
    break;
  case PROP_WIDTH:
    composite->width = g_value_get_uint (value);
    break;
  case PROP_HEIGHT:
    composite->height = g_value_get_uint (value);
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
  case PROP_A_X:
    g_value_set_uint (value, composite->a_x);
    break;
  case PROP_A_Y:
    g_value_set_uint (value, composite->a_y);
    break;
  case PROP_A_WIDTH:
    g_value_set_uint (value, composite->a_width);
    break;
  case PROP_A_HEIGHT:
    g_value_set_uint (value, composite->a_height);
    break;
  case PROP_B_X:
    g_value_set_uint (value, composite->b_x);
    break;
  case PROP_B_Y:
    g_value_set_uint (value, composite->b_y);
    break;
  case PROP_B_WIDTH:
    g_value_set_uint (value, composite->b_width);
    break;
  case PROP_B_HEIGHT:
    g_value_set_uint (value, composite->b_height);
    break;
  case PROP_WIDTH:
    g_value_set_uint (value, composite->width);
    break;
  case PROP_HEIGHT:
    g_value_set_uint (value, composite->height);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (composite), property_id,
	pspec);
    break;
  }
}

static GString *
gst_composite_get_pipeline_string (GstComposite * composite)
{
  GString *desc;
  gint ax = composite->a_x;
  gint ay = composite->a_y;
  gint bx = composite->b_x;
  gint by = composite->b_y;

  desc = g_string_new ("");

  g_string_append_printf (desc, "intervideosrc name=source_a "
      "channel=composite_a ");
  g_string_append_printf (desc, "intervideosrc name=source_b "
      "channel=composite_b ");
  g_string_append_printf (desc, "tcpserversink name=sink sync=false "
      "port=%d ", composite->sink_port);
  g_string_append_printf (desc, "videomixer name=compose "
      "sink_0::zorder=0 "
      "sink_0::xpos=%d "
      "sink_0::ypos=%d "
      "sink_1::alpha=0.8 "
      "sink_1::xpos=%d "
      "sink_1::ypos=%d "
      "sink_1::zorder=1 ",
      ax, ay, bx, by);
  g_string_append_printf (desc, "source_b.!video/x-raw,width=%d,height=%d "
      "! queue2 ! compose.sink_1 ",
      composite->b_width, composite->b_height);
  g_string_append_printf (desc, "source_a.!video/x-raw,width=%d,height=%d "
      "! queue2 ! compose.sink_0 ",
      composite->a_width, composite->a_height);
  g_string_append_printf (desc, "compose. ! gdppay ! tee name=result ");
  g_string_append_printf (desc, "result. ! queue ! sink. ");

  if (opts.record_filename) {
    g_string_append_printf (desc, "result. ! queue ! record. ");
#if 0
    g_string_append_printf (desc, "filesink name=record location=%s ",
	opts.record_filename);
#else
    g_string_append_printf (desc, "gdpdepay name=record ");
    g_string_append_printf (desc, "record. ! intervideosink "
	"channel=composite_video sync=false ");
#endif
  }

  return desc;
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

  INFO ("null: %s (%p)", worker->name, composite);

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

  g_object_class_install_property (object_class, PROP_A_X,
      g_param_spec_uint ("ax", "A xpos", "Channel A frame xpos",
          0, G_MAXINT, GST_SWITCH_COMPOSITE_DEFAULT_A_X,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_A_Y,
      g_param_spec_uint ("ay", "A ypos", "Channel A frame ypos",
          0, G_MAXINT, GST_SWITCH_COMPOSITE_DEFAULT_A_Y,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_A_WIDTH,
      g_param_spec_uint ("awidth", "A Width", "Channel A frame width",
          1, G_MAXINT, GST_SWITCH_COMPOSITE_DEFAULT_A_WIDTH,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_A_HEIGHT,
      g_param_spec_uint ("aheight", "A Height", "Channel A frame height",
          1, G_MAXINT, GST_SWITCH_COMPOSITE_DEFAULT_A_HEIGHT,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_B_X,
      g_param_spec_uint ("bx", "B xpos", "Channel B frame xpos",
          0, G_MAXINT, GST_SWITCH_COMPOSITE_DEFAULT_B_X,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_B_Y,
      g_param_spec_uint ("by", "B ypos", "Channel B frame ypos",
          0, G_MAXINT, GST_SWITCH_COMPOSITE_DEFAULT_B_Y,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_B_WIDTH,
      g_param_spec_uint ("bwidth", "B Width", "Channel B frame width",
          1, G_MAXINT, GST_SWITCH_COMPOSITE_DEFAULT_B_WIDTH,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_B_HEIGHT,
      g_param_spec_uint ("bheight", "B Height", "Channel B frame height",
          1, G_MAXINT, GST_SWITCH_COMPOSITE_DEFAULT_B_HEIGHT,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_WIDTH,
      g_param_spec_uint ("width", "Composite Width", "Output frame width",
          1, G_MAXINT, GST_SWITCH_COMPOSITE_DEFAULT_A_WIDTH,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_HEIGHT,
      g_param_spec_uint ("height", "Composite Height", "Output frame height",
          1, G_MAXINT, GST_SWITCH_COMPOSITE_DEFAULT_A_HEIGHT,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  worker_class->null_state = (GstWorkerNullStateFunc) gst_composite_null;
  worker_class->prepare = (GstWorkerPrepareFunc) gst_composite_prepare;
  worker_class->get_pipeline_string = (GstWorkerGetPipelineString)
    gst_composite_get_pipeline_string;
}
