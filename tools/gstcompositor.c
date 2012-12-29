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

void
gst_compositor_init (GstCompositor * compositor, GstSwitcher *switcher)
{
  memset (compositor, 0, sizeof (GstCompositor));
  compositor->switcher = switcher;
}

void
gst_compositor_fini (GstCompositor * compositor)
{
  if (compositor->source_element) {
    gst_object_unref (compositor->source_element);
    compositor->source_element = NULL;
  }
  if (compositor->sink_element) {
    gst_object_unref (compositor->sink_element);
    compositor->sink_element = NULL;
  }
  if (compositor->pipeline) {
    gst_element_set_state (compositor->pipeline, GST_STATE_NULL);
    gst_object_unref (compositor->pipeline);
    compositor->pipeline = NULL;
  }
  g_free (compositor);
}

GstElement *
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

static gboolean onesecond_timer (gpointer priv);

void
gst_compositor_start (GstCompositor * compositor)
{
  gst_element_set_state (compositor->pipeline, GST_STATE_READY);

  compositor->timer_id = g_timeout_add (1000, onesecond_timer, compositor);
}

static void
gst_compositor_stop (GstCompositor * compositor)
{
  gst_element_set_state (compositor->pipeline, GST_STATE_NULL);

  g_source_remove (compositor->timer_id);
}

static void
gst_compositor_handle_eos (GstCompositor * compositor)
{
  gst_compositor_stop (compositor);
}

static void
gst_compositor_handle_error (GstCompositor * compositor, GError * error,
    const char *debug)
{
  ERROR ("%s", error->message);
  gst_compositor_stop (compositor);
}

static void
gst_compositor_handle_warning (GstCompositor * compositor, GError * error,
    const char *debug)
{
  WARN ("%s", error->message);
}

static void
gst_compositor_handle_info (GstCompositor * compositor, GError * error,
    const char *debug)
{
  INFO ("%s", error->message);
}

static void
gst_compositor_handle_null_to_ready (GstCompositor * compositor,
    GstElement * pipeline)
{
  gst_element_set_state (pipeline, GST_STATE_PAUSED);
}

static void
gst_compositor_handle_ready_to_paused (GstCompositor * compositor,
    GstElement * pipeline)
{
  if (!compositor->paused_for_buffering) {
    gst_element_set_state (pipeline, GST_STATE_PLAYING);
  }
}

static void
gst_compositor_handle_paused_to_playing (GstCompositor * compositor,
    GstElement * pipeline)
{
}

static void
gst_compositor_handle_playing_to_paused (GstCompositor * compositor,
    GstElement * pipeline)
{
}

static void
gst_compositor_handle_paused_to_ready (GstCompositor * compositor,
    GstElement * pipeline)
{
}

static void
gst_compositor_handle_ready_to_null (GstCompositor * compositor,
    GstElement * pipeline)
{
  //g_main_loop_quit (compositor->main_loop);
}

static gboolean
gst_compositor_handle_pipeline_state_changed (GstCompositor * compositor,
    GstElement * pipeline, GstStateChange statechange)
{
  switch (statechange) {
  case GST_STATE_CHANGE_NULL_TO_READY:
    gst_compositor_handle_null_to_ready (compositor, pipeline);
    break;
  case GST_STATE_CHANGE_READY_TO_PAUSED:
    gst_compositor_handle_ready_to_paused (compositor, pipeline);
    break;
  case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
    gst_compositor_handle_paused_to_playing (compositor, pipeline);
    break;
  case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
    gst_compositor_handle_playing_to_paused (compositor, pipeline);
    break;
  case GST_STATE_CHANGE_PAUSED_TO_READY:
    gst_compositor_handle_paused_to_ready (compositor, pipeline);
    break;
  case GST_STATE_CHANGE_READY_TO_NULL:
    gst_compositor_handle_ready_to_null (compositor, pipeline);
    break;
  default:
    return FALSE;
  }
  return TRUE;
}

