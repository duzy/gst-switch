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

/*! @file */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstswitchcapture.h"
#include "gstcomposite.h"
#include <unistd.h>
#include <stdlib.h>
#include "../logutils.h"

#define GST_TYPE_SWITCH_CAPTURE_WORKER (gst_switch_capture_worker_get_type ())
#define GST_SWITCH_CAPTURE_WORKER(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), GST_TYPE_SWITCH_CAPTURE_WORKER, GstSwitchCaptureWorker))
#define GST_SWITCH_CAPTURE_WORKER_CLASS(class) (G_TYPE_CHECK_CLASS_CAST ((class), GST_TYPE_SWITCH_CAPTURE_WORKER, GstSwitchCaptureWorkerClass))
#define GST_IS_SWITCH_CAPTURE_WORKER(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), GST_TYPE_SWITCH_CAPTURE_WORKER))
#define GST_IS_SWITCH_CAPTURE_WORKER_CLASS(class) (G_TYPE_CHECK_CLASS_TYPE ((class), GST_TYPE_SWITCH_CAPTURE_WORKER))

/**
 * @class GstSwitchCaptureWorker
 * @struct _GstSwitchCaptureWorker
 * @brief The worker for capturing from camera.
 */
typedef struct _GstSwitchCaptureWorker
{
  GstWorker base;
  GstSwitchCapture *capture;
} GstSwitchCaptureWorker;

/**
 * @class GstSwitchCaptureWorkerClass
 * @struct _GstSwitchCaptureWorkerClass
 * @brief The class of GstSwitchCaptureWorker
 */
typedef struct _GstSwitchCaptureWorkerClass
{
  GstWorkerClass base_class;
} GstSwitchCaptureWorkerClass;

GType gst_switch_capture_worker_get_type (void);

G_DEFINE_TYPE (GstSwitchCapture, gst_switch_capture, GST_TYPE_SWITCH_CLIENT);
G_DEFINE_TYPE (GstSwitchCaptureWorker, gst_switch_capture_worker,
    GST_TYPE_WORKER);

gboolean verbose = FALSE;

static GOptionEntry options[] = {
  {"verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Be verbose", NULL},
  {NULL}
};

/**
 * @brief Parse command line arguments.
 */
static void
gst_switch_capture_parse_args (int *argc, char **argv[])
{
  GOptionContext *context;
  GError *error = NULL;
  context = g_option_context_new ("");
  g_option_context_add_main_entries (context, options, "gst-switch-cap");
  g_option_context_add_group (context, gst_init_get_option_group ());
  if (!g_option_context_parse (context, argc, argv, &error)) {
    g_print ("option parsing failed: %s\n", error->message);
    exit (1);
  }
  g_option_context_free (context);
}

/**
 *  @brief Get the pipeline string of camera capturing.
 *  @param worker The GstWorker instance.
 *  @param data Caller defined data.
 *  @memberof GstSwitchCapture
 */
static GString *
gst_switch_capture_pipeline (GstWorker * worker, GstSwitchCapture * capture)
{
  GString *desc;
  guint w = GST_SWITCH_COMPOSITE_DEFAULT_WIDTH;
  guint h = GST_SWITCH_COMPOSITE_DEFAULT_HEIGHT;

  desc = g_string_new ("");

  if (0) {
    g_string_append_printf (desc,
        "filesrc location=\"/home/duzy/Downloads/locked-mode-7010.dv\" ");
    g_string_append_printf (desc, "! dvdemux ! dvdec ");
  } else {
    g_string_append_printf (desc, "v4l2src ! tee name=video ");
  }

  g_string_append_printf (desc, "video. ! queue2 ");
  g_string_append_printf (desc,
      "! videoscale ! video/x-raw,width=%d,height=%d ",
      GST_SWITCH_FACEDETECT_FRAME_WIDTH, GST_SWITCH_FACEDETECT_FRAME_HEIGHT);
  g_string_append_printf (desc, "! videoconvert ");
  g_string_append_printf (desc, "! facedetect2 ! speakertrack name=tracker ");
  g_string_append_printf (desc, "! videoconvert ");
  g_string_append_printf (desc, "! xvimagesink ");

  g_string_append_printf (desc, "video. ! queue2 ");
  g_string_append_printf (desc, "! videoconvert ");
  g_string_append_printf (desc, "! videoscale "
      "! video/x-raw,format=I420,width=%d,height=%d ", w, h);
  g_string_append_printf (desc, "! gdppay ! tcpclientsink port=%d ", 3000);

  return desc;
}

/**
 *  @deprecated Not used!
 */
static gpointer
gst_switch_capture_pulse (GstSwitchCapture * capture)
{
  while (GST_IS_SWITCH_CAPTURE (capture)) {
    usleep (1000 * 1);
  }

  INFO ("end pulse");
  return NULL;
}

