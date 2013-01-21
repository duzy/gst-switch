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
#define GST_SWITCH_UI_LOCK_COMPOSE(ui) (g_mutex_lock (&(ui)->compose_lock))
#define GST_SWITCH_UI_UNLOCK_COMPOSE(ui) (g_mutex_unlock (&(ui)->compose_lock))
#define GST_SWITCH_UI_LOCK_SELECT(ui) (g_mutex_lock (&(ui)->select_lock))
#define GST_SWITCH_UI_UNLOCK_SELECT(ui) (g_mutex_unlock (&(ui)->select_lock))

G_DEFINE_TYPE (GstSwitchUI, gst_switch_ui, GST_TYPE_SWITCH_CLIENT);

gboolean verbose;

static GOptionEntry entries[] = {
  {"verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Be verbose", NULL},
  {NULL}
};

static const gchar * gst_switch_ui_css = 
  ".preview_frame {\n"
  "  border-style: solid;\n"
  "  border-width: 5px;\n"
  "  border-radius: 5px;\n"
  "  border-color: rgba(0,0,0,0.2);\n"
  "  padding: 0px;\n"
  "}\n"
  ".audio_frame {\n"
  "  border-style: solid;\n"
  "  border-width: 5px;\n"
  "  border-color: #F00;\n"
  "}\n"
  ".preview_frame:selected {\n"
  "  border-style: solid;\n"
  "  border-width: 5px;\n"
  "  border-color: #0F0;\n"
  "}\n"
  ;

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

/*
static gboolean
draw_cb (GtkWidget *widget, cairo_t *cr)
{
  GtkStyleContext *style;
  style = gtk_widget_get_style_context (widget);
  gtk_style_context_save (style);
  //gtk_style_context_add_class (style, "red");
  gtk_render_frame (style, cr, 0, 0, 100, 100);
  //gtk_style_context_remove_class (style, "red");
  gtk_style_context_restore (style);
  return TRUE;
}
*/

static gboolean gst_switch_ui_key_event (GtkWidget *w, GdkEvent *event,
    GstSwitchUI *ui);

