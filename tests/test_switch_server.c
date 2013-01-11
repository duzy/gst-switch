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

#include <gst/gst.h>
#include <gio/gio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include "../logutils.h"

typedef struct _TestCase TestCase;
struct _TestCase
{
  const gchar *name;
  GMainLoop *mainloop;
  GstElement *pipeline;
  GString *desc;
  gint timer;
  gint live_seconds;
  gint error_count;
};

static void
testcase_quit (TestCase *t)
{
  gst_element_set_state (t->pipeline, GST_STATE_NULL);
  g_main_loop_quit (t->mainloop);
}

static void
testcase_fail (TestCase *t)
{
  testcase_quit (t);
  g_test_fail ();
}

static void
testcase_ok (TestCase *t)
{
  testcase_quit (t);
}

static void
testcase_state_change (TestCase *t, GstState oldstate, GstState newstate, GstState pending)
{
  GstStateChange statechange = GST_STATE_TRANSITION (oldstate, newstate);
  switch (statechange) {
  case GST_STATE_CHANGE_NULL_TO_READY:
    gst_element_set_state (t->pipeline, GST_STATE_PAUSED);
    break;
  case GST_STATE_CHANGE_READY_TO_PAUSED:
    gst_element_set_state (t->pipeline, GST_STATE_PLAYING);
    break;
  case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
    break;
  case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
    break;
  case GST_STATE_CHANGE_PAUSED_TO_READY:
    break;    
  case GST_STATE_CHANGE_READY_TO_NULL:
    INFO ("quit: %s\n", t->name);
    testcase_ok (t);
    break;
  default:
    break;
  }
}

static void
testcase_error_message (TestCase *t, GError *error, const gchar *info)
{
  ERROR ("error (%s): %s", t->name, error->message);
  WARN ("----------: %s", info);

  t->error_count += 1;
}

static gboolean
testcase_pipeline_message (GstBus * bus, GstMessage * message, gpointer data)
{
  TestCase *t = (TestCase *) data;
  switch (GST_MESSAGE_TYPE (message)) {
  case GST_MESSAGE_STATE_CHANGED:
  {
    if (GST_ELEMENT (message->src) == t->pipeline) {
      GstState oldstate, newstate, pending;
      gst_message_parse_state_changed (message, &oldstate, &newstate, &pending);
      testcase_state_change (t, oldstate, newstate, pending);
    }
  } break;
  case GST_MESSAGE_ERROR:
  {
    GError *error = NULL;
    gchar *info = NULL;
    gst_message_parse_error (message, &error, &info);
    testcase_error_message (t, error, info);
    g_error_free (error);
    g_free (info);
  } break;
  case GST_MESSAGE_EOS:
  {
    gst_element_set_state (t->pipeline, GST_STATE_NULL);
    testcase_ok (t);
  } break;
  default:
  {
    INFO ("%s: %s", GST_OBJECT_NAME (message->src), GST_MESSAGE_TYPE_NAME (message));
  } break;
  }
  return TRUE;
}

static gboolean
testcase_launch_pipeline (TestCase *t)
{
  GError *error = NULL;
  GstBus *bus;

  if (!t->desc) {
    ERROR ("invalid test case");
    testcase_fail (t);
    return FALSE;
  }

  t->pipeline = (GstElement *) gst_parse_launch (t->desc->str, &error);

  if (error) {
    ERROR ("%s: %s", t->name, error->message);
    testcase_fail (t);
    return FALSE;
  }

  gst_pipeline_set_auto_flush_bus (GST_PIPELINE (t->pipeline), FALSE);
  bus = gst_pipeline_get_bus (GST_PIPELINE (t->pipeline));
  gst_bus_add_watch (bus, testcase_pipeline_message, t);
  gst_element_set_state (t->pipeline, GST_STATE_READY);
  return TRUE;
}

static void
testcase_server_activate (GApplication *app)
{
  g_application_hold (app);
  INFO ("activated");
  g_application_release (app);
}

static void
testcase_server_open (GApplication  *application, GFile **files,
    gint n_files, const gchar *hint)
{
  gint i;

  g_application_hold (application);
  g_print ("open");

  for (i = 0; i < n_files; i++) {
    gchar *uri = g_file_get_uri (files[i]);
    g_print (" %s", uri);
    g_free (uri);
  }

  g_application_release (application);

  g_print ("\n");
}