/**
 *  @brief Callback function of prepare-worker signal.
 *  @param worker The GstWorker instance.
 *  @param data Caller defined data.
 *  @memberof GstSwitchCapture
 */
static void
gst_switch_capture_prepare (GstWorker * worker, GstSwitchCapture * capture)
{
  g_return_if_fail (GST_IS_WORKER (worker));

  INFO ("%s: %s", worker->name, __FUNCTION__);
}

/**
 *  @brief Callback function of start-worker signal.
 *  @param worker The GstWorker instance.
 *  @param data Caller defined data.
 *  @memberof GstSwitchCapture
 */
static void
gst_switch_capture_start (GstWorker * worker, GstSwitchCapture * capture)
{
  g_return_if_fail (GST_IS_WORKER (worker));

  INFO ("%s: %s", worker->name, __FUNCTION__);
}

/**
 *  @brief Callback function of end-worker signal.
 *  @param worker The GstWorker instance.
 *  @param data Caller defined data.
 *  @memberof GstSwitchCapture
 */
static void
gst_switch_capture_end (GstWorker * worker, GstSwitchCapture * capture)
{
  g_return_if_fail (GST_IS_WORKER (worker));

  INFO ("%s: %s", worker->name, __FUNCTION__);
}

/**
 *  @brief Callback function of end-worker signal.
 *  @param worker The GstWorker instance.
 *  @param data Caller defined data.
 *  @memberof GstSwitchCapture
 */
static void
gst_switch_capture_run (GstSwitchCapture * capture)
{
  if (!gst_switch_client_connect (GST_SWITCH_CLIENT (capture),
          CLIENT_ROLE_CAPTURE)) {
    ERROR ("failed to connect to controller");
    return;
  }
  gst_worker_start (GST_WORKER (capture->worker));
  g_main_loop_run (capture->mainloop);
}

/**
 * @brief
 * @param ui The GstSwitchUI instance.
 * @param error
 * @memberof GstSwitchUI
 */
static void
gst_switch_capture_on_controller_closed (GstSwitchCapture * capture,
    GError * error)
{
  g_main_loop_quit (capture->mainloop);
}

/**
 * @brief Initialize GstSwitchCapture instances.
 * @param capture The GstSwitchCapture instance.
 * @memberof GstSwitchCapture
 */
static void
gst_switch_capture_init (GstSwitchCapture * capture)
{
  capture->mainloop = g_main_loop_new (NULL, TRUE);
  capture->worker =
      GST_SWITCH_CAPTURE_WORKER (g_object_new (GST_TYPE_SWITCH_CAPTURE_WORKER,
          "name", "capture", NULL));

  capture->worker->capture = capture;

  capture->worker->base.pipeline_func =
      (GstWorkerGetPipelineString) gst_switch_capture_pipeline;
  capture->worker->base.pipeline_func_data = capture;

  g_signal_connect (capture->worker, "prepare-worker",
      G_CALLBACK (gst_switch_capture_prepare), capture);
  g_signal_connect (capture->worker, "start-worker",
      G_CALLBACK (gst_switch_capture_start), capture);
  g_signal_connect (capture->worker, "end-worker",
      G_CALLBACK (gst_switch_capture_end), capture);

  capture->pulse = g_thread_new ("capture-pulse",
      (GThreadFunc) gst_switch_capture_pulse, capture);
}

/**
 * @brief Initialize GstSwitchCapture instances.
 * @param capture The GstSwitchCapture instance.
 * @memberof GstSwitchCapture
 */
static void
gst_switch_capture_finalize (GstSwitchCapture * capture)
{
  g_object_unref (capture->mainloop);
  g_object_unref (capture->worker);
  g_thread_unref (capture->pulse);

  capture->mainloop = NULL;
  capture->worker = NULL;
  capture->pulse = NULL;

  if (G_OBJECT_CLASS (gst_switch_capture_parent_class)->finalize)
    (*G_OBJECT_CLASS (gst_switch_capture_parent_class)->finalize) (G_OBJECT
        (capture));
}

static void
gst_switch_capture_select_face (GstSwitchCapture * capture, gint x, gint y)
{
  GstWorker *worker = GST_WORKER (capture->worker);
  GstElement *element = gst_worker_get_element (worker, "tracker");
  if (element) {
    GstStructure *st = gst_structure_new ("select",
        "x", G_TYPE_INT, x, "y", G_TYPE_INT, y, NULL);
    GstEvent *ev = gst_event_new_custom (GST_EVENT_CUSTOM_DOWNSTREAM, st);
    gboolean okay = gst_element_send_event (element, ev);
    INFO ("select face: %d, %d (selected: %d)", x, y, okay);
  } else {
    ERROR ("select face: %d, %d (No tracker!)", x, y);
  }
}

static void
gst_switch_capture_worker_init (GstSwitchCaptureWorker * captureworker)
{
}

