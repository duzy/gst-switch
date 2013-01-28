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
#include "gstrecorder.h"

#define GST_COMPOSITE_LOCK(composite) (g_mutex_lock (&(composite)->lock))
#define GST_COMPOSITE_UNLOCK(composite) (g_mutex_unlock (&(composite)->lock))
#define GST_COMPOSITE_LOCK_RECORDER(composite) (g_mutex_lock (&(composite)->recorder_lock))
#define GST_COMPOSITE_UNLOCK_RECORDER(composite) (g_mutex_unlock (&(composite)->recorder_lock))
#define GST_COMPOSITE_LOCK_TRANSITION(composite) (g_mutex_lock (&(composite)->transition_lock))
#define GST_COMPOSITE_UNLOCK_TRANSITION(composite) (g_mutex_unlock (&(composite)->transition_lock))

enum
{
  PROP_0,
  PROP_MODE,
  PROP_PORT,
  PROP_ENCODE_PORT,
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
  SIGNAL_START_OUTPUT,
  SIGNAL_START_RECORDER,
  SIGNAL_END_OUTPUT,
  SIGNAL_END_RECORDER,
  SIGNAL__LAST,
};

static guint gst_composite_signals[SIGNAL__LAST] = { 0 };
extern gboolean verbose;

#define gst_composite_parent_class parent_class
G_DEFINE_TYPE (GstComposite, gst_composite, GST_TYPE_WORKER);

static void gst_composite_set_mode (GstComposite *, GstCompositeMode);
static void gst_composite_start_transition (GstComposite *);

static void
gst_composite_init (GstComposite * composite)
{
  composite->adjusting = FALSE;
  composite->transition = FALSE;
  composite->deprecated = FALSE;
  composite->output = NULL;
  composite->recorder = NULL;

  g_mutex_init (&composite->lock);
  g_mutex_init (&composite->recorder_lock);
  g_mutex_init (&composite->transition_lock);

  gst_composite_set_mode (composite, DEFAULT_COMPOSE_MODE);

  /* Indicating transition from no-mode to default mode.
   */
  composite->transition = TRUE;

  INFO ("init %p", composite);
}

static void
gst_composite_dispose (GstComposite * composite)
{
  INFO ("dispose %p", composite);
  G_OBJECT_CLASS (parent_class)->dispose (G_OBJECT (composite));
}

static void
gst_composite_finalize (GstComposite * composite)
{
  g_mutex_clear (&composite->lock);
  g_mutex_clear (&composite->recorder_lock);
  g_mutex_clear (&composite->transition_lock);

  if (G_OBJECT_CLASS (parent_class)->finalize)
    (*G_OBJECT_CLASS (parent_class)->finalize) (G_OBJECT (composite));
}

static void
gst_composite_set_mode (GstComposite * composite, GstCompositeMode mode)
{
  guint h;

  if (composite->transition) {
    WARN ("changing mode in transition is not allowed");
    return;
  }

  composite->a_x = 0;
  composite->a_y = 0;
  composite->a_width  = GST_SWITCH_COMPOSITE_DEFAULT_WIDTH;
  composite->a_height = GST_SWITCH_COMPOSITE_DEFAULT_HEIGHT;
  switch ((composite->mode = mode)) {
  case COMPOSE_MODE_0:
    composite->b_x = 0;
    composite->b_y = 0;
    composite->b_width  = 0;
    composite->b_height = 0;
    composite->width = composite->a_x + composite->a_width;
    composite->height = composite->a_y + composite->a_height;
    break;
  case COMPOSE_MODE_1:
    composite->b_x = (guint) ((double) composite->a_width * 0.08 + 0.5);
    composite->b_y = (guint) ((double) composite->a_height * 0.08 + 0.5);
    composite->b_width  = (guint) ((double) composite->a_width * 0.3 + 0.5);
    composite->b_height = (guint) ((double) composite->a_height * 0.3 + 0.5);
    composite->width = composite->a_x + composite->a_width;
    composite->height = composite->a_y + composite->a_height;
    break;
  case COMPOSE_MODE_2:
    composite->b_x = composite->a_x + composite->a_width + 1;
    composite->b_y = composite->a_y;
    composite->b_width  = (guint) ((double) composite->a_width * 0.3 + 0.5);
    composite->b_height = (guint) ((double) composite->a_height * 0.3 + 0.5);
    goto compute_side_by_side_size;
  case COMPOSE_MODE_3:
    composite->b_x = composite->a_x + composite->a_width + 1;
    composite->b_y = composite->a_y;
    composite->b_width  = GST_SWITCH_COMPOSITE_DEFAULT_WIDTH;
    composite->b_height = GST_SWITCH_COMPOSITE_DEFAULT_HEIGHT;
  compute_side_by_side_size:
    composite->width = composite->b_x + composite->b_width;
    composite->height = composite->a_y + composite->a_height;
    if (composite->height < (h = composite->b_y + composite->b_height))
      composite->height = h;
  default:
    break;
  }

  INFO ("new mode %d, %dx%d (%dx%d, %dx%d)", mode,
      composite->width, composite->height,
      composite->a_width, composite->a_height,
      composite->b_width, composite->b_height);

  if (composite->output && composite->recorder) {
    gst_composite_start_transition (composite);
  }
}

