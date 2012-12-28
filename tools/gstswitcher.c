/* GstSwitcher
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
#include "gstswitchsrv.h"

void
gst_switcher_init (GstSwitcher *switcher, GstCompositor * compositor)
{
  memset (switcher, 0, sizeof (GstSwitcher));
  switcher->compositor = compositor;
}

void
gst_switcher_fini (GstSwitcher * switcher)
{
  if (switcher->source_element) {
    gst_object_unref (switcher->source_element);
    switcher->source_element = NULL;
  }
  if (switcher->sink_element) {
    gst_object_unref (switcher->sink_element);
    switcher->sink_element = NULL;
  }
  if (switcher->switch_element) {
    gst_object_unref (switcher->switch_element);
    switcher->switch_element = NULL;
  }
  if (switcher->conv_element) {
    gst_object_unref (switcher->conv_element);
    switcher->conv_element = NULL;
  }

  if (switcher->pipeline) {
    gst_element_set_state (switcher->pipeline, GST_STATE_NULL);
    gst_object_unref (switcher->pipeline);
    switcher->pipeline = NULL;
  }

  g_free (switcher);
}

static GstElement *
gst_switcher_test_pipeline (GstSwitcher * switcher)
{
  GString *desc;
  GstElement *pipeline;
  GError *error = NULL;

  desc = g_string_new ("");

#if 0
  g_string_append_printf (desc, "tcpmixsrc name=source mode=loop "
      "fill=none port=%d ", opts.port);
  g_string_append_printf (desc, "switch name=switch ");
  g_string_append_printf (desc, "filesink name=sink "
      "location=%s ", opts.test_switch);
  g_string_append_printf (desc, "convbin name=conv "
      "converter=identity ");
  g_string_append_printf (desc, "funnel name=sum ");
  g_string_append_printf (desc, "source. ! conv. ");
  g_string_append_printf (desc, "conv. ! switch. ");
  g_string_append_printf (desc, "switch. ! sum. ");
  g_string_append_printf (desc, "sum. ! sink.");
#else
  g_string_append_printf (desc, "multiqueue name=switch ");
  g_string_append_printf (desc, "funnel name=compose ");
  g_string_append_printf (desc, "funnel name=preview ");
  g_string_append_printf (desc, "tcpserversink name=sink1 port=3001 ");
  g_string_append_printf (desc, "tcpserversink name=sink2 port=3002 ");
  g_string_append_printf (desc, "switch.src_0 ! compose. ");
  g_string_append_printf (desc, "switch.src_1 ! compose. ");
  g_string_append_printf (desc, "switch.src_2 ! preview. ");
  g_string_append_printf (desc, "switch.src_3 ! preview. ");
  g_string_append_printf (desc, "switch.src_4 ! preview. ");
  g_string_append_printf (desc, "compose. ! sink1. ");
  g_string_append_printf (desc, "preview. ! sink2. ");
#endif

  if (opts.verbose)
    g_print ("pipeline: %s\n", desc->str);

  pipeline = (GstElement *) gst_parse_launch (desc->str, &error);
  g_string_free (desc, FALSE);

  if (error) {
    g_print ("pipeline parsing error: %s\n", error->message);
    gst_object_unref (pipeline);
    return NULL;
  }

  return pipeline;
}

static GstElement *
gst_switcher_create_pipeline (GstSwitcher * switcher)
{
  GString *desc;
  GstElement *pipeline;
  GError *error = NULL;

  INFO ("Listenning on port %d", opts.port);

  desc = g_string_new ("");

  g_string_append_printf (desc, "tcpmixsrc name=source mode=loop "
      "fill=none autosink=convert port=%d ", opts.port);
  g_string_append_printf (desc, "source. ! convert. ");

  g_string_append_printf (desc, "convbin name=convert "
      "converter=gdpdepay autosink=switch ");

  /*
  g_string_append_printf (desc, "switch name=switch ");
  g_string_append_printf (desc, "convbin name=conv "
      "converter=gdpdepay autosink=switch ");
  g_string_append_printf (desc, "source. ! conv. ");
  g_string_append_printf (desc, "switch. ! sink1. ");
  g_string_append_printf (desc, "switch. ! sink2. ");
  */
  /*
  g_string_append_printf (desc, "source. ! switch. ");
  g_string_append_printf (desc, "switch. ! sink1. ");
  g_string_append_printf (desc, "switch. ! sink2. ");
  */
  /*
  g_string_append_printf (desc, "funnel name=switch ");
  g_string_append_printf (desc, "funnel name=compose ");
  g_string_append_printf (desc, "funnel name=preview ");
  g_string_append_printf (desc, "tee name=branch ");
  g_string_append_printf (desc, "tcpserversink name=sink1 port=3001 ");
  g_string_append_printf (desc, "tcpserversink name=sink2 port=3002 ");
  g_string_append_printf (desc, "switch. ! branch. ");
  g_string_append_printf (desc, "branch.src_0 ! compose. ");
  g_string_append_printf (desc, "branch.src_1 ! compose. ");
  g_string_append_printf (desc, "branch.src_2 ! preview. ");
  g_string_append_printf (desc, "branch.src_3 ! preview. ");
  g_string_append_printf (desc, "compose. ! queue ! sink1. ");
  g_string_append_printf (desc, "preview. ! queue ! sink2. ");
  */
  g_string_append_printf (desc, "multiqueue name=switch ");
  //g_string_append_printf (desc, "switch name=switch ");
  g_string_append_printf (desc, "switch. ! compose_a. ");
  //g_string_append_printf (desc, "switch. ! compose_b. ");
  /*
  g_string_append_printf (desc, "switch.src_0 ! compose_a. ");
  g_string_append_printf (desc, "switch.src_1 ! compose_b. ");
  g_string_append_printf (desc, "switch.src_2 ! preview. ");
  g_string_append_printf (desc, "switch.src_3 ! preview. ");
  g_string_append_printf (desc, "switch.src_4 ! preview. ");
  g_string_append_printf (desc, "switch.src_5 ! preview. ");
  */

  // input-selector
  //g_string_append_printf (desc, "identity name=compose_a ");
  //g_string_append_printf (desc, "identity name=compose_b ");
  //g_string_append_printf (desc, "compose_a. ! sink1. ");
  //g_string_append_printf (desc, "compose_b. ! sink3. ");

  g_string_append_printf (desc, "identity name=compose_a ");
  g_string_append_printf (desc, "identity name=compose_b ");
  g_string_append_printf (desc, "compose_a. ! queue ! gdppay ! tcpserversink port=3001 ");
  //g_string_append_printf (desc, "compose_b. ! queue ! gdppay ! tcpserversink port=3002 ");
  
  //g_string_append_printf (desc, "videotestsrc pattern=snow "
  //    "! gdppay ! compose_a. ");

  /*
  g_string_append_printf (desc, "funnel name=preview ");
  g_string_append_printf (desc, "preview. ! preview_sink. ");

  g_string_append_printf (desc, "identity name=preview_sink ");
  g_string_append_printf (desc, "preview_sink. ! sink2. ");
  */

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