static int
testcase_server_command_line (GApplication *app, GApplicationCommandLine *cmdline)
{
  gint argc, i;
  gchar **argv;
  argv = g_application_command_line_get_arguments (cmdline, &argc);
  /*
  g_application_command_line_print (cmdline, "./tools/gst-switch-srv ");
  g_application_command_line_print (cmdline, "-v");
  g_application_command_line_print (cmdline, " --gst-debug-no-color\n");
  INFO ("cmd: %d, %s", argc, argv[0]);
  */
  for (i = 0; i < argc; ++i) INFO ("argument: %s", argv[i]);
  g_strfreev (argv);
  return 0;
}

#if 0
static gpointer
testcase_run_server (TestCase *t)
{
  GString *cmd = g_string_new ("./tools/gst-switch-srv");
  gint cmdret;
  g_string_append_printf (cmd, " -v");
  g_string_append_printf (cmd, " --gst-debug-no-color");
  cmdret = system (cmd->str);
  g_string_free (cmd, FALSE);
  if (cmdret) {
    ERROR ("error-exit: %d", cmdret);
  }
  return NULL;
}
#endif

static GPid
launch_server ()
{
  GPid pid = 0;
  gchar *argv[] = {
    "./tools/gst-switch-srv", "-v",
    "--gst-debug-no-color",
    "--record=test-recording.data",
    NULL
  };
  GError *error = NULL;
  gboolean ok;
  ok = g_spawn_async (NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD,
      NULL, NULL, &pid, &error);
  if (!ok) {
    ERROR ("spawn: %s", error->message);
    return 0;
  }
  INFO ("server: %d", pid);
  return pid;
}

static GPid
launch_ui ()
{
  GPid pid = 0;
  gchar *argv[] = {
    "./tools/gst-switch-ui", "-v",
    "--gst-debug-no-color",
    NULL
  };
  GError *error = NULL;
  gboolean ok;
  ok = g_spawn_async (NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD,
      NULL, NULL, &pid, &error);
  if (!ok) {
    ERROR ("spawn: %s", error->message);
    return 0;
  }
  INFO ("server: %d", pid);
  return pid;
}

static void
close_server(GPid server_pid)
{
  //kill (server_pid, SIGKILL);
  kill (server_pid, SIGTERM);
  g_spawn_close_pid (server_pid);
}

static gboolean
testcase_second_timer (TestCase *t)
{
  t->live_seconds -= 1;

  if (t->live_seconds == 0) {
    testcase_ok (t);
  }

  return 0 < t->live_seconds ? TRUE : FALSE;
}

static gpointer
testcase_run (TestCase *t)
{
  g_print ("========== %s ==========\n", t->name);
  t->mainloop = g_main_loop_new (NULL, TRUE);
  if (testcase_launch_pipeline (t)) {
    if (0 < t->live_seconds) {
      t->timer = g_timeout_add (1000, (GSourceFunc) testcase_second_timer, t);
    }
    g_main_loop_run (t->mainloop);
    gst_object_unref (t->pipeline);
  } else {
    ERROR ("launch failed");
    g_test_fail ();
  }
  g_main_loop_unref (t->mainloop);
  g_string_free (t->desc, FALSE);
  g_source_remove (t->timer);
  t->mainloop = NULL;
  t->pipeline = NULL;
  t->desc = NULL;
  t->timer = 0;
  return NULL;
}

static GThread *
testcase_run_thread (TestCase *testcase, const gchar *name)
{
  return g_thread_new (name, (GThreadFunc) testcase_run, testcase);
}

static void
test_controller (void)
{
  //TestCase testcase = { __FUNCTION__, 0 };
  //g_thread_join (testcase_run_thread (&testcase, __FUNCTION__));
}

