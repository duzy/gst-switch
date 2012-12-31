/* GstSwitchUI
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

#include "gstswitchui.h"

G_DEFINE_TYPE (GstSwitchUI, gst_switchui, G_TYPE_OBJECT);

gboolean verbose;

static GOptionEntry entries[] = {
  {"verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Be verbose", NULL},
  {NULL}
};

static void
gst_switchui_parse_args (int *argc, char **argv[])
{
  GOptionContext *context;
  GError *error = NULL;
  context = g_option_context_new ("");
  g_option_context_add_main_entries (context, entries, "switchui");
  g_option_context_add_group (context, gst_init_get_option_group ());
  if (!g_option_context_parse (context, argc, argv, &error)) {
    g_print ("option parsing failed: %s\n", error->message);
    exit (1);
  }
  g_option_context_free (context);
}

static void
gst_switchui_init (GstSwitchUI * ui)
{
  ui->main_loop = NULL;
}

static void
gst_switchui_finalize (GstSwitchUI * ui)
{
  g_main_loop_quit (ui->main_loop);
  g_main_loop_unref (ui->main_loop);
  ui->main_loop = NULL;

  if (G_OBJECT_CLASS (gst_switchui_parent_class)->finalize)
    (*G_OBJECT_CLASS (gst_switchui_parent_class)->finalize) (G_OBJECT (ui));
}

static void
gst_switchui_quit (GstSwitchUI * ui)
{
  g_main_loop_quit (ui->main_loop);
}

static void
gst_switchui_run (GstSwitchUI * ui)
{
  ui->main_loop = g_main_loop_new (NULL, TRUE);

  g_main_loop_run (ui->main_loop);
}

static void
gst_switchui_class_init (GstSwitchUIClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = (GObjectFinalizeFunc) gst_switchui_finalize;
}

int
main (int argc, char *argv[])
{
  GstSwitchUI *ui;

  gst_switchui_parse_args (&argc, &argv);

  ui = GST_SWITCH_UI (g_object_new (GST_TYPE_SWITCH_UI, NULL));

  gst_switchui_run (ui);

  g_object_unref (G_OBJECT (ui));
  return 0;
}