static void
gst_switcher_start (GstSwitcher * switcher)
{
  gst_element_set_state (switcher->pipeline, GST_STATE_READY);

  switcher->timer_id = g_timeout_add (1000, onesecond_timer, switcher);
}

static void
gst_switcher_stop (GstSwitcher * switcher)
{
  gst_element_set_state (switcher->pipeline, GST_STATE_NULL);

  g_source_remove (switcher->timer_id);
}

static void
gst_switcher_handle_eos (GstSwitcher * switcher)
{
  gst_switcher_stop (switcher);
}

static void
gst_switcher_handle_error (GstSwitcher * switcher, GError * error,
    const char *debug)
{
  ERROR ("%s", error->message);
  gst_switcher_stop (switcher);
}

static void
gst_switcher_handle_warning (GstSwitcher * switcher, GError * error,
    const char *debug)
{
  WARN ("%s", error->message);
}

static void
gst_switcher_handle_info (GstSwitcher * switcher, GError * error,
    const char *debug)
{
  INFO ("%s", error->message);
}

static void
gst_switcher_handle_null_to_ready (GstSwitcher * switcher,
    GstElement * pipeline)
{
  gst_element_set_state (pipeline, GST_STATE_PAUSED);
}

static void
gst_switcher_handle_ready_to_paused (GstSwitcher * switcher,
    GstElement * pipeline)
{
  if (!switcher->paused_for_buffering) {
    gst_element_set_state (pipeline, GST_STATE_PLAYING);
  }
}

static void
gst_switcher_handle_paused_to_playing (GstSwitcher * switcher,
    GstElement * pipeline)
{
}

