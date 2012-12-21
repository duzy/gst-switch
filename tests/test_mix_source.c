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
#include <unistd.h>

static gint test_port = 3030;

typedef struct _Stuff Stuff;
struct _Stuff
{
  const gchar *name;
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
	break;
      case GST_STATE_CHANGE_READY_TO_NULL:
	g_print ("%s: quit\n", stuff->name);
	g_main_loop_quit (stuff->mainloop);
	break;
      default:
	goto dump;
      }
    } else {
      goto dump;      
    }
  } break;
  case GST_MESSAGE_ERROR:
  {
    GError *err = NULL;
    gchar *dbg_info = NULL;

    gst_message_parse_error (message, &err, &dbg_info);
    g_printerr ("%s: %s: error: %s\n",
	stuff->name, GST_OBJECT_NAME (message->src), err->message);
    g_printerr ("%s: %s\n",
	stuff->name, (dbg_info) ? dbg_info : "(unknown)");
    g_error_free (err);
    g_free (dbg_info);
  } break;
  default:
    goto dump;
  }

  return TRUE;

 dump:
  g_print ("%s: %s:\t%s\n", stuff->name,
      GST_OBJECT_NAME (message->src), GST_MESSAGE_TYPE_NAME (message));
  return TRUE;
}

static void
test_run_pipeline (const gchar *name, GString *desc)
{
  Stuff stuff;
  GstBus *bus;
  GError *error = NULL;

  stuff.name = name;
  stuff.bin = (GstElement *) gst_parse_launch (desc->str, &error);

  if (error) {
    g_print ("error: %s\n", error->message);
    g_test_fail ();
    return;
  }

  stuff.mainloop = g_main_loop_new (NULL, TRUE);

  gst_pipeline_set_auto_flush_bus (GST_PIPELINE (stuff.bin), FALSE);
  bus = gst_pipeline_get_bus (GST_PIPELINE (stuff.bin));

  gst_bus_add_watch (bus, handle_bus_message, &stuff);

  //source = gst_bin_get_by_name (GST_BIN (stuff.bin), "source");
  //sink = gst_bin_get_by_name (GST_BIN (stuff.bin), "sink");

  gst_element_set_state (stuff.bin, GST_STATE_READY);
  g_main_loop_run (stuff.mainloop);
}

static gpointer
test_burn_thread (gpointer data)
{
  GString *desc = g_string_new ("");

#if 1
  //g_string_append (desc, "videotestsrc ! videoconvert ! videoscale ! xvimagesink");
  g_string_append_printf (desc, "tcpmixsrc port=%d ! gdpdepay ! xvimagesink",
      test_port);
#else
  g_string_append_printf (desc, "tcpmixsrc port=%d ! funnel ! fdsink fd=2",
      test_port);
#endif

  g_print ("burning..\n");

  test_run_pipeline ("burn", desc);
  g_string_free (desc, FALSE);
  return NULL;
}

static gpointer
test_feed_thread (gpointer data)
{
  GString *desc = g_string_new ("");

#if 1
  g_string_append_printf (desc, "videotestsrc ! gdppay ! tcpclientsink port=%d",
      test_port);
#else
  g_string_append_printf (desc, "filesrc location=/dev/zero ! tcpclientsink port=%d",
      test_port);
#endif

  g_print ("feeding..\n");

  test_run_pipeline ("feed", desc);
  g_string_free (desc, FALSE);
  return NULL;
}

int main(int argc, char**argv)
{
#if 0
  GThread *t1, *t2;

  gst_init (&argc, &argv);

  t1 = g_thread_new ("burn", test_burn_thread, NULL); usleep (500000);
  t2 = g_thread_new ("feed", test_feed_thread, NULL);
  g_thread_join (t1);
  g_thread_join (t2);
  g_thread_unref (t1);
  g_thread_unref (t2);

#else

  if (fork ()) {
    gst_init (&argc, &argv);
    test_burn_thread (NULL);
  } else {
    usleep (500000);
    gst_init (&argc, &argv);
    test_feed_thread (NULL);
  }

#endif
  return 0;
}
