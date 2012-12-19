/* test-mix-source
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

static gint test_port = 3030;

typedef struct _Stuff Stuff;
struct _Stuff
{
  GMainLoop *mainloop;
  GstElement *bin;
};

static gboolean
handle_bus_message (GstBus * bus, GstMessage * message, gpointer data)
{
  Stuff *stuff = (Stuff *) data;
  switch (GST_MESSAGE_TYPE (message)) {
  case GST_MESSAGE_STATE_CHANGED:
  {
    GstState oldstate, newstate, pending;
    gst_message_parse_state_changed (message, &oldstate, &newstate, &pending);
    if (GST_ELEMENT (message->src) == stuff->bin) {
      switch (GST_STATE_TRANSITION (oldstate, newstate)) {
      case GST_STATE_CHANGE_NULL_TO_READY:
	g_main_loop_quit (stuff->mainloop);
	break;
      default:
	break;
      }
    }
  } break;
  default:
    break;
  }

  return TRUE;
}

static gpointer
test_burn_thread (gpointer data)
{
  GString *desc;
  GstElement *bin;
  GMainLoop *mainloop;
  GstBus *bus;
  GError *error = NULL;

  desc = g_string_new ("");

  //g_string_append (desc, "videotestsrc ! xvimagesink");
  g_string_append_printf (desc, "tcpmixsrc port=%d ! xvimagesink", test_port);

  bin = (GstElement *) gst_parse_launch (desc->str, &error);
  g_string_free (desc, FALSE);

  if (error) {
    g_test_fail ();
    return NULL;
  }

  mainloop = g_main_loop_new (NULL, TRUE);

  gst_pipeline_set_auto_flush_bus (GST_PIPELINE (bin), FALSE);
  bus = gst_pipeline_get_bus (GST_PIPELINE (bin));

  Stuff stuff = { mainloop, bin };
  gst_bus_add_watch (bus, handle_bus_message, &stuff);

  //source = gst_bin_get_by_name (GST_BIN (bin), "source");
  //sink = gst_bin_get_by_name (GST_BIN (bin), "sink");

  gst_element_set_state (bin, GST_STATE_READY);
  g_main_loop_run (mainloop);
  return NULL;
}

static gpointer
test_feed_thread (gpointer data)
{
  // TODO: test code
  return NULL;
}

int main(int argc, char**argv)
{
  gst_init (&argc, &argv);

  GThread *t1 = g_thread_new ("burn", test_burn_thread, NULL);
  GThread *t2 = g_thread_new ("feed", test_feed_thread, NULL);
  g_thread_join (t1);
  g_thread_join (t2);
  g_thread_unref (t1);
  g_thread_unref (t2);
  return 0;
}