static void
test_video (void)
{
  const gint seconds = 30;
  GPid server_pid;
  GThread *thread_source1;
  GThread *thread_source2;
  GThread *thread_source3;
  GThread *thread_sink0;
  GThread *thread_sink1;
  GThread *thread_sink2;
  GThread *thread_sink3;
  TestCase source1 = { "test_video_source1", 0 };
  TestCase source2 = { "test_video_source2", 0 };
  TestCase source3 = { "test_video_source3", 0 };
  TestCase sink0 = { "test_video_compose_sink", 0 };
  TestCase sink1 = { "test_video_preview_sink1", 0 };
  TestCase sink2 = { "test_video_preview_sink2", 0 };
  TestCase sink3 = { "test_video_preview_sink3", 0 };
  const gchar *textoverlay = "textoverlay "
    "font-desc=\"Sans 80\" "
    "auto-resize=true "
    "shaded-background=true "
    ;

  source1.live_seconds = seconds;
  source1.desc = g_string_new ("videotestsrc pattern=0 ");
  g_string_append_printf (source1.desc, "! video/x-raw,width=1280,height=720 ");
  g_string_append_printf (source1.desc, "! %s text=source1 ", textoverlay);
  g_string_append_printf (source1.desc, "! gdppay ! tcpclientsink port=3000 ");

  source2.live_seconds = seconds;
  source2.desc = g_string_new ("videotestsrc pattern=1 ");
  g_string_append_printf (source2.desc, "! video/x-raw,width=1280,height=720 ");
  g_string_append_printf (source2.desc, "! %s text=source2 ", textoverlay);
  g_string_append_printf (source2.desc, "! gdppay ! tcpclientsink port=3000 ");

  source3.live_seconds = seconds;
  source3.desc = g_string_new ("videotestsrc pattern=15 ");
  g_string_append_printf (source3.desc, "! video/x-raw,width=1280,height=720 ");
  g_string_append_printf (source3.desc, "! %s text=source3 ", textoverlay);
  g_string_append_printf (source3.desc, "! gdppay ! tcpclientsink port=3000 ");

  sink0.live_seconds = seconds;
  sink0.desc = g_string_new ("tcpclientsrc port=3001 ");
  g_string_append_printf (sink0.desc, "! gdpdepay ");
  g_string_append_printf (sink0.desc, "! videoconvert ");
  g_string_append_printf (sink0.desc, "! xvimagesink");

  sink1.live_seconds = seconds;
  sink1.desc = g_string_new ("tcpclientsrc port=3003 ");
  g_string_append_printf (sink1.desc, "! gdpdepay ");
  g_string_append_printf (sink1.desc, "! videoconvert ");
  g_string_append_printf (sink1.desc, "! xvimagesink");

  sink2.live_seconds = seconds;
  sink2.desc = g_string_new ("tcpclientsrc port=3004 ");
  g_string_append_printf (sink2.desc, "! gdpdepay ");
  g_string_append_printf (sink2.desc, "! videoconvert ");
  g_string_append_printf (sink2.desc, "! xvimagesink");

  sink3.live_seconds = seconds;
  sink3.desc = g_string_new ("tcpclientsrc port=3005 ");
  g_string_append_printf (sink3.desc, "! gdpdepay ");
  g_string_append_printf (sink3.desc, "! videoconvert ");
  g_string_append_printf (sink3.desc, "! xvimagesink");

  server_pid = launch_server ();
  g_assert_cmpint (server_pid, !=, 0);
  sleep (2); /* give a second for server to be online */

  thread_source1 = testcase_run_thread (&source1, source1.name);
  sleep (1); /* give a second for source1 to be online */
  thread_source2 = testcase_run_thread (&source2, source2.name);
  thread_source3 = testcase_run_thread (&source3, source3.name);
  sleep (1); /* give a second for sources to be online */
  thread_sink0 = testcase_run_thread (&sink0, sink0.name);
  thread_sink1 = testcase_run_thread (&sink1, sink1.name);
  thread_sink2 = testcase_run_thread (&sink2, sink2.name);
  thread_sink3 = testcase_run_thread (&sink3, sink3.name);

  g_thread_join (thread_source1);
  g_thread_join (thread_source2);
  g_thread_join (thread_source3);
  g_thread_join (thread_sink0);
  g_thread_join (thread_sink1);
  g_thread_join (thread_sink2);
  g_thread_join (thread_sink3);
  g_thread_unref (thread_source1);
  g_thread_unref (thread_source2);
  g_thread_unref (thread_source3);
  g_thread_unref (thread_sink0);
  g_thread_unref (thread_sink1);
  g_thread_unref (thread_sink2);
  g_thread_unref (thread_sink3);

  close_server (server_pid);

  g_assert_cmpstr (source1.name, ==, "test_video_source1");
  g_assert_cmpint (source1.timer, ==, 0);
  g_assert_cmpint (source1.error_count, ==, 0);
  g_assert (source1.desc == NULL);
  g_assert (source1.mainloop == NULL);
  g_assert (source1.pipeline == NULL);

  g_assert_cmpstr (source2.name, ==, "test_video_source2");
  g_assert_cmpint (source2.timer, ==, 0);
  g_assert_cmpint (source2.error_count, ==, 0);
  g_assert (source2.desc == NULL);
  g_assert (source2.mainloop == NULL);
  g_assert (source2.pipeline == NULL);

  g_assert_cmpstr (source3.name, ==, "test_video_source3");
  g_assert_cmpint (source3.timer, ==, 0);
  g_assert_cmpint (source3.error_count, ==, 0);
  g_assert (source3.desc == NULL);
  g_assert (source3.mainloop == NULL);
  g_assert (source3.pipeline == NULL);

  g_assert_cmpstr (sink0.name, ==, "test_video_compose_sink");
  g_assert_cmpint (sink0.timer, ==, 0);
  g_assert_cmpint (sink0.error_count, ==, 0);
  g_assert (sink0.desc == NULL);
  g_assert (sink0.mainloop == NULL);
  g_assert (sink0.pipeline == NULL);

  g_assert_cmpstr (sink1.name, ==, "test_video_preview_sink1");
  g_assert_cmpint (sink1.timer, ==, 0);
  g_assert_cmpint (sink1.error_count, ==, 0);
  g_assert (sink1.desc == NULL);
  g_assert (sink1.mainloop == NULL);
  g_assert (sink1.pipeline == NULL);

  g_assert_cmpstr (sink2.name, ==, "test_video_preview_sink2");
  g_assert_cmpint (sink2.timer, ==, 0);
  g_assert_cmpint (sink2.error_count, ==, 0);
  g_assert (sink2.desc == NULL);
  g_assert (sink2.mainloop == NULL);
  g_assert (sink2.pipeline == NULL);

  g_assert_cmpstr (sink3.name, ==, "test_video_preview_sink3");
  g_assert_cmpint (sink3.timer, ==, 0);
  g_assert_cmpint (sink3.error_count, ==, 0);
  g_assert (sink3.desc == NULL);
  g_assert (sink3.mainloop == NULL);
  g_assert (sink3.pipeline == NULL);

  {
    GFile *file = g_file_new_for_path ("test-recording.data");
    g_assert (g_file_query_exists (file, NULL));
    g_object_unref (file);
  }
}