static void
gst_switch_ui_init (GstSwitchUI * ui)
{
  GtkWidget *main_box;
  GtkWidget *scrollwin;
  GdkDisplay *display;
  GdkScreen *screen;
  GError *error = NULL;

  g_mutex_init (&ui->audio_lock);
  g_mutex_init (&ui->compose_lock);
  g_mutex_init (&ui->select_lock);

  display = gdk_display_get_default ();
  screen = gdk_display_get_default_screen (display);

  ui->css = gtk_css_provider_new ();
  gtk_style_context_add_provider_for_screen (screen,
      GTK_STYLE_PROVIDER (ui->css),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  ui->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (ui->window), 640, 480);
  gtk_window_set_title (GTK_WINDOW (ui->window), "GstSwitch");

  g_signal_connect (G_OBJECT (ui->window), "key-press-event",
      G_CALLBACK (gst_switch_ui_key_event), ui);

  main_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
  ui->preview_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
  gtk_widget_set_name (ui->preview_box, "previews");
  gtk_widget_set_size_request (ui->preview_box, 120, -1);
  gtk_widget_set_vexpand (ui->preview_box, TRUE);

  ui->compose_view = gtk_drawing_area_new ();
  gtk_widget_set_name (ui->compose_view, "compose");
  gtk_widget_set_double_buffered (ui->compose_view, FALSE);
  gtk_widget_set_hexpand (ui->compose_view, TRUE);
  gtk_widget_set_vexpand (ui->compose_view, TRUE);
  gtk_widget_set_events (ui->compose_view, GDK_EXPOSURE_MASK
      //| GDK_LEAVE_NOTIFY_MASK
      | GDK_BUTTON_PRESS_MASK
      | GDK_BUTTON_RELEASE_MASK
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

  gtk_css_provider_load_from_data (ui->css, gst_switch_ui_css, -1, &error);
  g_assert_no_error (error);
}

static void
gst_switch_ui_finalize (GstSwitchUI * ui)
{
  gtk_widget_destroy (GTK_WIDGET (ui->window));
  ui->window = NULL;

  g_object_unref (ui->css);

  g_mutex_clear (&ui->audio_lock);
  g_mutex_clear (&ui->compose_lock);
  g_mutex_clear (&ui->select_lock);

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
  gboolean active = FALSE;
  gint audio_port = 0;

  if (ui->audio) {
    GST_SWITCH_UI_LOCK_AUDIO (ui);
    if (ui->audio) audio_port = ui->audio->port;
    GST_SWITCH_UI_UNLOCK_AUDIO (ui);

    active = (audio_port == port);
  }

  visual = GST_AUDIO_VISUAL (
      g_object_new (GST_TYPE_AUDIO_VISUAL, "name", name, "port", port,
	  "handle", (gulong) GDK_WINDOW_XID (xview), "active", active, NULL));
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
  GList *v = NULL;
  GST_SWITCH_UI_LOCK_SELECT (ui);
  v = gtk_container_get_children (GTK_CONTAINER (ui->preview_box));
  for (; v; v = g_list_next (v)) {
    GtkWidget *frame = GTK_WIDGET (v->data);
    //GList *child = gtk_container_get_children (GTK_CONTAINER (frame));
    //GtkWidget *view = GTK_WIDGET (child->data);
    //gpointer data = g_object_get_data (G_OBJECT (view), name);
    gpointer data = g_object_get_data (G_OBJECT (frame), name);
    if (data) {
      if (GST_WORKER (data) == worker) {
	if (ui->selected == frame) {
	  GList *nxt = g_list_next (v);
	  if (nxt == NULL) nxt = g_list_previous (v);
	  ui->selected = nxt ? GTK_WIDGET (nxt->data) : NULL;
	}
	//gtk_widget_destroy (view);
	gtk_widget_destroy (frame);
	g_object_unref (worker);
      }
    }
  }
  GST_SWITCH_UI_UNLOCK_SELECT (ui);
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

static void
gst_switch_ui_end_audio (GstAudioPlay *audio, GstSwitchUI *ui)
{
  if (audio != ui->audio) {
    INFO ("audio %d ended", audio->port);
    g_object_unref (audio);
  } else if (ui->audio) {
    /*
    GList *v = gtk_container_get_children (GTK_CONTAINER (ui->preview_box));
    gint audio_port = 0;
    */
    GST_SWITCH_UI_LOCK_AUDIO (ui);
    //audio_port = ui->audio->port;
    g_object_unref (ui->audio);
    ui->audio = NULL;
    /*
    for (; v; v = g_list_next (v)) {
      style = gtk_widget_get_style_context (GTK_WIDGET (v->data));
      gtk_style_context_remove_class (style, "audio_frame");
    }
    */
    GST_SWITCH_UI_UNLOCK_AUDIO (ui);
  }
}

static GstAudioPlay *
gst_switch_ui_new_audio (GstSwitchUI *ui, gint port)
{
  gchar *name = g_strdup_printf ("audio-%d", port);
  GstAudioPlay *audio = GST_AUDIO_PLAY (
      g_object_new (GST_TYPE_AUDIO_PLAY, "name", name, "port", port, NULL));
  g_signal_connect (G_OBJECT (audio), "end-worker",
      G_CALLBACK (gst_switch_ui_end_audio), ui);
  gst_worker_prepare (GST_WORKER (audio));
  gst_worker_start (GST_WORKER (audio));
  g_free (name);
  return audio;
}

static void
gst_switch_ui_set_compose_port (GstSwitchUI *ui, gint port)
{
  GST_SWITCH_UI_LOCK_COMPOSE (ui);
  if (ui->compose && ui->compose->port == port) {
    INFO ("compose %d already displayed", port);
  } else {
    if (ui->compose)
      g_object_unref (ui->compose);

    ui->compose = gst_switch_ui_new_video_disp (ui,
	ui->compose_view, port);
  }
  GST_SWITCH_UI_UNLOCK_COMPOSE (ui);
}

static void
gst_switch_ui_set_audio_port (GstSwitchUI *ui, gint port)
{
  if (!port) {
    ERROR ("Invalid audio port");
    return;
  }

  GST_SWITCH_UI_LOCK_AUDIO (ui);
  if (ui->audio) {
    gst_worker_stop (GST_WORKER (ui->audio));
  }

  INFO ("active audio: %d", port);

  ui->audio = gst_switch_ui_new_audio (ui, port);
  GST_SWITCH_UI_UNLOCK_AUDIO (ui);
}

static gboolean
gst_switch_ui_preview_click (GtkWidget *w, GdkEvent *event, GstSwitchUI *ui)
{
  INFO ("event: %d", event->type);
  switch (event->type) {
  case GDK_BUTTON_RELEASE: {
    
  } break;
  default: break;
  }
  return TRUE;
}

static void
gst_switch_ui_add_preview_port (GstSwitchUI *ui, gint port, gint type)
{
  GstVideoDisp *disp = NULL;
  GstAudioVisual *visual = NULL;
  GtkStyleContext *style = NULL;
  GtkWidget *frame = gtk_frame_new (NULL);
  GtkWidget *preview = gtk_drawing_area_new ();
  gtk_widget_set_double_buffered (preview, FALSE);
  gtk_widget_set_size_request (preview, -1, 80);
  gtk_container_add (GTK_CONTAINER (frame), preview);
  gtk_container_add (GTK_CONTAINER (ui->preview_box), frame);
  gtk_widget_show_all (frame);

  style = gtk_widget_get_style_context (frame);
  gtk_style_context_add_class (style, "preview_frame");
  //gtk_widget_set_state_flags (frame, GTK_STATE_FLAG_SELECTED, TRUE);

  style = gtk_widget_get_style_context (preview);
  gtk_style_context_add_class (style, "preview_drawing_area");

  gtk_widget_set_events (preview, GDK_BUTTON_RELEASE_MASK);
  g_signal_connect (preview, "button-release-event",
      G_CALLBACK (gst_switch_ui_preview_click), ui);

  switch (type) {
  case GST_SERVE_VIDEO_STREAM:
    disp = gst_switch_ui_new_video_disp (ui, preview, port);
    g_object_set_data (G_OBJECT (frame), "video-display", disp);
    g_signal_connect (G_OBJECT(disp), "end-worker",
	G_CALLBACK (gst_switch_ui_end_video_disp), ui);
    break;
  case GST_SERVE_AUDIO_STREAM:
    visual = gst_switch_ui_new_audio_visual (ui, preview, port);
    g_object_set_data (G_OBJECT (frame), "audio-visual", visual);
    g_signal_connect (G_OBJECT(visual), "end-worker",
	G_CALLBACK (gst_switch_ui_end_audio_visual), ui);
    if (visual->active) {
      style = gtk_widget_get_style_context (frame);
      gtk_style_context_add_class (style, "audio_frame");
    }
    break;
  default:
    gtk_widget_destroy (preview);
    gtk_widget_destroy (frame);
    break;
  }
}

static void
gst_switch_ui_select_preview (GstSwitchUI *ui, guint key)
{
  GList *view = NULL, *selected = NULL;
  GtkWidget *previous = NULL;
  GST_SWITCH_UI_LOCK_SELECT (ui);
  view = gtk_container_get_children (GTK_CONTAINER (ui->preview_box));
  if (ui->selected == NULL) {
    if (view) switch (key) {
    case GDK_KEY_Up:
      selected = g_list_last (view);
      break;
    case GDK_KEY_Down:
      selected = view;
      break;
    }
  } else {
    for (; view; view = g_list_next (view)) {
      if (GTK_WIDGET (view->data) == ui->selected) {
	selected = view;
	break;
      }
    }
    previous = GTK_WIDGET (selected->data);
    switch (key) {
    case GDK_KEY_Up:
      selected = g_list_previous (selected);
      break;
    case GDK_KEY_Down:
      selected = g_list_next (selected);
      break;
    }
  }

  if (selected) {
    ui->selected = GTK_WIDGET (selected->data);
  }

  if (ui->selected) {
    if (previous) gtk_widget_unset_state_flags (previous,
	GTK_STATE_FLAG_SELECTED);
    gtk_widget_set_state_flags (ui->selected, GTK_STATE_FLAG_SELECTED, TRUE);
  }
  GST_SWITCH_UI_UNLOCK_SELECT (ui);
}

static void
gst_switch_ui_switch (GstSwitchUI *ui, gint key)
{
  gint port;
  gpointer data;
  gboolean ok = FALSE;
  GST_SWITCH_UI_LOCK_SELECT (ui);
  if (!ui->selected) {
    goto end;
  }

  data = g_object_get_data (G_OBJECT (ui->selected), "video-display");
  if (data) {
    port = GST_VIDEO_DISP (data)->port;
    switch (key) {
    case GDK_KEY_A:
    case GDK_KEY_a:
      ok = gst_switch_client_switch (GST_SWITCH_CLIENT (ui), 'A', port);
      INFO ("switch-a: %d, %d", port, ok);
      break;
    case GDK_KEY_B:
    case GDK_KEY_b:
      ok = gst_switch_client_switch (GST_SWITCH_CLIENT (ui), 'B', port);
      INFO ("switch-b: %d, %d", port, ok);
      break;
    }
    goto end;
  }

  data = g_object_get_data (G_OBJECT (ui->selected), "audio-visual");
  if (data) {
    port = GST_VIDEO_DISP (data)->port;
    switch (key) {
    case GDK_KEY_A:
    case GDK_KEY_a:
      ok = gst_switch_client_switch (GST_SWITCH_CLIENT (ui), 'a', port);
      INFO ("switch-audio: %d, %d", port, ok);
      break;
    }
    goto end;
  }

 end:
  GST_SWITCH_UI_UNLOCK_SELECT (ui);
}

static void
gst_switch_ui_next_compose_mode (GstSwitchUI *ui)
{
  gboolean ok = FALSE;
  ui->compose_mode += 1;
  if (3 < ui->compose_mode) {
    ui->compose_mode = 1;
  }
  ok = gst_switch_client_set_composite_mode (GST_SWITCH_CLIENT (ui),
      ui->compose_mode);
  INFO ("set composite mode: %d (%d)", ui->compose_mode, ok);
}

static gboolean
gst_switch_ui_key_event (GtkWidget *w, GdkEvent *event, GstSwitchUI *ui)
{
  switch (event->type) {
  case GDK_KEY_PRESS:
  case GDK_KEY_RELEASE: {
    GdkEventKey *ke = (GdkEventKey *) event;
    switch (ke->keyval) {
    case GDK_KEY_Up:
    case GDK_KEY_Down:
      gst_switch_ui_select_preview (ui, ke->keyval);
      break;
    case GDK_KEY_A:
    case GDK_KEY_a:
    case GDK_KEY_B:
    case GDK_KEY_b:
      gst_switch_ui_switch (ui, ke->keyval);
      break;
    case GDK_KEY_Tab:
      gst_switch_ui_next_compose_mode (ui);
      break;
    }
  } break;
  default: break;
  }
  return FALSE;
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
