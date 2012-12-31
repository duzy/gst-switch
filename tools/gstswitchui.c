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

#include <gst/video/videooverlay.h>
#include <stdlib.h>
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
gst_switchui_quit (GstSwitchUI * ui)
{
  gtk_main_quit ();
}

static void
gst_switchui_window_closed (GtkWidget * widget, GdkEvent * event,
    gpointer data)
{
  GstSwitchUI * ui = GST_SWITCH_UI (data);
  gst_switchui_quit (ui);
}

static gboolean
gst_switchui_compose_view_expose (GtkWidget * widget, GdkEventExpose * event,
    gpointer data)
{
  GstSwitchUI * ui = GST_SWITCH_UI (data);
  return FALSE;
}

static gboolean
gst_switchui_compose_view_motion (GtkWidget * widget, GdkEventMotion * event,
    gpointer data)
{
  GstSwitchUI * ui = GST_SWITCH_UI (data);
  return FALSE;
}

static gboolean
gst_switchui_compose_view_press (GtkWidget * widget, GdkEventButton * event,
    gpointer data)
{
  return FALSE;
}

static void
gst_switchui_init (GstSwitchUI * ui)
{
  ui->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (ui->window), 640, 480);
  gtk_window_set_title (GTK_WINDOW (ui->window), "GstSwitch");
  g_signal_connect (G_OBJECT (ui->window), "delete-event",
      G_CALLBACK (gst_switchui_window_closed), ui);

  ui->compose_view = gtk_drawing_area_new ();
  g_signal_connect (G_OBJECT (ui->compose_view), "expose-event",
      G_CALLBACK (gst_switchui_compose_view_expose), ui);
  g_signal_connect (G_OBJECT (ui->compose_view), "motion-notify-event",
      G_CALLBACK (gst_switchui_compose_view_motion), ui);
  g_signal_connect (G_OBJECT (ui->compose_view), "button-press-event",
      G_CALLBACK (gst_switchui_compose_view_press), ui);

  gtk_widget_set_double_buffered (ui->compose_view, FALSE);
  gtk_widget_set_events (ui->compose_view, GDK_EXPOSURE_MASK
      //| GDK_LEAVE_NOTIFY_MASK
      | GDK_BUTTON_PRESS_MASK
      //| GDK_POINTER_MOTION_MASK
      //| GDK_POINTER_MOTION_HINT_MASK
      );

  gtk_container_add (GTK_CONTAINER (ui->window), ui->compose_view);
  gtk_container_set_border_width (GTK_CONTAINER (ui->window), 5);
}

static void
gst_switchui_finalize (GstSwitchUI * ui)
{
  gtk_widget_destroy (GTK_WIDGET (ui->window));
  ui->window = NULL;

  if (G_OBJECT_CLASS (gst_switchui_parent_class)->finalize)
    (*G_OBJECT_CLASS (gst_switchui_parent_class)->finalize) (G_OBJECT (ui));
}

static void
gst_switchui_prepare_connections (GstSwitchUI * ui)
{
#if 0
  GdkWindow *xview = gtk_widget_get_window (ui->compose_view);
  gulong xid = GDK_WINDOW_XID (xview);
  GstElement *sink = /* xvimagesink */;
  gst_video_overlay_set_window_handle (GST_VIDEO_OVERLAY (sink), xid);
#endif
}

static void
gst_switchui_run (GstSwitchUI * ui)
{
  gtk_widget_show_all (ui->window);
  gtk_widget_realize (ui->window);
  gst_switchui_prepare_connections (ui);
  gtk_main ();
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
  gtk_init (&argc, &argv);

  ui = GST_SWITCH_UI (g_object_new (GST_TYPE_SWITCH_UI, NULL));

  gst_switchui_run (ui);

  g_object_unref (G_OBJECT (ui));
  return 0;
}