static void
test_video_recording_result (void)
{
  {
    GFile *file = g_file_new_for_path ("test-recording.data");
    GError *error = NULL;
    g_assert (g_file_query_exists (file, NULL));
    g_assert (g_file_delete (file, NULL, &error));
    g_assert (error == NULL);
    g_assert (!g_file_query_exists (file, NULL));
    g_object_unref (file);
  }
}

static void
test_audio (void)
{
  const gint seconds = 30;
  GThread *thread_source1;
  GThread *thread_source2;
  GThread *thread_source3;
  GThread *thread_sink1;
  GThread *thread_sink2;
  GThread *thread_sink3;
  TestCase source1 = { "test_audio_source1", 0 };
  TestCase source2 = { "test_audio_source2", 0 };
  TestCase source3 = { "test_audio_source3", 0 };
  TestCase sink1 = { "test_audio_preview_sink1", 0 };
  TestCase sink2 = { "test_audio_preview_sink2", 0 };
  TestCase sink3 = { "test_audio_preview_sink3", 0 };
  GPid server_pid;
  const gchar *textoverlay = "textoverlay "
    "font-desc=\"Sans 80\" "
    "auto-resize=true "
    "shaded-background=true "
    ;

  source1.live_seconds = seconds;
  source1.desc = g_string_new ("audiotestsrc ");
  //g_string_append_printf (source1.desc, "! audio/x-raw ");
  g_string_append_printf (source1.desc, "! gdppay ! tcpclientsink port=4000");

  source2.live_seconds = seconds;
  source2.desc = g_string_new ("audiotestsrc ");
  g_string_append_printf (source2.desc, "! gdppay ! tcpclientsink port=4000");

  source3.live_seconds = seconds;
  source3.desc = g_string_new ("audiotestsrc ");
  g_string_append_printf (source3.desc, "! gdppay ! tcpclientsink port=4000");

  sink1.live_seconds = seconds;
  sink1.desc = g_string_new ("tcpclientsrc port=3003 ");
  g_string_append_printf (sink1.desc, "! gdpdepay ! faad ! goom2k1 ");
  g_string_append_printf (sink1.desc, "! %s text=audio1 ", textoverlay);
  g_string_append_printf (sink1.desc, "! videoconvert ");
  g_string_append_printf (sink1.desc, "! xvimagesink");

  sink2.live_seconds = seconds;
  sink2.desc = g_string_new ("tcpclientsrc port=3004 ");
  g_string_append_printf (sink2.desc, "! gdpdepay ! faad ! goom2k1 ");
  g_string_append_printf (sink2.desc, "! %s text=audio2 ", textoverlay);
  g_string_append_printf (sink2.desc, "! videoconvert ");
  g_string_append_printf (sink2.desc, "! xvimagesink");

  sink3.live_seconds = seconds;
  sink3.desc = g_string_new ("tcpclientsrc port=3005 ");
  g_string_append_printf (sink3.desc, "! gdpdepay ! faad ! goom2k1 ");
  g_string_append_printf (sink3.desc, "! %s text=audio3 ", textoverlay);
  g_string_append_printf (sink3.desc, "! videoconvert ");
  g_string_append_printf (sink3.desc, "! xvimagesink");

  server_pid = launch_server ();
  g_assert_cmpint (server_pid, !=, 0);
  sleep (2); /* give a second for server to be online */

  thread_source1 = testcase_run_thread (&source1, source1.name);
  thread_source2 = testcase_run_thread (&source2, source2.name);
  thread_source3 = testcase_run_thread (&source3, source3.name);
  sleep (2); /* give a second for audios to be online */
  thread_sink1 = testcase_run_thread (&sink1, sink1.name);
  thread_sink2 = testcase_run_thread (&sink2, sink2.name);
  thread_sink3 = testcase_run_thread (&sink3, sink3.name);
  g_thread_join (thread_source1);
  g_thread_join (thread_source2);
  g_thread_join (thread_source3);
  g_thread_join (thread_sink1);
  g_thread_join (thread_sink2);
  g_thread_join (thread_sink3);
  g_thread_unref (thread_source1);
  g_thread_unref (thread_source2);
  g_thread_unref (thread_source3);
  g_thread_unref (thread_sink1);
  g_thread_unref (thread_sink2);
  g_thread_unref (thread_sink3);

  close_server (server_pid);

  g_assert_cmpint (source1.timer, ==, 0);
  g_assert (source1.desc == NULL);
  g_assert (source1.mainloop == NULL);
  g_assert (source1.pipeline == NULL);

  g_assert_cmpint (source2.timer, ==, 0);
  g_assert (source2.desc == NULL);
  g_assert (source2.mainloop == NULL);
  g_assert (source2.pipeline == NULL);

  g_assert_cmpint (source3.timer, ==, 0);
  g_assert (source3.desc == NULL);
  g_assert (source3.mainloop == NULL);
  g_assert (source3.pipeline == NULL);

  {
    GFile *file = g_file_new_for_path ("test-recording.data");
    g_assert (g_file_query_exists (file, NULL));
    g_object_unref (file);
  }
}

