/* GstSwitchSrv
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

#include <gst/gst.h>
#include <stdlib.h>
#include "../logutils.h"

#define GETTEXT_PACKAGE "switchsrv"

typedef struct _GstSwitchSrv GstSwitchSrv;
struct _GstSwitchSrv
{
  GstElement *pipeline;
  GstBus *bus;
  GMainLoop *main_loop;

  GstElement *source_element;
  GstElement *sink_element;
  GstElement *switch_element;
  GstElement *conv_element;

  gboolean paused_for_buffering;
  guint timer_id;
};

static /*const*/ struct {
  gboolean verbose;
  gchar * test_switch;
  gint port;
} opts = {
  0, NULL, 3000,
};

static GOptionEntry entries[] = {
  {"verbose", 'v', 0,		G_OPTION_ARG_NONE, &opts.verbose,
       "Prompt more messages", NULL},
  {"test-switch", 't', 0,	G_OPTION_ARG_STRING, &opts.test_switch,
       "Perform switch test", "OUTPUT"},
  {"port", 'p', 0,		G_OPTION_ARG_INT, &opts.port,
       "Specify the listen port.", "NUM"},
  { NULL }
};

static void
parse_args (int *argc, char **argv[])
{
  GError *error = NULL;
  GOptionContext *context;

  context = g_option_context_new ("");
  g_option_context_add_main_entries (context, entries, GETTEXT_PACKAGE);
  g_option_context_add_group (context, gst_init_get_option_group ());
  if (!g_option_context_parse (context, argc, argv, &error)) {
    g_print ("option parsing failed: %s\n", error->message);
    exit (1);
  }

  g_option_context_free (context);
}

static GstSwitchSrv *
gst_switchsrv_new (void)
{
  GstSwitchSrv *switchsrv;

  switchsrv = g_new0 (GstSwitchSrv, 1);

  return switchsrv;
}

static void
gst_switchsrv_free (GstSwitchSrv * switchsrv)
{
  if (switchsrv->source_element) {
    gst_object_unref (switchsrv->source_element);
    switchsrv->source_element = NULL;
  }
  if (switchsrv->sink_element) {
    gst_object_unref (switchsrv->sink_element);
    switchsrv->sink_element = NULL;
  }
  if (switchsrv->switch_element) {
    gst_object_unref (switchsrv->switch_element);
    switchsrv->switch_element = NULL;
  }
  if (switchsrv->conv_element) {
    gst_object_unref (switchsrv->conv_element);
    switchsrv->conv_element = NULL;
  }

  if (switchsrv->pipeline) {
    gst_element_set_state (switchsrv->pipeline, GST_STATE_NULL);
    gst_object_unref (switchsrv->pipeline);
    switchsrv->pipeline = NULL;
  }
  g_free (switchsrv);
}

static GstElement *
gst_switchsrv_test_switch_pipeline (GstSwitchSrv * switchsrv)
{
  GString *pipe_desc;
  GstElement *pipeline;
  GError *error = NULL;

  pipe_desc = g_string_new ("");

  g_string_append_printf (pipe_desc, "tcpmixsrc name=source mode=loop "
      "fill=none port=%d ", opts.port);
  g_string_append_printf (pipe_desc, "switch name=switch ");
  g_string_append_printf (pipe_desc, "filesink name=sink "
      "location=%s ", opts.test_switch);
  g_string_append_printf (pipe_desc, "convbin name=conv "
      "converter=identity ");
  g_string_append_printf (pipe_desc, "funnel name=sum ");
  g_string_append_printf (pipe_desc, "source. ! conv. ");
  g_string_append_printf (pipe_desc, "conv. ! switch. ");
  g_string_append_printf (pipe_desc, "switch. ! sum. ");
  g_string_append_printf (pipe_desc, "sum. ! sink.");

  if (opts.verbose)
    g_print ("pipeline: %s\n", pipe_desc->str);

  pipeline = (GstElement *) gst_parse_launch (pipe_desc->str, &error);
  g_string_free (pipe_desc, FALSE);

  if (error) {
    g_print ("pipeline parsing error: %s\n", error->message);
    gst_object_unref (pipeline);
    return NULL;
  }

  return pipeline;
}

