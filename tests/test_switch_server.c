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

static struct {
  gboolean disable_test_controller;
  gboolean disable_test_video;
  gboolean disable_test_audio;
  gboolean disable_test_ui_integration;
  gboolean disable_test_random_connection;
  gboolean disable_test_fuzz_ui;
  gboolean test_external_server;
  gboolean test_external_ui;
} opts = {
 .disable_test_controller	= FALSE,
 .disable_test_video		= FALSE,
 .disable_test_audio		= FALSE,
 .disable_test_ui_integration	= FALSE,
 .disable_test_random_connection= FALSE,
 .disable_test_fuzz_ui		= FALSE,
 .test_external_server		= FALSE,
 .test_external_ui		= FALSE,
};

static GOptionEntry option_entries[] = {
  {"disable-test-controller",		0, 0, G_OPTION_ARG_NONE, &opts.disable_test_controller,		"Disable testing controller", NULL},
  {"disable-test-video",		0, 0, G_OPTION_ARG_NONE, &opts.disable_test_video,		"Disable testing video", NULL},
  {"disable-test-audio",		0, 0, G_OPTION_ARG_NONE, &opts.disable_test_audio,		"Disable testing audio", NULL},
  {"disable-test-ui-integration",	0, 0, G_OPTION_ARG_NONE, &opts.disable_test_ui_integration,	"Disable testing UI integration", NULL},
  {"disable-test-random-connection",	0, 0, G_OPTION_ARG_NONE, &opts.disable_test_random_connection,	"Disable testing random connection", NULL},
  {"disable-test-fuzz-ui",		0, 0, G_OPTION_ARG_NONE, &opts.disable_test_fuzz_ui,		"Disable testing fuzz input", NULL},
  {"test-external-server",		0, 0, G_OPTION_ARG_NONE, &opts.test_external_server,		"Testing external server", NULL},
  {"test-external-ui",			0, 0, G_OPTION_ARG_NONE, &opts.test_external_ui,		"Testing external ui", NULL},
  {NULL}
};

typedef struct _TestCase TestCase;
struct _TestCase
{
  const gchar *name;
  GMainLoop *mainloop;
  GstElement *pipeline;
  GMutex lock;
  GThread *thread;
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
    /*
    INFO ("quit: %s\n", t->name);
    */
    testcase_ok (t);
    break;
  default:
    break;
  }
}

