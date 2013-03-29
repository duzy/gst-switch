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

#include "gstworker.h"
#include <unistd.h>

gboolean verbose = FALSE;

/**
 *  @brief Get the pipeline string of camera capturing.
 *  @param worker The GstWorker instance.
 *  @param data Caller defined data.
 */
static GString *
gst_switch_capture_pipeline (GstWorker * worker, gpointer data)
{
  GString *desc;

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
      "! videoscale ! video/x-raw,width=300,height=200 ");
  g_string_append_printf (desc, "! videoconvert ");
  g_string_append_printf (desc, "! facedetect2 ! speakertrack ");
  g_string_append_printf (desc, "! videoconvert ");
  g_string_append_printf (desc, "! xvimagesink ");

  g_string_append_printf (desc, "video. ! queue2 ");
  g_string_append_printf (desc, "! videoconvert ");
  g_string_append_printf (desc, "! fakesink ");

  return desc;
}

/**
 *  @deprecated Not used!
 */
static gpointer
gst_switch_capture_pulse (GstWorker * worker)
{
  while (GST_IS_WORKER (worker)) {
    usleep (1000 * 1);
    //INFO ("tick: %d", transition);
  }

  INFO ("end pulse");
  return NULL;
}

/**
 *  @brief Callback function of prepare-worker signal.
 *  @param worker The GstWorker instance.
 *  @param data Caller defined data.
 */
static void
gst_switch_capture_prepare (GstWorker * worker, gpointer data)
{
  g_return_if_fail (GST_IS_WORKER (worker));

  INFO ("%s: %s", worker->name, __FUNCTION__);
}

/**
 *  @brief Callback function of start-worker signal.
 *  @param worker The GstWorker instance.
 *  @param data Caller defined data.
 */
static void
gst_switch_capture_start (GstWorker * worker, gpointer data)
{
  g_return_if_fail (GST_IS_WORKER (worker));

  INFO ("%s: %s", worker->name, __FUNCTION__);
}

/**
 *  @brief Callback function of end-worker signal.
 *  @param worker The GstWorker instance.
 *  @param data Caller defined data.
 */
static void
gst_switch_capture_end (GstWorker * worker, gpointer data)
{
  g_return_if_fail (GST_IS_WORKER (worker));

  INFO ("%s: %s", worker->name, __FUNCTION__);
}

/**
 *  @brief The entry of gst-switch-cap.
 */
int
main (int argc, char **argv)
{
  gst_init (&argc, &argv);

  GMainLoop *mainloop = g_main_loop_new (NULL, TRUE);
  GstWorker *worker0 = GST_WORKER (g_object_new (GST_TYPE_WORKER,
          "name", "capture", NULL));

  worker0->pipeline_func = gst_switch_capture_pipeline;

  g_signal_connect (worker0, "prepare-worker",
      G_CALLBACK (gst_switch_capture_prepare), NULL);
  g_signal_connect (worker0, "start-worker",
      G_CALLBACK (gst_switch_capture_start), NULL);
  g_signal_connect (worker0, "end-worker",
      G_CALLBACK (gst_switch_capture_end), NULL);

  g_thread_new ("capture-pulse", (GThreadFunc) gst_switch_capture_pulse,
      worker0);

  gst_worker_start (worker0);

  g_main_loop_run (mainloop);

  INFO ("end");
  return 0;
}
