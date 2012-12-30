/* GstCompositor
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

G_DEFINE_TYPE (GstCompositor, gst_compositor, GST_WORKER_TYPE);

static void
gst_compositor_init (GstCompositor * compositor)
{
}

static void
gst_compositor_finalize (GstCompositor * compositor)
{
}

static GstElement *
gst_compositor_create_pipeline (GstCompositor * compositor)
{
  GString *desc;
  GstElement *pipeline;
  GError *error = NULL;

  INFO ("Listenning on port %d", opts.port);

  desc = g_string_new ("");

#if 0
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
#else
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
    GstCompositor * compositor)
{
  INFO ("source-pad-added: %s.%s", GST_ELEMENT_NAME (element),
      GST_PAD_NAME (pad));
}

static gboolean
gst_compositor_prepare (GstCompositor *compositor)
{
  GstWorker *worker = GST_WORKER (compositor);
  if (worker->source) {
    g_signal_connect (worker->source, "pad-added",
	G_CALLBACK (on_source_pad_added), worker);
  }
  return TRUE;
}

static void
gst_compositor_class_init (GstCompositorClass * klass)
{
  GObjectClass * object_class = G_OBJECT_CLASS (klass);
  GstWorkerClass * worker_class = GST_WORKER_CLASS (klass);
  object_class->finalize = (GObjectFinalizeFunc) gst_compositor_finalize;
  worker_class->create_pipeline = (GstWorkerCreatePipelineFunc)
    gst_compositor_create_pipeline;
  worker_class->prepare = (GstWorkerPrepareFunc) gst_compositor_prepare;
}