static void
testcase_error_message (TestCase *t, GError *error, const gchar *info)
{
  /*
  ERROR ("%s: %s", t->name, error->message);
  */

  /*
  g_print ("%s\n", info);
  */

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
    /*
    INFO ("%s: %s", GST_OBJECT_NAME (message->src), GST_MESSAGE_TYPE_NAME (message));
    */
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
child_quit (GPid pid, gint status, gpointer data)
{
  GOutputStream *ostream = G_OUTPUT_STREAM (data);
  GError *error = NULL;

  INFO ("quit %d", pid);

  g_output_stream_flush (ostream, NULL, &error);
  g_assert_no_error (error);
  g_output_stream_close (ostream, NULL, &error);
  g_assert_no_error (error);
  g_object_unref (ostream);
}

static void
child_stdout (GIOChannel *channel, GIOCondition condition, gpointer data)
{
  char buf[1024];
  GError *error = NULL;
  gsize bytes_read;
  GOutputStream *ostream = G_OUTPUT_STREAM (data);

  //INFO ("out");

  if (condition & G_IO_IN) {
    gsize bytes_written;
    GIOStatus status = g_io_channel_read_chars (channel, buf, sizeof (buf), &bytes_read, &error);
    g_assert_no_error (error);
    g_output_stream_write_all (ostream, buf, bytes_read, &bytes_written, NULL, &error);
    g_assert_no_error (error);
    g_assert_cmpint (bytes_read, ==, bytes_written);
    (void) status;
  }
  if (condition & G_IO_HUP) {
    g_output_stream_flush (ostream, NULL, &error);
    g_assert_no_error (error);
  }
}

static void
child_stderr (GIOChannel *channel, GIOCondition condition, gpointer data)
{
  //INFO ("err");
}

static GPid
launch (const gchar *name, ...)
{
  GPid pid = 0;
  GError *error = NULL;
  GMainContext *context;
  GIOChannel *channel;
  GSource *source;
  gint fd_in, fd_out, fd_err;
  GFileOutputStream *outstream;
  GFile *outfile;
  gchar *outfilename;
  gchar **argv;
  gboolean ok;
  GPtrArray *array = g_ptr_array_new ();
  const gchar *arg;
  va_list va;
  va_start (va, name);
  for (arg = name; arg; arg = va_arg (va, const gchar *))
    g_ptr_array_add (array, g_strdup (arg));
  va_end (va);
  g_ptr_array_add (array, NULL);
  argv = (gchar **) g_ptr_array_free (array, FALSE);

  ok = g_spawn_async_with_pipes (NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD,
      NULL, NULL, &pid, &fd_in, &fd_out, &fd_err, &error);

  g_free (argv);

  g_assert_no_error (error);
  g_assert (ok);

  context = g_main_context_default ();

  outfilename = g_strdup_printf ("test-server-%d.log", pid);
  outfile = g_file_new_for_path (outfilename);
  outstream = g_file_create (outfile, G_FILE_CREATE_NONE, NULL, &error);

  g_free (outfilename);
  g_assert_no_error (error);
  g_assert (outfile);
  g_assert (outstream);

  channel = g_io_channel_unix_new (fd_out);
  source = g_io_create_watch (channel, G_IO_IN | G_IO_HUP | G_IO_ERR);
  g_source_set_callback (source, (GSourceFunc) child_stdout, outstream, NULL);
  g_source_attach (source, context);
  g_source_unref (source);
  g_io_channel_unref (channel);

  (void) child_stderr;
  /*
  channel = g_io_channel_unix_new (fd_err);
  source = g_io_create_watch (channel, G_IO_IN | G_IO_HUP | G_IO_ERR);
  g_source_set_callback (source, (GSourceFunc) child_stderr, NULL, NULL);
  g_source_attach (source, context);
  g_source_unref (source);
  g_io_channel_unref (channel);
  */

  g_child_watch_add (pid, child_quit, outstream);

  //g_main_context_unref (context);
  return pid;
}

static GPid
launch_server ()
{
  GPid pid = launch ("./tools/gst-switch-srv", "-v",
      "--gst-debug-no-color",
      "--record=test-recording.data",
      NULL);
  INFO ("server %d", pid);
  return pid;
}

static GPid
launch_ui ()
{
  GPid pid = launch ("./tools/gst-switch-ui", "-v",
      "--gst-debug-no-color",
      NULL);
  INFO ("ui %d", pid);
  return pid;
}

static void
close_pid (GPid pid)
{
  //kill (pid, SIGKILL);
  kill (pid, SIGTERM);
  g_spawn_close_pid (pid);
  sleep (2); /* give a second for cleaning up */
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
  g_mutex_init (&t->lock);

  g_print ("========== %s\n", t->name);
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

  if (t->error_count)
    ERROR ("%s: %d errors", t->name, t->error_count);

  g_mutex_lock (&t->lock);
  g_thread_unref (t->thread);
  t->thread = NULL;
  g_mutex_unlock (&t->lock);
  return NULL;
}

static void
testcase_run_thread (TestCase *t)
{
  t->thread = g_thread_new (t->name, (GThreadFunc) testcase_run, t);
}

static void
testcase_join (TestCase *t)
{
  GThread *thread = NULL;
  g_mutex_lock (&t->lock);
  if (t->thread) {
    thread = t->thread;
    g_thread_ref (thread);
  }
  g_mutex_unlock (&t->lock);
  if (thread) g_thread_join (thread);
}

static void
test_controller (void)
{
  g_print ("\n");

  WARN ("TODO: test controller");
}

static void
test_video (void)
{
  const gint seconds = 10;
  GPid server_pid = 0;
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

  g_print ("\n");
  g_assert (!source1.thread);
  g_assert (!source2.thread);
  g_assert (!source3.thread);
  g_assert (!sink0.thread);
  g_assert (!sink1.thread);
  g_assert (!sink2.thread);
  g_assert (!sink3.thread);

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

  if (!opts.test_external_server) {
    server_pid = launch_server ();
    g_assert_cmpint (server_pid, !=, 0);
    sleep (2); /* give a second for server to be online */
  }

  testcase_run_thread (&source1);
  sleep (1); /* give a second for source1 to be online */
  testcase_run_thread (&source2);
  testcase_run_thread (&source3);
  sleep (1); /* give a second for sources to be online */
  testcase_run_thread (&sink0);
  testcase_run_thread (&sink1);
  testcase_run_thread (&sink2);
  testcase_run_thread (&sink3);
  testcase_join (&source1);
  testcase_join (&source2);
  testcase_join (&source3);
  testcase_join (&sink0);
  testcase_join (&sink1);
  testcase_join (&sink2);
  testcase_join (&sink3);

  if (!opts.test_external_server)
    close_pid (server_pid);

  g_assert_cmpstr (source1.name, ==, "test_video_source1");
  g_assert_cmpint (source1.timer, ==, 0);
  g_assert (source1.desc == NULL);
  g_assert (source1.mainloop == NULL);
  g_assert (source1.pipeline == NULL);

  g_assert_cmpstr (source2.name, ==, "test_video_source2");
  g_assert_cmpint (source2.timer, ==, 0);
  g_assert (source2.desc == NULL);
  g_assert (source2.mainloop == NULL);
  g_assert (source2.pipeline == NULL);

  g_assert_cmpstr (source3.name, ==, "test_video_source3");
  g_assert_cmpint (source3.timer, ==, 0);
  g_assert (source3.desc == NULL);
  g_assert (source3.mainloop == NULL);
  g_assert (source3.pipeline == NULL);

  g_assert_cmpstr (sink0.name, ==, "test_video_compose_sink");
  g_assert_cmpint (sink0.timer, ==, 0);
  g_assert (sink0.desc == NULL);
  g_assert (sink0.mainloop == NULL);
  g_assert (sink0.pipeline == NULL);

  g_assert_cmpstr (sink1.name, ==, "test_video_preview_sink1");
  g_assert_cmpint (sink1.timer, ==, 0);
  g_assert (sink1.desc == NULL);
  g_assert (sink1.mainloop == NULL);
  g_assert (sink1.pipeline == NULL);

  g_assert_cmpstr (sink2.name, ==, "test_video_preview_sink2");
  g_assert_cmpint (sink2.timer, ==, 0);
  g_assert (sink2.desc == NULL);
  g_assert (sink2.mainloop == NULL);
  g_assert (sink2.pipeline == NULL);

  g_assert_cmpstr (sink3.name, ==, "test_video_preview_sink3");
  g_assert_cmpint (sink3.timer, ==, 0);
  g_assert (sink3.desc == NULL);
  g_assert (sink3.mainloop == NULL);
  g_assert (sink3.pipeline == NULL);

  if (!opts.test_external_server) {
    GFile *file = g_file_new_for_path ("test-recording.data");
    g_assert (g_file_query_exists (file, NULL));
    g_object_unref (file);
  }
}

static void
test_video_recording_result (void)
{
  g_print ("\n");
  if (!opts.test_external_server) {
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
  const gint seconds = 10;
  TestCase source1 = { "test_audio_source1", 0 };
  TestCase source2 = { "test_audio_source2", 0 };
  TestCase source3 = { "test_audio_source3", 0 };
  TestCase sink1 = { "test_audio_preview_sink1", 0 };
  TestCase sink2 = { "test_audio_preview_sink2", 0 };
  TestCase sink3 = { "test_audio_preview_sink3", 0 };
  GPid server_pid = 0;
  const gchar *textoverlay = "textoverlay "
    "font-desc=\"Sans 80\" "
    "auto-resize=true "
    "shaded-background=true "
    ;

  g_print ("\n");
  g_assert (!source1.thread);
  g_assert (!source2.thread);
  g_assert (!source3.thread);
  g_assert (!sink1.thread);
  g_assert (!sink2.thread);
  g_assert (!sink3.thread);

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

  if (!opts.test_external_server) {
    server_pid = launch_server ();
    g_assert_cmpint (server_pid, !=, 0);
    sleep (3); /* give a second for server to be online */
  }

  testcase_run_thread (&source1);
  testcase_run_thread (&source2);
  testcase_run_thread (&source3);
  sleep (2); /* give a second for audios to be online */
  testcase_run_thread (&sink1);
  testcase_run_thread (&sink2);
  testcase_run_thread (&sink3);
  testcase_join (&source1);
  testcase_join (&source2);
  testcase_join (&source3);
  testcase_join (&sink1);
  testcase_join (&sink2);
  testcase_join (&sink3);

  if (!opts.test_external_server)
    close_pid (server_pid);

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

  if (!opts.test_external_server) {
    GFile *file = g_file_new_for_path ("test-recording.data");
    g_assert (g_file_query_exists (file, NULL));
    g_object_unref (file);
  }
}

static void
test_audio_recording_result (void)
{
  g_print ("\n");
  if (!opts.test_external_server) {
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
test_ui_integrated (void)
{
  const gint seconds = 10;
  GPid server_pid = 0;
  GPid ui_pid;
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

  g_print ("\n");

  video_source1.live_seconds = seconds;
  video_source1.desc = g_string_new ("videotestsrc pattern=0 ");
  g_string_append_printf (video_source1.desc, "! video/x-raw,width=1280,height=720 ");
  g_string_append_printf (video_source1.desc, "! %s text=video1 ", textoverlay);
  g_string_append_printf (video_source1.desc, "! gdppay ! tcpclientsink port=3000 ");

  video_source2.live_seconds = seconds;
  video_source2.desc = g_string_new ("videotestsrc pattern=1 ");
  g_string_append_printf (video_source2.desc, "! video/x-raw,width=1280,height=720 ");
  g_string_append_printf (video_source2.desc, "! %s text=video2 ", textoverlay);
  g_string_append_printf (video_source2.desc, "! gdppay ! tcpclientsink port=3000 ");

  video_source3.live_seconds = seconds;
  video_source3.desc = g_string_new ("videotestsrc pattern=15 ");
  g_string_append_printf (video_source3.desc, "! video/x-raw,width=1280,height=720 ");
  g_string_append_printf (video_source3.desc, "! %s text=video3 ", textoverlay);
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

  if (!opts.test_external_server) {
    server_pid = launch_server ();
    g_assert_cmpint (server_pid, !=, 0);
    sleep (3); /* give a second for server to be online */
  }

  ui_pid = launch_ui ();
  g_assert_cmpint (ui_pid, !=, 0);
  sleep (2); /* give a second for ui to be ready */

  testcase_run_thread (&video_source1); //sleep (1);
  testcase_run_thread (&video_source2); //sleep (1);
  testcase_run_thread (&video_source3); //sleep (1);
  testcase_run_thread (&audio_source1); //sleep (1);
  testcase_run_thread (&audio_source2); //sleep (1);
  testcase_run_thread (&audio_source3); //sleep (1);
  testcase_join (&video_source1);
  testcase_join (&video_source2);
  testcase_join (&video_source3);
  testcase_join (&audio_source1);
  testcase_join (&audio_source2);
  testcase_join (&audio_source3);

  close_pid (ui_pid);
  if (!opts.test_external_server)
    close_pid (server_pid);
}

static void
test_recording_result (void)
{
  g_print ("\n");
  if (!opts.test_external_server) {
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
test_random_connections (void)
{
  GPid server_pid = 0;
  GPid ui_pid = 0;
  TestCase video_source1 = { "test_video_source1", 0 };
  //TestCase video_source2 = { "test_video_source2", 0 };
  //TestCase video_source3 = { "test_video_source3", 0 };
  TestCase audio_source1 = { "test_audio_source1", 0 };
  //TestCase audio_source2 = { "test_audio_source2", 0 };
  //TestCase audio_source3 = { "test_audio_source3", 0 };
  const gchar *textoverlay = "textoverlay "
    "font-desc=\"Sans 80\" "
    "auto-resize=true "
    "shaded-background=true "
    ;
  gint n, m;

  g_print ("\n");

  if (!opts.test_external_server) {
    server_pid = launch_server ();
    g_assert_cmpint (server_pid, !=, 0);
    sleep (2); /* give a second for server to be online */
  }

  for (m = 0; m < 3; ++n) {
    if (!opts.test_external_ui) {
      ui_pid = launch_ui ();
      g_assert_cmpint (ui_pid, !=, 0);
      sleep (1); /* give a second for ui to be ready */
    }

    for (n = 0; n < 3; ++n) {
      video_source1.live_seconds = 5;
      video_source1.desc = g_string_new ("videotestsrc pattern=0 ");
      g_string_append_printf (video_source1.desc, "! video/x-raw,width=1280,height=720 ");
      g_string_append_printf (video_source1.desc, "! %s text=video1-%d ", textoverlay, n);
      g_string_append_printf (video_source1.desc, "! gdppay ! tcpclientsink port=3000 ");

      audio_source1.live_seconds = 5;
      audio_source1.desc = g_string_new ("audiotestsrc ");
      g_string_append_printf (audio_source1.desc, "! gdppay ! tcpclientsink port=4000");

      testcase_run_thread (&video_source1);
      testcase_run_thread (&audio_source1);
      testcase_join (&video_source1);
      testcase_join (&audio_source1);
    }

    for (n = 0; n < 3; ++n) {
      video_source1.live_seconds = 2;
      video_source1.desc = g_string_new ("videotestsrc pattern=0 ");
      g_string_append_printf (video_source1.desc, "! video/x-raw,width=1280,height=720 ");
      g_string_append_printf (video_source1.desc, "! %s text=video1-%d ", textoverlay, n);
      g_string_append_printf (video_source1.desc, "! gdppay ! tcpclientsink port=3000 ");

      audio_source1.live_seconds = 2;
      audio_source1.desc = g_string_new ("audiotestsrc ");
      g_string_append_printf (audio_source1.desc, "! gdppay ! tcpclientsink port=4000");

      testcase_run_thread (&video_source1);
      testcase_run_thread (&audio_source1);
      testcase_join (&video_source1);
      testcase_join (&audio_source1);
    }

    if (!opts.test_external_ui)
      close_pid (ui_pid);
  }

  if (!opts.test_external_server)
    close_pid (server_pid);
}

static void
test_fuzz_ui_integrated (void)
{
  const gint seconds = 10;
  GPid server_pid = 0;
  GPid ui_pid;
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

  g_print ("\n");

  video_source1.live_seconds = seconds;
  video_source1.desc = g_string_new ("videotestsrc pattern=0 ");
  g_string_append_printf (video_source1.desc, "! video/x-raw,width=1280,height=720 ");
  g_string_append_printf (video_source1.desc, "! %s text=video1 ", textoverlay);
  g_string_append_printf (video_source1.desc, "! gdppay ! tcpclientsink port=3000 ");

  video_source2.live_seconds = seconds;
  video_source2.desc = g_string_new ("videotestsrc pattern=1 ");
  g_string_append_printf (video_source2.desc, "! video/x-raw,width=1280,height=720 ");
  g_string_append_printf (video_source2.desc, "! %s text=video2 ", textoverlay);
  g_string_append_printf (video_source2.desc, "! gdppay ! tcpclientsink port=3000 ");

  video_source3.live_seconds = seconds;
  video_source3.desc = g_string_new ("videotestsrc pattern=15 ");
  g_string_append_printf (video_source3.desc, "! video/x-raw,width=1280,height=720 ");
  g_string_append_printf (video_source3.desc, "! %s text=video3 ", textoverlay);
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

  if (!opts.test_external_server) {
    server_pid = launch_server ();
    g_assert_cmpint (server_pid, !=, 0);
    sleep (3); /* give a second for server to be online */
  }

  ui_pid = launch_ui ();
  g_assert_cmpint (ui_pid, !=, 0);
  sleep (2); /* give a second for ui to be ready */

  testcase_run_thread (&video_source1); //sleep (1);
  testcase_run_thread (&video_source2); //sleep (1);
  testcase_run_thread (&video_source3); //sleep (1);
  testcase_run_thread (&audio_source1); //sleep (1);
  testcase_run_thread (&audio_source2); //sleep (1);
  testcase_run_thread (&audio_source3); //sleep (1);
  testcase_join (&video_source1);
  testcase_join (&video_source2);
  testcase_join (&video_source3);
  testcase_join (&audio_source1);
  testcase_join (&audio_source2);
  testcase_join (&audio_source3);

  close_pid (ui_pid);
  if (!opts.test_external_server)
    close_pid (server_pid);
}

int main (int argc, char**argv)
{
  {
    GOptionContext *context;
    GOptionGroup *group;
    GError *error = NULL;
    gboolean ok;
    group = g_option_group_new ("gst-switch-test", "gst-switch test suite",
	"",
	NULL, NULL);
    context = g_option_context_new ("");
    g_option_context_add_main_entries (context, option_entries, "test-switch-server");
    g_option_context_add_group (context, group);
    ok = g_option_context_parse (context, &argc, &argv, &error);
    g_option_context_free (context);
    if (!ok) {
      g_print ("option parsing failed: %s\n", error->message);
      return 1;
    }
  }

  gst_init (&argc, &argv);
  g_test_init (&argc, &argv, NULL);
  if (!opts.disable_test_controller) {
    g_test_add_func ("/gst-switch/controller", test_controller);
  }
  if (!opts.disable_test_video) {
    g_test_add_func ("/gst-switch/video", test_video);
    g_test_add_func ("/gst-switch/video-recording-result", test_video_recording_result);
  }
  if (!opts.disable_test_audio) {
    g_test_add_func ("/gst-switch/audio", test_audio);
    g_test_add_func ("/gst-switch/audio-recording-result", test_audio_recording_result);
  }
  if (!opts.disable_test_ui_integration) {
    g_test_add_func ("/gst-switch/ui-integrated", test_ui_integrated);
    g_test_add_func ("/gst-switch/recording-result", test_recording_result);
  }
  if (!opts.disable_test_random_connection) {
    g_test_add_func ("/gst-switch/random-connections", test_random_connections);
    g_test_add_func ("/gst-switch/recording-result", test_recording_result);
  }
  if (!opts.disable_test_fuzz_ui) {
    g_test_add_func ("/gst-switch/fuzz-ui-integrated", test_fuzz_ui_integrated);
    g_test_add_func ("/gst-switch/recording-result", test_recording_result);
  }
  return g_test_run ();
}