static void
gst_switcher_handle_playing_to_paused (GstSwitcher * switcher,
    GstElement * pipeline)
{
}

static void
gst_switcher_handle_paused_to_ready (GstSwitcher * switcher,
    GstElement * pipeline)
{
}

static void
gst_switcher_handle_ready_to_null (GstSwitcher * switcher,
    GstElement * pipeline)
{
  //g_main_loop_quit (switcher->main_loop);
}

static gboolean
gst_switcher_handle_pipeline_state_changed (GstSwitcher * switcher,
    GstElement * pipeline, GstStateChange statechange)
{
  switch (statechange) {
  case GST_STATE_CHANGE_NULL_TO_READY:
    gst_switcher_handle_null_to_ready (switcher, pipeline);
    break;
  case GST_STATE_CHANGE_READY_TO_PAUSED:
    gst_switcher_handle_ready_to_paused (switcher, pipeline);
    break;
  case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
    gst_switcher_handle_paused_to_playing (switcher, pipeline);
    break;
  case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
    gst_switcher_handle_playing_to_paused (switcher, pipeline);
    break;
  case GST_STATE_CHANGE_PAUSED_TO_READY:
    gst_switcher_handle_paused_to_ready (switcher, pipeline);
    break;
  case GST_STATE_CHANGE_READY_TO_NULL:
    gst_switcher_handle_ready_to_null (switcher, pipeline);
    break;
  default:
    return FALSE;
  }
  return TRUE;
}