static GstElement *
gst_switchsrv_create_pipeline (GstSwitchSrv * switchsrv)
{
  GString *desc;
  GstElement *pipeline;
  GError *error = NULL;

  INFO ("Listenning on port %d", opts.port);

  desc = g_string_new ("");

  g_string_append_printf (desc, "tcpmixsrc name=source mode=loop "
      "fill=none port=%d ", opts.port);
  g_string_append_printf (desc, "switch name=switch ");
  g_string_append_printf (desc, "convbin name=conv "
      "converter=gdpdepay autosink=switch ");
  g_string_append_printf (desc, "xvimagesink name=sink1 ");
  //g_string_append_printf (desc, "xvimagesink name=sink2 ");
  g_string_append_printf (desc, "source. ! conv. ");
#if 0
  g_string_append_printf (desc, "conv. ! sink1. ");
  g_string_append_printf (desc, "conv. ! sink2. ");
#else
  g_string_append_printf (desc, "switch. ! sink1. ");
  g_string_append_printf (desc, "switch. ! sink2. ");
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

static void
gst_switchsrv_start (GstSwitchSrv * switchsrv)
{
  gst_element_set_state (switchsrv->pipeline, GST_STATE_READY);

  switchsrv->timer_id = g_timeout_add (1000, onesecond_timer, switchsrv);
}

static void
gst_switchsrv_stop (GstSwitchSrv * switchsrv)
{
  gst_element_set_state (switchsrv->pipeline, GST_STATE_NULL);

  g_source_remove (switchsrv->timer_id);
}

static void
gst_switchsrv_handle_eos (GstSwitchSrv * switchsrv)
{
  gst_switchsrv_stop (switchsrv);
}

static void
gst_switchsrv_handle_error (GstSwitchSrv * switchsrv, GError * error,
    const char *debug)
{
  ERROR ("%s", error->message);
  gst_switchsrv_stop (switchsrv);
}

static void
gst_switchsrv_handle_warning (GstSwitchSrv * switchsrv, GError * error,
    const char *debug)
{
  WARN ("%s", error->message);
}

static void
gst_switchsrv_handle_info (GstSwitchSrv * switchsrv, GError * error,
    const char *debug)
{
  INFO ("%s", error->message);
}

static void
gst_switchsrv_handle_null_to_ready (GstSwitchSrv * switchsrv)
{
  gst_element_set_state (switchsrv->pipeline, GST_STATE_PAUSED);
}

static void
gst_switchsrv_handle_ready_to_paused (GstSwitchSrv * switchsrv)
{
  if (!switchsrv->paused_for_buffering) {
    gst_element_set_state (switchsrv->pipeline, GST_STATE_PLAYING);
  }
}

static void
gst_switchsrv_handle_paused_to_playing (GstSwitchSrv * switchsrv)
{

}

static void
gst_switchsrv_handle_playing_to_paused (GstSwitchSrv * switchsrv)
{

}

static void
gst_switchsrv_handle_paused_to_ready (GstSwitchSrv * switchsrv)
{

}

static void
gst_switchsrv_handle_ready_to_null (GstSwitchSrv * switchsrv)
{
  g_main_loop_quit (switchsrv->main_loop);

}

static gboolean
gst_switchsrv_handle_message (GstBus * bus, GstMessage * message, gpointer data)
{
  GstSwitchSrv *switchsrv = (GstSwitchSrv *) data;

  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_EOS:
      gst_switchsrv_handle_eos (switchsrv);
      break;
    case GST_MESSAGE_ERROR:
    {
      GError *error = NULL;
      gchar *debug;

      gst_message_parse_error (message, &error, &debug);
      gst_switchsrv_handle_error (switchsrv, error, debug);
    } break;
    case GST_MESSAGE_WARNING:
    {
      GError *error = NULL;
      gchar *debug;

      gst_message_parse_warning (message, &error, &debug);
      gst_switchsrv_handle_warning (switchsrv, error, debug);
    } break;
    case GST_MESSAGE_INFO:
    {
      GError *error = NULL;
      gchar *debug;

      gst_message_parse_info (message, &error, &debug);
      gst_switchsrv_handle_info (switchsrv, error, debug);
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
      GstState oldstate, newstate, pending;

      gst_message_parse_state_changed (message, &oldstate, &newstate, &pending);
      if (GST_ELEMENT (message->src) == switchsrv->pipeline) {
        if (opts.verbose)
          g_print ("switch-server: state change from %s to %s\n",
              gst_element_state_get_name (oldstate),
              gst_element_state_get_name (newstate));
        switch (GST_STATE_TRANSITION (oldstate, newstate)) {
          case GST_STATE_CHANGE_NULL_TO_READY:
            gst_switchsrv_handle_null_to_ready (switchsrv);
            break;
          case GST_STATE_CHANGE_READY_TO_PAUSED:
            gst_switchsrv_handle_ready_to_paused (switchsrv);
            break;
          case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
            gst_switchsrv_handle_paused_to_playing (switchsrv);
            break;
          case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
            gst_switchsrv_handle_playing_to_paused (switchsrv);
            break;
          case GST_STATE_CHANGE_PAUSED_TO_READY:
            gst_switchsrv_handle_paused_to_ready (switchsrv);
            break;
          case GST_STATE_CHANGE_READY_TO_NULL:
            gst_switchsrv_handle_ready_to_null (switchsrv);
            break;
          default:
            if (opts.verbose)
              g_print ("unknown state change from %s to %s\n",
                  gst_element_state_get_name (oldstate),
                  gst_element_state_get_name (newstate));
        }
      }
    } break;
    case GST_MESSAGE_BUFFERING:
    {
      int percent;
      gst_message_parse_buffering (message, &percent);
      //g_print("buffering %d\n", percent);
      if (!switchsrv->paused_for_buffering && percent < 100) {
        g_print ("pausing for buffing\n");
        switchsrv->paused_for_buffering = TRUE;
        gst_element_set_state (switchsrv->pipeline, GST_STATE_PAUSED);
      } else if (switchsrv->paused_for_buffering && percent == 100) {
        g_print ("unpausing for buffing\n");
        switchsrv->paused_for_buffering = FALSE;
        gst_element_set_state (switchsrv->pipeline, GST_STATE_PLAYING);
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
  //GstSwitchSrv *switchsrv = (GstSwitchSrv *)priv;

  //g_print (".\n");

  return TRUE;
}

static void
on_source_pad_added (GstElement * element, GstPad * pad,
    GstSwitchSrv * switchsrv)
{
  INFO ("source-pad-added: %s.%s", GST_ELEMENT_NAME (element),
      GST_PAD_NAME (pad));
}

static void
on_convert_pad_added (GstElement * element, GstPad * pad,
    GstSwitchSrv * switchsrv)
{
  INFO ("convert-pad-added: %s.%s", GST_ELEMENT_NAME (element),
      GST_PAD_NAME (pad));
}

static void
on_source_new_client (GstElement * element, GstPad * pad,
    GstSwitchSrv * switchsrv)
{
  INFO ("new client on %s.%s", GST_ELEMENT_NAME (element),
      GST_PAD_NAME (pad));
}

static void
gst_switchsrv_set_pipeline (GstSwitchSrv * switchsrv, GstElement *pipeline)
{
  switchsrv->pipeline = pipeline;

  gst_pipeline_set_auto_flush_bus (GST_PIPELINE (pipeline), FALSE);
  switchsrv->bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (switchsrv->bus, gst_switchsrv_handle_message, switchsrv);

  switchsrv->source_element = gst_bin_get_by_name (GST_BIN (pipeline), "source");
  switchsrv->sink_element = gst_bin_get_by_name (GST_BIN (pipeline), "sink");
  switchsrv->switch_element = gst_bin_get_by_name (GST_BIN (pipeline), "switch");
  switchsrv->conv_element = gst_bin_get_by_name (GST_BIN (pipeline), "conv");

  if (switchsrv->source_element) {
    g_signal_connect (switchsrv->source_element, "pad-added",
	G_CALLBACK (on_source_pad_added), switchsrv);

    g_signal_connect (switchsrv->source_element, "new-client",
	G_CALLBACK (on_source_new_client), switchsrv);
  }

  if (switchsrv->conv_element) {
    g_signal_connect (switchsrv->conv_element, "pad-added",
	G_CALLBACK (on_convert_pad_added), switchsrv);
  }

  {
    GstElement * swit = switchsrv->switch_element;
    GList * item = GST_ELEMENT_PADS (swit);
    GstPad * pad;
    for (; item; item = g_list_next (item)) {
      pad = GST_PAD (item->data);
      INFO ("switch-pad: %s", GST_PAD_NAME (pad));
    }
  }
}

/* helper functions */

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

int
main (int argc, char *argv[])
{
  GstSwitchSrv *switchsrv;
  GstElement *pipeline;

  parse_args (&argc, &argv);

  switchsrv = gst_switchsrv_new ();

  if (opts.test_switch)
    pipeline = gst_switchsrv_test_switch_pipeline (switchsrv);
  else
    pipeline = gst_switchsrv_create_pipeline (switchsrv);

  gst_switchsrv_set_pipeline (switchsrv, pipeline);
  gst_switchsrv_start (switchsrv);

  switchsrv->main_loop = g_main_loop_new (NULL, TRUE);

  g_main_loop_run (switchsrv->main_loop);

  gst_switchsrv_free (switchsrv);
  return 0;
}
