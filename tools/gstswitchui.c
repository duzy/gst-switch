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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include "gstswitchui.h"
#include "gstswitchcontroller.h"
#include "gstvideodisp.h"
#include "gstaudiovisual.h"
#include "gstaudioplay.h"
#include "gstcase.h"

#define GST_SWITCH_UI_LOCK_AUDIO(ui) (g_mutex_lock (&(ui)->audio_lock))
#define GST_SWITCH_UI_UNLOCK_AUDIO(ui) (g_mutex_unlock (&(ui)->audio_lock))

G_DEFINE_TYPE (GstSwitchUI, gst_switch_ui, GST_TYPE_SWITCH_CLIENT);

gboolean verbose;

static GOptionEntry entries[] = {
  {"verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Be verbose", NULL},
  {NULL}
};

static void
gst_switch_ui_parse_args (int *argc, char **argv[])
{
  GOptionContext *context;
  GError *error = NULL;
  context = g_option_context_new ("");
  g_option_context_add_main_entries (context, entries, "gst-switch");
  g_option_context_add_group (context, gst_init_get_option_group ());
  if (!g_option_context_parse (context, argc, argv, &error)) {
    g_print ("option parsing failed: %s\n", error->message);
    exit (1);
  }
  g_option_context_free (context);
}

static void
gst_switch_ui_quit (GstSwitchUI * ui)
{
  gtk_main_quit ();
}

static void
gst_switch_ui_window_closed (GtkWidget * widget, GdkEvent * event,
    gpointer data)
{
  GstSwitchUI * ui = GST_SWITCH_UI (data);
  gst_switch_ui_quit (ui);
}

static gboolean
gst_switch_ui_compose_view_expose (GtkWidget * widget, GdkEventExpose * event,
    gpointer data)
{
  GstSwitchUI * ui = GST_SWITCH_UI (data);
  (void) ui;
  return FALSE;
}

static gboolean
gst_switch_ui_compose_view_motion (GtkWidget * widget, GdkEventMotion * event,
    gpointer data)
{
  GstSwitchUI * ui = GST_SWITCH_UI (data);
  (void) ui;
  return FALSE;
}

static gboolean
gst_switch_ui_compose_view_press (GtkWidget * widget, GdkEventButton * event,
    gpointer data)
{
  GstSwitchUI * ui = GST_SWITCH_UI (data);
  (void) ui;
  return FALSE;
}

static void
gst_switch_ui_init (GstSwitchUI * ui)
{
  GtkWidget *main_box;
  GtkWidget *scrollwin;

  g_mutex_init (&ui->audio_lock);

  ui->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (ui->window), 640, 480);
  gtk_window_set_title (GTK_WINDOW (ui->window), "GstSwitch");

  main_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
  ui->preview_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
  gtk_widget_set_size_request (ui->preview_box, 120, -1);
  gtk_widget_set_vexpand (ui->preview_box, TRUE);

  ui->compose_view = gtk_drawing_area_new ();
  gtk_widget_set_double_buffered (ui->compose_view, FALSE);
  gtk_widget_set_hexpand (ui->compose_view, TRUE);
  gtk_widget_set_vexpand (ui->compose_view, TRUE);
  gtk_widget_set_events (ui->compose_view, GDK_EXPOSURE_MASK
      //| GDK_LEAVE_NOTIFY_MASK
      | GDK_BUTTON_PRESS_MASK
      //| GDK_POINTER_MOTION_MASK
      //| GDK_POINTER_MOTION_HINT_MASK
      );

  scrollwin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollwin),
      GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_size_request (scrollwin, 130, -1);
  gtk_widget_set_vexpand (scrollwin, TRUE);
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrollwin),
      ui->preview_box);

  gtk_container_add (GTK_CONTAINER (main_box), scrollwin);
  gtk_container_add (GTK_CONTAINER (main_box), ui->compose_view);
  gtk_container_add (GTK_CONTAINER (ui->window), main_box);
  gtk_container_set_border_width (GTK_CONTAINER (ui->window), 5);

  g_signal_connect (G_OBJECT (ui->window), "delete-event",
      G_CALLBACK (gst_switch_ui_window_closed), ui);

  g_signal_connect (G_OBJECT (ui->compose_view), "expose-event",
      G_CALLBACK (gst_switch_ui_compose_view_expose), ui);
  g_signal_connect (G_OBJECT (ui->compose_view), "motion-notify-event",
      G_CALLBACK (gst_switch_ui_compose_view_motion), ui);
  g_signal_connect (G_OBJECT (ui->compose_view), "button-press-event",
      G_CALLBACK (gst_switch_ui_compose_view_press), ui);
}

