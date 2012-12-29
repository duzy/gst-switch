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
#include "gstswitchsrv.h"

#define GETTEXT_PACKAGE "switchsrv"

GstSwitchSrvOpts opts = {
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
gst_switchsrv_parse_args (int *argc, char **argv[])
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

static void
run(GstSwitcher *switcher, GstCompositor *compositor)
{
  GMainLoop *main_loop;
  GstElement *pipeline;

  pipeline = gst_switcher_create_pipeline (&switcher);
  gst_switcher_set_pipeline (&switcher, pipeline);
  gst_switcher_start (&switcher);

  pipeline = gst_compositor_create_pipeline (compositor);
  gst_compositor_set_pipeline (compositor, pipeline);
  gst_compositor_start (compositor);

  main_loop = g_main_loop_new (NULL, TRUE);

  switcher->main_loop = main_loop;
  compositor->main_loop = main_loop;

  g_main_loop_run (main_loop);
}

int
main (int argc, char *argv[])
{
  GstSwitcher switcher;
  GstCompositor compositor;
  GThread *switch_thread;
  GThread *compositor_thread;

  gst_switchsrv_parse_args (&argc, &argv);

  gst_switcher_init (&switcher, &compositor);
  gst_compositor_init (&compositor, &switcher);

#if 0
  switch_thread = g_thread_new ("switcher",
      (GThreadFunc) gst_switcher_run, &switcher);

  compositor_thread = g_thread_new ("compositor",
      (GThreadFunc) gst_compositor_run,& compositor);

  g_thread_join (switch_thread);
  g_thread_join (compositor_thread);
#else

  run (&switcher, &compositor);
  
#endif

  gst_switcher_fini (&switcher);
  gst_compositor_fini (&compositor);
  return 0;
}