static gboolean
gst_compositor_handle_message (GstBus * bus, GstMessage * message, gpointer data)
{
  GstCompositor *compositor = (GstCompositor *) data;

  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_EOS:
      gst_compositor_handle_eos (compositor);
      break;
    case GST_MESSAGE_ERROR:
    {
      GError *error = NULL;
      gchar *debug;

      gst_message_parse_error (message, &error, &debug);
      gst_compositor_handle_error (compositor, error, debug);
    } break;
    case GST_MESSAGE_WARNING:
    {
      GError *error = NULL;
      gchar *debug;

      gst_message_parse_warning (message, &error, &debug);
      gst_compositor_handle_warning (compositor, error, debug);
    } break;
    case GST_MESSAGE_INFO:
    {
      GError *error = NULL;
      gchar *debug;

      gst_message_parse_info (message, &error, &debug);
      gst_compositor_handle_info (compositor, error, debug);
    } break;
    case GST_MESSAGE_TAG:
    {
      GstTagList *tag_list;

      gst_message_parse_tag (message, &tag_list);
      if (opts.verbose)
        g_print ("tag\n");
    } break;
    case GST_MESSAGE_STATE_CHANGED:
    {
      gboolean ret;
      GstState oldstate, newstate, pending;
      gst_message_parse_state_changed (message, &oldstate, &newstate, &pending);
      if (GST_ELEMENT (message->src) == compositor->pipeline) {
        if (opts.verbose)
          g_print ("switch-server: state change from %s to %s\n",
              gst_element_state_get_name (oldstate),
              gst_element_state_get_name (newstate));

	ret = gst_compositor_handle_pipeline_state_changed (compositor,
	    GST_ELEMENT (message->src),
	    GST_STATE_TRANSITION (oldstate, newstate));

	if (!ret && opts.verbose)
	  g_print ("unknown state change from %s to %s\n",
	      gst_element_state_get_name (oldstate),
	      gst_element_state_get_name (newstate));
      }
    } break;
    case GST_MESSAGE_BUFFERING:
    {
      int percent;
      gst_message_parse_buffering (message, &percent);
      //g_print("buffering %d\n", percent);
      if (!compositor->paused_for_buffering && percent < 100) {
        g_print ("pausing for buffing\n");
        compositor->paused_for_buffering = TRUE;
        gst_element_set_state (compositor->pipeline, GST_STATE_PAUSED);
      } else if (compositor->paused_for_buffering && percent == 100) {
        g_print ("unpausing for buffing\n");
        compositor->paused_for_buffering = FALSE;
        gst_element_set_state (compositor->pipeline, GST_STATE_PLAYING);
      }
    } break;
    default:
      if (opts.verbose) {
        //g_print ("message: %s\n", GST_MESSAGE_TYPE_NAME (message));
      }
      break;
  }

  return TRUE;
}

static gboolean
onesecond_timer (gpointer priv)
{
  //GstCompositor *compositor = (GstCompositor *)priv;

  //g_print (".\n");

  return TRUE;
}

static void
on_source_pad_added (GstElement * element, GstPad * pad,
    GstCompositor * compositor)
{
  INFO ("source-pad-added: %s.%s", GST_ELEMENT_NAME (element),
      GST_PAD_NAME (pad));
}

void
gst_compositor_set_pipeline (GstCompositor * compositor, GstElement *pipeline)
{
  compositor->pipeline = pipeline;

  gst_pipeline_set_auto_flush_bus (GST_PIPELINE (pipeline), FALSE);
  compositor->bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (compositor->bus, gst_compositor_handle_message, compositor);

  compositor->source_element = gst_bin_get_by_name (GST_BIN (pipeline), "source");
  compositor->sink_element = gst_bin_get_by_name (GST_BIN (pipeline), "sink");

  if (compositor->source_element) {
    g_signal_connect (compositor->source_element, "pad-added",
	G_CALLBACK (on_source_pad_added), compositor);
  }
}

gpointer gst_compositor_run (GstCompositor *compositor)
{
  GstElement *pipeline = gst_compositor_create_pipeline (compositor);

  gst_compositor_set_pipeline (compositor, pipeline);
  gst_compositor_start (compositor);

  compositor->main_loop = g_main_loop_new (NULL, TRUE);

  g_main_loop_run (compositor->main_loop);
  return NULL;
}