static void
gst_switch_ui_finalize (GstSwitchUI * ui)
{
  gtk_widget_destroy (GTK_WIDGET (ui->window));
  ui->window = NULL;

  g_mutex_clear (&ui->audio_lock);

  if (G_OBJECT_CLASS (gst_switch_ui_parent_class)->finalize)
    (*G_OBJECT_CLASS (gst_switch_ui_parent_class)->finalize) (G_OBJECT (ui));
}

static void
gst_switch_ui_on_controller_closed (GstSwitchUI * ui, GError *error)
{
  GtkWidget * msg = gtk_message_dialog_new (GTK_WINDOW (ui->window),
      GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
      "Switch server closed.\n"
      "%s", error->message);
  gtk_widget_show_now (msg);
  gtk_main_quit ();
}

static void gst_switch_ui_set_audio_port (GstSwitchUI *, gint);
static void gst_switch_ui_set_compose_port (GstSwitchUI *, gint);
static void gst_switch_ui_add_preview_port (GstSwitchUI *, gint, gint);

static void
gst_switch_ui_prepare_videos (GstSwitchUI * ui)
{
  GVariant *preview_ports;
  gsize n, num_previews = 0;
  gint port;

  port = gst_switch_client_get_compose_port (GST_SWITCH_CLIENT (ui));
  gst_switch_ui_set_compose_port (ui, port);

  port = gst_switch_client_get_audio_port (GST_SWITCH_CLIENT (ui));
  gst_switch_ui_set_audio_port (ui, port);

  port = gst_switch_client_get_encode_port (GST_SWITCH_CLIENT (ui));
  INFO ("Encoded output port: %d", port);

  preview_ports = gst_switch_client_get_preview_ports (GST_SWITCH_CLIENT (ui));
  if (preview_ports) {
    GVariant *ports = NULL;
    GError *error = NULL;
    gchar *s = NULL;
    gint type;

    g_variant_get (preview_ports, "(&s)", &s);
    ports = g_variant_parse (G_VARIANT_TYPE ("a(ii)"), s, NULL, NULL, &error);

    num_previews = g_variant_n_children (ports);
    for (n = 0; n < num_previews; ++n) {
      g_variant_get_child (ports, n, "(ii)", &port, &type);
      gst_switch_ui_add_preview_port (ui, port, type);
      //INFO ("preview: %d, %d", port, type);
    }
  }
}

static void
gst_switch_ui_run (GstSwitchUI * ui)
{
  gst_switch_client_connect (GST_SWITCH_CLIENT (ui));

  gtk_widget_show_all (ui->window);
  gtk_widget_realize (ui->window);
  gst_switch_ui_prepare_videos (ui);
  gtk_main ();
}

static GstVideoDisp *
gst_switch_ui_new_video_disp (GstSwitchUI *ui, GtkWidget *view, gint port)
{
  gchar *name = g_strdup_printf ("video-%d", port);
  GdkWindow *xview = gtk_widget_get_window (view);
  GstVideoDisp *disp = GST_VIDEO_DISP (g_object_new (GST_TYPE_VIDEO_DISP,
	  "name", name, "port", port,
	  "handle", (gulong) GDK_WINDOW_XID (xview),
	  NULL));
  g_object_set_data (G_OBJECT (view), "video-display", disp);
  gst_worker_prepare (GST_WORKER (disp));
  gst_worker_start (GST_WORKER (disp));
  g_free (name);
  return disp;
}

static GstAudioVisual *
gst_switch_ui_new_audio_visual (GstSwitchUI *ui, GtkWidget *view, gint port)
{
  gchar *name = g_strdup_printf ("visual-%d", port);
  GdkWindow *xview = gtk_widget_get_window (view);
  GstAudioVisual *visual;
  gint audio_port = 0;

  if (ui->audio) {
    GST_SWITCH_UI_LOCK_AUDIO (ui);
    if (ui->audio) audio_port = ui->audio->port;
    GST_SWITCH_UI_UNLOCK_AUDIO (ui);
  }

  visual = GST_AUDIO_VISUAL (
      g_object_new (GST_TYPE_AUDIO_VISUAL, "name", name, "port", port,
	  "handle", (gulong) GDK_WINDOW_XID (xview),
	  "active", (audio_port == port), NULL));
  g_object_set_data (G_OBJECT (view), "audio-visual", visual);
  gst_worker_prepare (GST_WORKER (visual));
  gst_worker_start (GST_WORKER (visual));
  g_free (name);
  return visual;
}

