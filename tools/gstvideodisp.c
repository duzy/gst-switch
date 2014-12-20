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

/*! @file */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstvideodisp.h"
#include "gstswitchserver.h"
#include <gst/video/videooverlay.h>

#define parent_class gst_video_disp_parent_class

enum
{
  PROP_0,
  PROP_PORT,
  PROP_HANDLE,
};

extern gboolean verbose;

G_DEFINE_TYPE (GstVideoDisp, gst_video_disp, GST_TYPE_WORKER);

/**
 * @brief
 * @param disp The GstVideoDisp instance.
 * @memberof GstVideoDisp
 */
static void
gst_video_disp_init (GstVideoDisp * disp)
{
  //INFO ("init %p", disp);
}

/**
 * @brief
 * @param disp The GstVideoDisp instance.
 * @memberof GstVideoDisp
 */
static void
gst_video_disp_dispose (GstVideoDisp * disp)
{
  //INFO ("dispose %p", disp);
  G_OBJECT_CLASS (parent_class)->dispose (G_OBJECT (disp));
}

/**
 * @brief
 * @param disp The GstVideoDisp instance.
 * @memberof GstVideoDisp
 */
static void
gst_video_disp_finalize (GstVideoDisp * disp)
{
  if (G_OBJECT_CLASS (parent_class)->finalize)
    (*G_OBJECT_CLASS (parent_class)->finalize) (G_OBJECT (disp));
}

/**
 * @brief
 * @param disp The GstVideoDisp instance.
 * @param property_id
 * @param value
 * @param pspec
 * @memberof GstVideoDisp
 */
static void
gst_video_disp_set_property (GstVideoDisp * disp, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  switch (property_id) {
    case PROP_PORT:
      disp->port = g_value_get_uint (value);
      break;
    case PROP_HANDLE:
      disp->handle = g_value_get_ulong (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (disp), property_id, pspec);
      break;
  }
}

/**
 * @brief
 * @param disp The GstVideoDisp instance.
 * @param property_id
 * @param value
 * @param pspec
 * @memberof GstVideoDisp
 */
static void
gst_video_disp_get_property (GstVideoDisp * disp, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  switch (property_id) {
    case PROP_PORT:
      g_value_set_uint (value, disp->port);
      break;
    case PROP_HANDLE:
      g_value_set_ulong (value, disp->handle);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (disp), property_id, pspec);
      break;
  }
}

/**
 * @brief
 * @param disp The GstVideoDisp instance.
 * @memberof GstVideoDisp
 */
static GString *
gst_video_disp_get_pipeline_string (GstVideoDisp * disp)
{
  GString *desc;

  INFO ("display video %d", disp->port);

  desc = g_string_new ("");

  g_string_append_printf (desc, "tcpclientsrc name=source "
      "port=%d ", disp->port);
  g_string_append_printf (desc, "! gdpdepay ");
  g_string_append_printf (desc, "! videoconvert ");
  g_string_append_printf (desc, "! cairooverlay name=overlay ");
  g_string_append_printf (desc, "! videoconvert ");
  g_string_append_printf (desc, "! xvimagesink name=sink sync=false ");

  return desc;
}

/**
 * @brief
 * @param disp The GstVideoDisp instance.
 * @memberof GstVideoDisp
 */
static gboolean
gst_video_disp_prepare (GstVideoDisp * disp)
{
  GstWorker *worker = GST_WORKER (disp);
  GstElement *sink = gst_worker_get_element_unlocked (worker, "sink");

  g_return_val_if_fail (GST_IS_ELEMENT (sink), FALSE);

  gst_video_overlay_set_window_handle (GST_VIDEO_OVERLAY (sink), disp->handle);

  gst_object_unref (sink);

  //INFO ("prepared display video on %ld", disp->handle);
  return TRUE;
}

/**
 * @brief
 * @param klass The GstVideoDispClass instance.
 * @memberof GstVideoDispClass
 */
static void
gst_video_disp_class_init (GstVideoDispClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstWorkerClass *worker_class = GST_WORKER_CLASS (klass);

  object_class->dispose = (GObjectFinalizeFunc) gst_video_disp_dispose;
  object_class->finalize = (GObjectFinalizeFunc) gst_video_disp_finalize;
  object_class->set_property =
      (GObjectSetPropertyFunc) gst_video_disp_set_property;
  object_class->get_property =
      (GObjectGetPropertyFunc) gst_video_disp_get_property;

  g_object_class_install_property (object_class, PROP_PORT,
      g_param_spec_uint ("port", "Port",
          "Sink port",
          GST_SWITCH_MIN_SINK_PORT,
          GST_SWITCH_MAX_SINK_PORT,
          GST_SWITCH_MIN_SINK_PORT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_HANDLE,
      g_param_spec_ulong ("handle", "Handle",
          "Window Handle", 0,
          ((gulong) - 1), 0, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  worker_class->prepare = (GstWorkerPrepareFunc) gst_video_disp_prepare;
  worker_class->get_pipeline_string = (GstWorkerGetPipelineStringFunc)
      gst_video_disp_get_pipeline_string;
}