static gboolean
gst_switcher_handle_message (GstBus * bus, GstMessage * message, gpointer data)
{
  GstSwitcher *switcher = (GstSwitcher *) data;

  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_EOS:
      gst_switcher_handle_eos (switcher);
      break;
    case GST_MESSAGE_ERROR:
    {
      GError *error = NULL;
      gchar *debug;

      gst_message_parse_error (message, &error, &debug);
      gst_switcher_handle_error (switcher, error, debug);
    } break;
    case GST_MESSAGE_WARNING:
    {
      GError *error = NULL;
      gchar *debug;

      gst_message_parse_warning (message, &error, &debug);
      gst_switcher_handle_warning (switcher, error, debug);
    } break;
    case GST_MESSAGE_INFO:
    {
      GError *error = NULL;
      gchar *debug;

      gst_message_parse_info (message, &error, &debug);
      gst_switcher_handle_info (switcher, error, debug);
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
      if (GST_ELEMENT (message->src) == switcher->pipeline) {
        if (opts.verbose)
          g_print ("switch-server: state change from %s to %s\n",
              gst_element_state_get_name (oldstate),
              gst_element_state_get_name (newstate));

	ret = gst_switcher_handle_pipeline_state_changed (switcher,
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
      if (!switcher->paused_for_buffering && percent < 100) {
        g_print ("pausing for buffing\n");
        switcher->paused_for_buffering = TRUE;
        gst_element_set_state (switcher->pipeline, GST_STATE_PAUSED);
      } else if (switcher->paused_for_buffering && percent == 100) {
        g_print ("unpausing for buffing\n");
        switcher->paused_for_buffering = FALSE;
        gst_element_set_state (switcher->pipeline, GST_STATE_PLAYING);
      }
    } break;
    case GST_MESSAGE_STATE_DIRTY:
    case GST_MESSAGE_CLOCK_PROVIDE:
    case GST_MESSAGE_CLOCK_LOST:
    case GST_MESSAGE_NEW_CLOCK:
    case GST_MESSAGE_STRUCTURE_CHANGE:
    case GST_MESSAGE_STREAM_STATUS:
      break;
    case GST_MESSAGE_STEP_DONE:
    case GST_MESSAGE_APPLICATION:
    case GST_MESSAGE_ELEMENT:
    case GST_MESSAGE_SEGMENT_START:
    case GST_MESSAGE_SEGMENT_DONE:
      //case GST_MESSAGE_DURATION:
    case GST_MESSAGE_LATENCY:
    case GST_MESSAGE_ASYNC_START:
    case GST_MESSAGE_ASYNC_DONE:
    case GST_MESSAGE_REQUEST_STATE:
    case GST_MESSAGE_STEP_START:
    case GST_MESSAGE_QOS:
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
  //GstSwitcher *switcher = (GstSwitcher *)priv;

  //g_print (".\n");

  return TRUE;
}

static void
on_source_pad_added (GstElement * element, GstPad * pad,
    GstSwitcher * switcher)
{
  INFO ("source-pad-added: %s.%s", GST_ELEMENT_NAME (element),
      GST_PAD_NAME (pad));
}

static void
on_switch_pad_added (GstElement * element, GstPad * pad,
    GstSwitcher * switcher)
{
  INFO ("source-pad-added: %s.%s", GST_ELEMENT_NAME (element),
      GST_PAD_NAME (pad));
}

static void
on_convert_pad_added (GstElement * element, GstPad * pad,
    GstSwitcher * switcher)
{
  INFO ("convert-pad-added: %s.%s", GST_ELEMENT_NAME (element),
      GST_PAD_NAME (pad));
}

static void
on_source_new_client (GstElement * element, GstPad * pad,
    GstSwitcher * switcher)
{
  INFO ("new client on %s.%s", GST_ELEMENT_NAME (element),
      GST_PAD_NAME (pad));
}

static void
on_sink1_pad_added (GstElement * element, GstPad * pad,
    GstSwitcher * switcher)
{
  INFO ("new pad %s.%s", GST_ELEMENT_NAME (element),
      GST_PAD_NAME (pad));
}

static void
on_sink2_pad_added (GstElement * element, GstPad * pad,
    GstSwitcher * switcher)
{
  INFO ("new pad %s.%s", GST_ELEMENT_NAME (element),
      GST_PAD_NAME (pad));
}

static void
gst_switcher_set_pipeline (GstSwitcher * switcher, GstElement *pipeline)
{
  switcher->pipeline = pipeline;

  gst_pipeline_set_auto_flush_bus (GST_PIPELINE (pipeline), FALSE);
  switcher->bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (switcher->bus, gst_switcher_handle_message, switcher);

  switcher->source_element = gst_bin_get_by_name (GST_BIN (pipeline), "source");
  switcher->sink_element = gst_bin_get_by_name (GST_BIN (pipeline), "sink");
  switcher->switch_element = gst_bin_get_by_name (GST_BIN (pipeline), "switch");
  switcher->conv_element = gst_bin_get_by_name (GST_BIN (pipeline), "conv");
  switcher->sink1_element = gst_bin_get_by_name (GST_BIN (pipeline), "sink1");
  switcher->sink2_element = gst_bin_get_by_name (GST_BIN (pipeline), "sink2");

  if (switcher->source_element) {
    g_signal_connect (switcher->source_element, "pad-added",
	G_CALLBACK (on_source_pad_added), switcher);

    g_signal_connect (switcher->source_element, "new-client",
	G_CALLBACK (on_source_new_client), switcher);
  }

  if (switcher->conv_element) {
    g_signal_connect (switcher->conv_element, "pad-added",
	G_CALLBACK (on_convert_pad_added), switcher);
  }

  if (switcher->switch_element) {
    GstElement * swit = switcher->switch_element;
    GList * item = GST_ELEMENT_PADS (swit);
    GstPad * pad;
    for (; item; item = g_list_next (item)) {
      pad = GST_PAD (item->data);
      INFO ("switch-pad: %s", GST_PAD_NAME (pad));
    }

    g_signal_connect (switcher->switch_element, "pad-added",
	G_CALLBACK (on_switch_pad_added), switcher);
  }

  if (switcher->sink1_element) {
    g_signal_connect (switcher->sink1_element, "pad-added",
	G_CALLBACK (on_sink1_pad_added), switcher);
  }

  if (switcher->sink2_element) {
    g_signal_connect (switcher->sink2_element, "pad-added",
	G_CALLBACK (on_sink2_pad_added), switcher);
  }
}

gpointer gst_switcher_run (GstSwitcher *switcher)
{
  GstElement *pipeline;

  if (opts.test_switch)
    pipeline = gst_switcher_test_pipeline (switcher);
  else
    pipeline = gst_switcher_create_pipeline (switcher);

  gst_switcher_set_pipeline (switcher, pipeline);
  gst_switcher_start (switcher);

  switcher->main_loop = g_main_loop_new (NULL, TRUE);

  g_main_loop_run (switcher->main_loop);
  return NULL;
}

#if 0
gboolean
have_element (const gchar * element_name)
{
  GstPluginFeature *feature;

  feature = gst_default_registry_find_feature (element_name,
      GST_TYPE_ELEMENT_FACTORY);
  if (feature) {
    g_object_unref (feature);
    return TRUE;
  }
  return FALSE;
}
#endif