static void
gst_switch_ui_remove_preview (GstSwitchUI *ui, GstWorker *worker,
    const gchar *name)
{
  GList *v = gtk_container_get_children (GTK_CONTAINER (ui->preview_box));
  for (; v; v = g_list_next (v)) {
    GtkWidget *view = GTK_WIDGET (v->data);
    gpointer data = g_object_get_data (G_OBJECT (view), name);
    if (data) {
      GstWorker *w = GST_WORKER (data);
      if (w == worker) {
	gtk_widget_destroy (view);
      }
    }
  }
}

static void
gst_switch_ui_end_video_disp (GstWorker *worker, GstSwitchUI *ui)
{
  GstVideoDisp *disp = GST_VIDEO_DISP (worker);
  INFO ("video ended: %s, %d", worker->name, disp->port);
  gst_switch_ui_remove_preview (ui, worker, "video-display");
}

static void
gst_switch_ui_end_audio_visual (GstWorker *worker, GstSwitchUI *ui)
{
  GstAudioVisual *visual = GST_AUDIO_VISUAL (worker);
  INFO ("audio ended: %s, %d", worker->name, visual->port);
  gst_switch_ui_remove_preview (ui, worker, "audio-visual");
}

static GstAudioPlay *
gst_switch_ui_new_audio (GstSwitchUI *ui, gint port)
{
  gchar *name = g_strdup_printf ("audio-%d", port);
  GstAudioPlay *audio = GST_AUDIO_PLAY (
      g_object_new (GST_TYPE_AUDIO_PLAY, "name", name, "port", port, NULL));
  gst_worker_prepare (GST_WORKER (audio));
  gst_worker_start (GST_WORKER (audio));
  g_free (name);
  return audio;
}

static void
gst_switch_ui_set_compose_port (GstSwitchUI *ui, gint port)
{
  if (ui->compose_port == port)
    return;

  if (ui->compose_video)
    g_object_unref (ui->compose_video);

  ui->compose_port = port;
  ui->compose_video = gst_switch_ui_new_video_disp (ui, ui->compose_view,
      port);
}

static void
gst_switch_ui_set_audio_port (GstSwitchUI *ui, gint port)
{
  GST_SWITCH_UI_LOCK_AUDIO (ui);
  if (ui->audio)
    g_object_unref (ui->audio);

  INFO ("active audio: %d", port);

  ui->audio = gst_switch_ui_new_audio (ui, port);
  GST_SWITCH_UI_UNLOCK_AUDIO (ui);
}

static void
gst_switch_ui_add_preview_port (GstSwitchUI *ui, gint port, gint type)
{
  GstVideoDisp *disp = NULL;
  GstAudioVisual *visu = NULL;
  GtkWidget *preview = gtk_drawing_area_new ();
  gtk_widget_set_double_buffered (preview, FALSE);
  gtk_widget_set_size_request (preview, -1, 80);
  gtk_widget_show (preview);
  gtk_container_add (GTK_CONTAINER (ui->preview_box), preview);

  switch (type) {
  case GST_SERVE_VIDEO_STREAM:
    disp = gst_switch_ui_new_video_disp (ui, preview, port);
    g_signal_connect (G_OBJECT(disp), "end-worker",
	G_CALLBACK (gst_switch_ui_end_video_disp), ui);
    break;
  case GST_SERVE_AUDIO_STREAM:
    visu = gst_switch_ui_new_audio_visual (ui, preview, port);
    g_signal_connect (G_OBJECT(visu), "end-worker",
	G_CALLBACK (gst_switch_ui_end_audio_visual), ui);
    break;
  default:
    gtk_widget_destroy (preview);
    break;
  }

  (void) disp;
  (void) visu;
}

static void
gst_switch_ui_class_init (GstSwitchUIClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstSwitchClientClass * client_class = GST_SWITCH_CLIENT_CLASS (klass);

  object_class->finalize = (GObjectFinalizeFunc) gst_switch_ui_finalize;

  client_class->connection_closed = (GstSwitchClientConnectionClosedFunc)
    gst_switch_ui_on_controller_closed;
  client_class->set_compose_port = (GstSwitchClientSetComposePortFunc)
    gst_switch_ui_set_compose_port;
  client_class->set_audio_port = (GstSwitchClientSetAudioPortFunc)
    gst_switch_ui_set_audio_port;
  client_class->add_preview_port = (GstSwitchClientAddPreviewPortFunc)
    gst_switch_ui_add_preview_port;
}

int
main (int argc, char *argv[])
{
  GstSwitchUI *ui;

  gst_switch_ui_parse_args (&argc, &argv);
  gtk_init (&argc, &argv);

  ui = GST_SWITCH_UI (g_object_new (GST_TYPE_SWITCH_UI, NULL));

  gst_switch_ui_run (ui);

  g_object_unref (G_OBJECT (ui));
  return 0;
}