static void
gst_switch_capture_worker_finalize (GstSwitchCaptureWorker * captureworker)
{
  const GObjectClass *object_class =
      G_OBJECT_CLASS (gst_switch_capture_worker_parent_class);
  if (object_class->finalize)
    (*object_class->finalize) (G_OBJECT (captureworker));
}

static void
gst_switch_capture_worker_error (GstSwitchCaptureWorker * captureworker,
    GError * error, const char *debug)
{
  ERROR ("%d: %s", error->code, error->message);
}

/*
static void
gst_switch_capture_worker_mark_face_remote (GstSwitchCaptureWorker *
    captureworker, guint x, guint y, guint w, guint h)
{
  gboolean okay =
      gst_switch_client_mark_face_remotely (GST_SWITCH_CLIENT
      (captureworker->capture), x, y, w, h);
  g_print ("mark: face[(%d,%d), %d,%d] (okay: %d)\n", x, y, w, h, okay);
}
*/

static void
gst_switch_capture_worker_faces_detected (GstSwitchCaptureWorker *
    captureworker, const GValue * faces, GstMessage * message)
{
  const guint size = gst_value_list_get_size (faces);
  GVariantBuilder *vb = g_variant_builder_new (G_VARIANT_TYPE_ARRAY);
  gboolean okay;
  int n;

  //g_print ("%s: faces=%d\n", GST_MESSAGE_SRC_NAME (message), size);

  for (n = 0; n < size; ++n) {
    const GValue *face_value = gst_value_list_get_value (faces, n);
    if (G_VALUE_HOLDS_BOXED (face_value)) {
      GstStructure *face = (GstStructure *) g_value_get_boxed (face_value);
      if (GST_IS_STRUCTURE (face)) {
        guint x, y, w, h;
        gst_structure_get_uint (face, "x", &x);
        gst_structure_get_uint (face, "y", &y);
        gst_structure_get_uint (face, "width", &w);
        gst_structure_get_uint (face, "height", &h);
        g_print ("%s: face[%d]=[(%d,%d), %d,%d]\n",
            GST_MESSAGE_SRC_NAME (message), n, x, y, w, h);
        //gst_switch_capture_worker_mark_face_remote (captureworker, x, y, w, h);
        g_variant_builder_add (vb, "(iiii)", x, y, w, h);
      }
    }
  }

  okay = gst_switch_client_mark_face_remotely (GST_SWITCH_CLIENT
      (captureworker->capture), g_variant_builder_end (vb));
  g_variant_builder_unref (vb);

  g_print ("mark: faces (%d) (okay: %d)\n", size, okay);
}

static gboolean
gst_switch_capture_worker_message (GstSwitchCaptureWorker * captureworker,
    GstMessage * message)
{
  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR:{
      GError *error = NULL;
      gchar *debug = NULL;
      gst_message_parse_error (message, &error, &debug);
      gst_switch_capture_worker_error (captureworker, error, debug);
    }
      break;

    case GST_MESSAGE_ELEMENT:{
      const GstStructure *st = (const GstStructure *)
          gst_message_get_structure (message);
      const GValue *faces = (const GValue *)
          gst_structure_get_value (st, "faces");
      if (faces)
        gst_switch_capture_worker_faces_detected (captureworker, faces,
            message);
    }
      break;

    default:
      break;
  }
  return TRUE;
}

/**
 * @brief
 * @param klass
 * @memberof GstSwitchUIClass
 */
static void
gst_switch_capture_class_init (GstSwitchCaptureClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstSwitchClientClass *client_class = GST_SWITCH_CLIENT_CLASS (klass);

  object_class->finalize = (GObjectFinalizeFunc) gst_switch_capture_finalize;

  client_class->connection_closed = (GstSwitchClientConnectionClosedFunc)
      gst_switch_capture_on_controller_closed;
  client_class->select_face = (GstSwitchClientSelectFaceFunc)
      gst_switch_capture_select_face;
}

/**
 * @brief
 * @param klass
 * @memberof GstSwitchCaptureWorkerClass
 */
static void
gst_switch_capture_worker_class_init (GstSwitchCaptureWorkerClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstWorkerClass *worker_class = GST_WORKER_CLASS (klass);

  object_class->finalize =
      (GObjectFinalizeFunc) gst_switch_capture_worker_finalize;

  worker_class->message = (GstWorkerMessageFunc)
      gst_switch_capture_worker_message;
  //worker_class->get_pipeline_string = (GstWorkerGetPipelineStringFunc)
  //    gst_switch_capture_worker_pipeline;
}

/**
 *  @brief The entry of gst-switch-cap.
 */
int
main (int argc, char **argv)
{
  GstSwitchCapture *capture = NULL;
  gst_switch_capture_parse_args (&argc, &argv);
  gst_init (&argc, &argv);

  capture = GST_SWITCH_CAPTURE (g_object_new (GST_TYPE_SWITCH_CAPTURE, NULL));

  gst_switch_capture_run (capture);

  g_object_unref (G_OBJECT (capture));
  return 0;
}