static void
test_audio_recording_result (void)
{
  {
    GFile *file = g_file_new_for_path ("test-recording.data");
    GError *error = NULL;
    g_assert (g_file_query_exists (file, NULL));
    g_assert (g_file_delete (file, NULL, &error));
    g_assert (error == NULL);
    g_assert (!g_file_query_exists (file, NULL));
    g_object_unref (file);
  }
}

static void
test_ui (void)
{
  const gint seconds = 40;
  GPid server_pid;
  GPid ui_pid;
  GThread *thread_video_source1;
  GThread *thread_video_source2;
  GThread *thread_video_source3;
  GThread *thread_audio_source1;
  GThread *thread_audio_source2;
  GThread *thread_audio_source3;
  TestCase video_source1 = { "test_video_source1", 0 };
  TestCase video_source2 = { "test_video_source2", 0 };
  TestCase video_source3 = { "test_video_source3", 0 };
  TestCase audio_source1 = { "test_audio_source1", 0 };
  TestCase audio_source2 = { "test_audio_source2", 0 };
  TestCase audio_source3 = { "test_audio_source3", 0 };
  const gchar *textoverlay = "textoverlay "
    "font-desc=\"Sans 80\" "
    "auto-resize=true "
    "shaded-background=true "
    ;

  video_source1.live_seconds = seconds;
  video_source1.desc = g_string_new ("videotestsrc pattern=0 ");
  g_string_append_printf (video_source1.desc, "! video/x-raw,width=1280,height=720 ");
  g_string_append_printf (video_source1.desc, "! %s text=video_source1 ", textoverlay);
  g_string_append_printf (video_source1.desc, "! gdppay ! tcpclientsink port=3000 ");

  video_source2.live_seconds = seconds;
  video_source2.desc = g_string_new ("videotestsrc pattern=1 ");
  g_string_append_printf (video_source2.desc, "! video/x-raw,width=1280,height=720 ");
  g_string_append_printf (video_source2.desc, "! %s text=video_source2 ", textoverlay);
  g_string_append_printf (video_source2.desc, "! gdppay ! tcpclientsink port=3000 ");

  video_source3.live_seconds = seconds;
  video_source3.desc = g_string_new ("videotestsrc pattern=15 ");
  g_string_append_printf (video_source3.desc, "! video/x-raw,width=1280,height=720 ");
  g_string_append_printf (video_source3.desc, "! %s text=video_source3 ", textoverlay);
  g_string_append_printf (video_source3.desc, "! gdppay ! tcpclientsink port=3000 ");

  audio_source1.live_seconds = seconds;
  audio_source1.desc = g_string_new ("audiotestsrc ");
  //g_string_append_printf (audio_source1.desc, "! audio/x-raw ");
  g_string_append_printf (audio_source1.desc, "! gdppay ! tcpclientsink port=4000");

  audio_source2.live_seconds = seconds;
  audio_source2.desc = g_string_new ("audiotestsrc ");
  g_string_append_printf (audio_source2.desc, "! gdppay ! tcpclientsink port=4000");

  audio_source3.live_seconds = seconds;
  audio_source3.desc = g_string_new ("audiotestsrc ");
  g_string_append_printf (audio_source3.desc, "! gdppay ! tcpclientsink port=4000");

  server_pid = launch_server ();
  g_assert_cmpint (server_pid, !=, 0);
  sleep (2); /* give a second for server to be online */

  ui_pid = launch_ui ();
  g_assert_cmpint (ui_pid, !=, 0);
  sleep (1); /* give a second for ui to be ready */

  thread_video_source1 = testcase_run_thread (&video_source1, video_source1.name);
  thread_video_source2 = testcase_run_thread (&video_source2, video_source2.name);
  thread_video_source3 = testcase_run_thread (&video_source3, video_source3.name);
  thread_audio_source1 = testcase_run_thread (&audio_source1, audio_source1.name);
  thread_audio_source2 = testcase_run_thread (&audio_source2, audio_source2.name);
  thread_audio_source3 = testcase_run_thread (&audio_source3, audio_source3.name);
  g_thread_join (thread_video_source1);
  g_thread_join (thread_video_source2);
  g_thread_join (thread_video_source3);
  g_thread_join (thread_audio_source1);
  g_thread_join (thread_audio_source2);
  g_thread_join (thread_audio_source3);
  g_thread_unref (thread_video_source1);
  g_thread_unref (thread_video_source2);
  g_thread_unref (thread_video_source3);
  g_thread_unref (thread_audio_source1);
  g_thread_unref (thread_audio_source2);
  g_thread_unref (thread_audio_source3);

  close_server (ui_pid);
  close_server (server_pid);
}

static void
test_recording_result (void)
{
  {
    GFile *file = g_file_new_for_path ("test-recording.data");
    GError *error = NULL;
    g_assert (g_file_query_exists (file, NULL));
    g_assert (g_file_delete (file, NULL, &error));
    g_assert (error == NULL);
    g_assert (!g_file_query_exists (file, NULL));
    g_object_unref (file);
  }
}

int main (int argc, char**argv)
{
  gst_init (&argc, &argv);
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/gst-switch/controller", test_controller);
  g_test_add_func ("/gst-switch/video", test_video);
  g_test_add_func ("/gst-switch/video-recording-result", test_video_recording_result);
  g_test_add_func ("/gst-switch/audio", test_audio);
  g_test_add_func ("/gst-switch/audio-recording-result", test_audio_recording_result);
  g_test_add_func ("/gst-switch/ui", test_ui);
  g_test_add_func ("/gst-switch/recording-result", test_recording_result);
  return g_test_run ();
}