static gboolean
gst_composite_ready_for_transition (GstComposite *composite)
{
  return !composite->transition;
}

static void
gst_composite_start_transition (GstComposite *composite)
{
  g_return_if_fail (GST_IS_COMPOSITE (composite));

  GST_COMPOSITE_LOCK_TRANSITION (composite);

  if (gst_composite_ready_for_transition (composite)) {
    composite->transition = gst_worker_stop (GST_WORKER (composite));
    INFO ("transtion ok=%d, %d, %dx%d", composite->transition,
	composite->mode, composite->width, composite->height);
  }

  GST_COMPOSITE_UNLOCK_TRANSITION (composite);
}

static void
gst_composite_set_property (GstComposite * composite, guint property_id,
    const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  case PROP_PORT:
    composite->sink_port = g_value_get_uint (value);
    break;
  case PROP_ENCODE_PORT:
    composite->encode_sink_port = g_value_get_uint (value);
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
  case PROP_MODE: {
    guint mode = g_value_get_uint (value);
    if (COMPOSE_MODE_0 <= mode && mode <= COMPOSE_MODE__LAST) {
      gst_composite_set_mode (composite, (GstCompositeMode) mode);
    } else {
      WARN ("invalid composite mode %d", mode);
    }
  } break;
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
  case PROP_MODE:
    g_value_set_uint (value, composite->mode);
    break;
  case PROP_PORT:
    g_value_set_uint (value, composite->sink_port);
    break;
  case PROP_ENCODE_PORT:
    g_value_set_uint (value, composite->encode_sink_port);
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

static void
gst_composite_apply_parameters (GstComposite * composite)
{
  GstWorkerClass * worker_class;

  g_return_if_fail (GST_IS_COMPOSITE (composite));

  worker_class = GST_WORKER_CLASS (G_OBJECT_GET_CLASS (composite));

  if (!worker_class->reset (GST_WORKER (composite))) {
    ERROR ("failed to reset composite");
  }
  if (!worker_class->reset (GST_WORKER (composite->output))) {
    ERROR ("failed to reset composite output");
  }

  g_object_set (composite->recorder,
      "port", composite->encode_sink_port,
      "mode", composite->mode, "width", composite->width,
      "height", composite->height, NULL);

  if (!worker_class->reset (GST_WORKER (composite->recorder))) {
    ERROR ("failed to reset composite recorder");
  }
}

static GString *
gst_composite_get_pipeline_string (GstComposite * composite)
{
  GString *desc;

  desc = g_string_new ("");

  g_string_append_printf (desc, "intervideosrc name=source_a "
      "channel=composite_a ");
  if (composite->mode == COMPOSE_MODE_0) {
    g_string_append_printf (desc, "source_a. "
	"! video/x-raw,width=%d,height=%d ! queue2 ! identity name=compose ",
	composite->a_width, composite->a_height);
  } else {
    g_string_append_printf (desc, "intervideosrc name=source_b "
	"channel=composite_b ");
    g_string_append_printf (desc, "videomixer name=compose "
	"sink_0::xpos=%d "
	"sink_0::ypos=%d "
	"sink_0::zorder=0 "
	"sink_1::xpos=%d "
	"sink_1::ypos=%d "
	"sink_1::zorder=1 ",
	composite->a_x, composite->a_y,
	composite->b_x, composite->b_y);
    g_string_append_printf (desc,"source_b. "
	"! video/x-raw,width=%d,height=%d ! queue2 ",
	composite->a_width, composite->a_height);
    if (composite->a_width  != composite->b_width ||
	composite->a_height != composite->b_height) {
      g_string_append_printf (desc, "! videoscale "
	  "! video/x-raw,width=%d,height=%d ",
	  composite->b_width, composite->b_height);
    }
    g_string_append_printf (desc, "! compose.sink_1 ");
    g_string_append_printf (desc, "source_a. "
	"! video/x-raw,width=%d,height=%d ! queue2 ! compose.sink_0 ",
	composite->a_width, composite->a_height);
  }
  g_string_append_printf (desc, "compose. ! video/x-raw,width=%d,height=%d "
      "! tee name=result ", composite->width, composite->height);
  g_string_append_printf (desc, "result. ! queue2 ! out. ");
  g_string_append_printf (desc, "intervideosink name=out "
      "channel=composite_out ");

  if (opts.record_filename) {
    g_string_append_printf (desc, "result. ! queue2 ! record. ");
    g_string_append_printf (desc, "intervideosink name=record "
	"channel=composite_video ");
  }

  return desc;
}

static GString *
gst_composite_get_output_string (GstWorker *worker,
    GstComposite * composite)
{
  GString *desc;

  desc = g_string_new ("");

  g_string_append_printf (desc, "intervideosrc name=source "
      "channel=composite_out ");
  g_string_append_printf (desc, "tcpserversink name=sink "
      "port=%d ", composite->sink_port);
  g_string_append_printf (desc, "source. ! video/x-raw,width=%d,height=%d "
      "! gdppay ! sink. ", composite->width, composite->height);

  return desc;
}

static void
gst_composite_start_output (GstWorker *worker, GstComposite *composite)
{
  g_signal_emit (composite, gst_composite_signals[SIGNAL_START_OUTPUT], 0);
}

static void
gst_composite_start_recorder (GstRecorder *rec, GstComposite *composite)
{
  g_signal_emit (composite, gst_composite_signals[SIGNAL_START_RECORDER], 0);
}

static void
gst_composite_end_output (GstWorker *worker, GstComposite *composite)
{
  g_signal_emit (composite, gst_composite_signals[SIGNAL_END_OUTPUT], 0);
}

static void
gst_composite_end_recorder (GstRecorder *rec, GstComposite *composite)
{
  g_signal_emit (composite, gst_composite_signals[SIGNAL_END_RECORDER], 0);
}

static gboolean
gst_composite_prepare (GstComposite *composite)
{
  g_return_val_if_fail (GST_IS_COMPOSITE (composite), FALSE);

  if (composite->output == NULL) {
    composite->output = GST_WORKER (g_object_new (GST_TYPE_WORKER,
	    "name", "output", NULL));
    composite->output->pipeline_func_data = composite;
    composite->output->pipeline_func = (GstWorkerGetPipelineString)
      gst_composite_get_output_string;
    g_signal_connect (composite->output, "start-worker",
	G_CALLBACK (gst_composite_start_output), composite);
    g_signal_connect (composite->output, "end-worker",
	G_CALLBACK (gst_composite_end_output), composite);
  }

  if (composite->recorder == NULL) {
    composite->recorder = GST_RECORDER (g_object_new (GST_TYPE_RECORDER,
	    "name", "recorder", "port", composite->encode_sink_port,
	    "mode", composite->mode, "width", composite->width,
	    "height", composite->height, NULL));
    g_signal_connect (composite->recorder, "start-worker",
	G_CALLBACK (gst_composite_start_recorder), composite);
    g_signal_connect (composite->recorder, "end-worker",
	G_CALLBACK (gst_composite_end_recorder), composite);
  }

  return TRUE;
}

static void
gst_composite_alive (GstComposite *composite)
{
  g_return_if_fail (GST_IS_COMPOSITE (composite));

  if (composite->transition) {
    GST_COMPOSITE_LOCK_TRANSITION (composite);
    if (composite->transition) {
      gst_worker_start (GST_WORKER (composite->output));
      gst_worker_start (GST_WORKER (composite->recorder));
      composite->transition = FALSE;
      INFO ("new mode %d, %dx%d transited", composite->mode,
	  composite->width, composite->height);
    }
    GST_COMPOSITE_UNLOCK_TRANSITION (composite);
  }

  if (composite->adjusting) {
    composite->adjusting = FALSE;
  }
}

static GstWorkerNullReturn
gst_composite_null (GstComposite *composite)
{
  g_return_val_if_fail (GST_IS_COMPOSITE (composite), GST_WORKER_NR_END);

  if (composite->transition) {
    GST_COMPOSITE_LOCK_TRANSITION (composite);
    if (composite->transition) {
      gst_worker_stop (GST_WORKER (composite->output));
      gst_worker_stop (GST_WORKER (composite->recorder));
      INFO ("new mode %d, %dx%d applying...",
	  composite->mode, composite->width, composite->height);
      gst_composite_apply_parameters (composite);
    }
    GST_COMPOSITE_UNLOCK_TRANSITION (composite);
  } else if (composite->adjusting) {
    GstWorkerClass * worker_class;
    worker_class = GST_WORKER_CLASS (G_OBJECT_GET_CLASS (composite));
    if (!worker_class->reset (GST_WORKER (composite))) {
      ERROR ("failed to reset composite");
    }
  }

  return composite->deprecated ? GST_WORKER_NR_END : GST_WORKER_NR_REPLAY;
}

gboolean
gst_composite_adjust_pip (GstComposite *composite, gint x, gint y,
    gint w, gint h)
{
  gboolean result = FALSE;

  g_return_val_if_fail (GST_IS_COMPOSITE (composite), FALSE);

  GST_COMPOSITE_LOCK (composite);
  if (composite->adjusting) {
    WARN ("last PIP adjustment request is progressing");
    goto end;
  }
  
  INFO ("adjust-pip: %d, %d, %d, %d", x, y, w, h);

  composite->b_x = x;
  composite->b_y = y;
  composite->b_width = w;
  composite->b_height = h;
  composite->adjusting = TRUE;
  gst_worker_stop (GST_WORKER (composite));

  result = TRUE;

 end:
  GST_COMPOSITE_UNLOCK (composite);

  return result;
}

gboolean
gst_composite_new_record (GstComposite *composite)
{
  gboolean result = FALSE;
  if (composite->recorder) {
    GST_COMPOSITE_LOCK_RECORDER (composite);
    if (composite->recorder) {
      gst_worker_stop (GST_WORKER (composite->recorder));
      g_object_set (G_OBJECT (composite->recorder),
	  "mode", composite->mode, "port", composite->encode_sink_port,
	  "width", composite->width, "height", composite->height, NULL);
      result = gst_worker_start (GST_WORKER (composite->recorder));
    }
    GST_COMPOSITE_UNLOCK_RECORDER (composite);
  }
  return result;
}

static void
gst_composite_error (GstComposite *composite)
{
  g_return_if_fail (GST_IS_COMPOSITE (composite));

  if (composite->transition) {
    GST_COMPOSITE_LOCK_TRANSITION (composite);
    if (composite->transition) {
      gboolean ok1, ok2, ok3;
      WARN ("new mode %d, %dx%d transition error",
	  composite->mode, composite->width, composite->height);
      ok1 = gst_worker_stop_force (GST_WORKER (composite), TRUE);
      ok2 = gst_worker_stop_force (GST_WORKER (composite->output), TRUE);
      ok3 = gst_worker_stop_force (GST_WORKER (composite->recorder), TRUE);
      //INFO ("stop: %d, %d, %d", ok1, ok2, ok3);
      (void) ok1, (void) ok2, (void) ok3;
      gst_composite_apply_parameters (composite);
    }
    GST_COMPOSITE_UNLOCK_TRANSITION (composite);
  }

  /*
  if (composite->adjusting) {
    composite->adjusting = FALSE;
  }
  */
}

static gboolean
gst_composite_message (GstComposite *composite, GstMessage * message)
{
  switch (GST_MESSAGE_TYPE (message)) {
  case GST_MESSAGE_ERROR:
    gst_composite_error (composite);
    break;
  default: break;
  }
  return TRUE;
}

static void
gst_composite_class_init (GstCompositeClass * klass)
{
  GObjectClass * object_class = G_OBJECT_CLASS (klass);
  GstWorkerClass * worker_class = GST_WORKER_CLASS (klass);

  object_class->dispose = (GObjectFinalizeFunc) gst_composite_dispose;
  object_class->finalize = (GObjectFinalizeFunc) gst_composite_finalize;
  object_class->set_property = (GObjectSetPropertyFunc) gst_composite_set_property;
  object_class->get_property = (GObjectGetPropertyFunc) gst_composite_get_property;

  gst_composite_signals[SIGNAL_START_OUTPUT] = 
    g_signal_new ("start-output", G_TYPE_FROM_CLASS (klass),
	G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (GstCompositeClass, start_output),
	NULL, NULL, g_cclosure_marshal_generic, G_TYPE_NONE, 0);

  gst_composite_signals[SIGNAL_START_RECORDER] = 
    g_signal_new ("start-recorder", G_TYPE_FROM_CLASS (klass),
	G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (GstCompositeClass, start_recorder),
	NULL, NULL, g_cclosure_marshal_generic, G_TYPE_NONE, 0);

  gst_composite_signals[SIGNAL_END_OUTPUT] = 
    g_signal_new ("end-output", G_TYPE_FROM_CLASS (klass),
	G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (GstCompositeClass, end_output),
	NULL, NULL, g_cclosure_marshal_generic, G_TYPE_NONE, 0);

  gst_composite_signals[SIGNAL_END_RECORDER] = 
    g_signal_new ("end-recorder", G_TYPE_FROM_CLASS (klass),
	G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (GstCompositeClass, end_recorder),
	NULL, NULL, g_cclosure_marshal_generic, G_TYPE_NONE, 0);

  g_object_class_install_property (object_class, PROP_MODE,
      g_param_spec_uint ("mode", "Mode", "Composite Mode",
          COMPOSE_MODE_0, COMPOSE_MODE__LAST, DEFAULT_COMPOSE_MODE,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_PORT,
      g_param_spec_uint ("port", "Port", "Sink port",
          GST_SWITCH_MIN_SINK_PORT, GST_SWITCH_MAX_SINK_PORT,
	  GST_SWITCH_MIN_SINK_PORT,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_ENCODE_PORT,
      g_param_spec_uint ("encode", "EncodePort", "Encoding Sink port",
          GST_SWITCH_MIN_SINK_PORT, GST_SWITCH_MAX_SINK_PORT,
	  GST_SWITCH_MIN_SINK_PORT,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_A_X,
      g_param_spec_uint ("ax", "A xpos", "Channel A frame xpos",
          0, G_MAXINT, 0,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_A_Y,
      g_param_spec_uint ("ay", "A ypos", "Channel A frame ypos",
          0, G_MAXINT, 0,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_A_WIDTH,
      g_param_spec_uint ("awidth", "A Width", "Channel A frame width",
          1, G_MAXINT, GST_SWITCH_COMPOSITE_DEFAULT_WIDTH,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_A_HEIGHT,
      g_param_spec_uint ("aheight", "A Height", "Channel A frame height",
          1, G_MAXINT, GST_SWITCH_COMPOSITE_DEFAULT_HEIGHT,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_B_X,
      g_param_spec_uint ("bx", "B xpos", "Channel B frame xpos",
          0, G_MAXINT, 0,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_B_Y,
      g_param_spec_uint ("by", "B ypos", "Channel B frame ypos",
          0, G_MAXINT, 0,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_B_WIDTH,
      g_param_spec_uint ("bwidth", "B Width", "Channel B frame width",
          1, G_MAXINT, GST_SWITCH_COMPOSITE_DEFAULT_WIDTH,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_B_HEIGHT,
      g_param_spec_uint ("bheight", "B Height", "Channel B frame height",
          1, G_MAXINT, GST_SWITCH_COMPOSITE_DEFAULT_HEIGHT,
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_WIDTH,
      g_param_spec_uint ("width", "Composite Width", "Output frame width",
          1, G_MAXINT, GST_SWITCH_COMPOSITE_DEFAULT_WIDTH,
	  G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_HEIGHT,
      g_param_spec_uint ("height", "Composite Height", "Output frame height",
          1, G_MAXINT, GST_SWITCH_COMPOSITE_DEFAULT_HEIGHT,
	  G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  worker_class->alive = (GstWorkerAliveFunc) gst_composite_alive;
  worker_class->null = (GstWorkerNullFunc) gst_composite_null;
  worker_class->prepare = (GstWorkerPrepareFunc) gst_composite_prepare;
  worker_class->message = (GstWorkerMessageFunc) gst_composite_message;
  worker_class->get_pipeline_string = (GstWorkerGetPipelineStringFunc)
    gst_composite_get_pipeline_string;
}
