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

G_DEFINE_TYPE (GstSwitchServer, gst_switchsrv, G_TYPE_OBJECT);

GstSwitchServerOpts opts = {
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
gst_switchsrv_init (GstSwitchServer *srv)
{
  srv->switcher = gst_switcher_new (srv);
  srv->compositor = gst_compositor_new (srv);
}

static void
gst_switchsrv_finalize (GstSwitchServer *srv)
{
  gst_switcher_free (srv->switcher);
  gst_compositor_free (srv->compositor);
  srv->switcher = NULL;
  srv->compositor = NULL;
}

static void
gst_switchsrv_class_init (GstSwitchServerClass *klass)
{
  GObjectClass * object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = (GObjectFinalizeFunc) gst_switchsrv_finalize;
}

static gpointer
gst_switchsrv_acceptor_worker (GstSwitchServer *srv)
{
  return NULL;
}

static void
gst_switchsrv_run(GstSwitchServer * srv)
{
  GMainLoop *main_loop;
  GThread *acceptor;

  gst_switcher_prepare (srv->switcher);
  gst_switcher_start (srv->switcher);

  gst_compositor_prepare (srv->compositor);
  gst_compositor_start (srv->compositor);

  main_loop = g_main_loop_new (NULL, TRUE);
  srv->switcher->main_loop = main_loop;
  srv->compositor->main_loop = main_loop;

  acceptor = g_thread_new ("switch-server-acceptor",
      (GThreadFunc) gst_switchsrv_acceptor_worker, NULL);

  g_main_loop_run (main_loop);

  g_thread_join (acceptor);
}

int
main (int argc, char *argv[])
{
  GstSwitchServer *srv;

  gst_switchsrv_parse_args (&argc, &argv);

  srv = GST_SWITCH_SERVER (g_object_new (GST_SWITCH_SERVER_TYPE, NULL));

  gst_switchsrv_run (srv);

  g_object_unref (G_OBJECT (srv));
  return 0;
}
