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
#include "gstswitchcontroller.h"

G_DEFINE_TYPE (GstSwitchUI, gst_switchui, G_TYPE_OBJECT);

static GDBusNodeInfo *introspection_data = NULL;
static const gchar introspection_xml[] =
  "<node>"
  "  <interface name='" SWITCH_UI_OBJECT_NAME "'>"
  "    <method name='test'>"
  "      <arg type='s' name='name' direction='in'/>"
  "      <arg type='s' name='result' direction='out'/>"
  "    </method>"
  "  </interface>"
  "</node>";

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
  (void) ui;
  return FALSE;
}

static gboolean
gst_switchui_compose_view_motion (GtkWidget * widget, GdkEventMotion * event,
    gpointer data)
{
  GstSwitchUI * ui = GST_SWITCH_UI (data);
  (void) ui;
  return FALSE;
}

static gboolean
gst_switchui_compose_view_press (GtkWidget * widget, GdkEventButton * event,
    gpointer data)
{
  GstSwitchUI * ui = GST_SWITCH_UI (data);
  (void) ui;
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

static GVariant *
gst_switchui_call_controller (GstSwitchUI * ui, const gchar *method_name,
    GVariant *parameters, const GVariantType *reply_type)
{
  GVariant *value = NULL;
  GError *error = NULL;

  if (!ui->controller)
    goto error_no_controller_connection;

  INFO ("calling: %s/%s", SWITCH_CONTROLLER_OBJECT_NAME, method_name);

  value = g_dbus_connection_call_sync (ui->controller, NULL, /* bus_name */
      SWITCH_CONTROLLER_OBJECT_PATH,
      SWITCH_CONTROLLER_OBJECT_NAME,
      method_name, parameters, reply_type,
      G_DBUS_CALL_FLAGS_NONE,
      5000, /* timeout_msec */
      NULL /* TODO: cancellable */,
      &error);

  if (!value)
    goto error_call_sync;

  return value;

  /* ERRORS */
 error_no_controller_connection:
  {
    ERROR ("No controller connection");
    return NULL;
  }

 error_call_sync:
  {
    ERROR ("%s", error->message);
    g_error_free (error);
    return NULL;
  }
}

static gchar *
gst_switchui_controller_test (GstSwitchUI * ui, const gchar *s)
{
  gchar *result = NULL;
  GVariant *value = gst_switchui_call_controller (ui, "test",
      g_variant_new ("(s)", s), G_VARIANT_TYPE ("(s)"));

  if (value) {
    g_variant_get (value, "(&s)", &result);
    if (result) result = g_strdup (result);
    g_variant_unref (value);
  }

  return result;
}

static gboolean
gst_switch_ui_method_match (const gchar *key,
    MethodTableEntry *entry, const gchar *match)
{
  if (g_strcmp0 (key, match) == 0)
    return TRUE;
  return FALSE;
}

static void
gst_switch_ui_do_method_call (
    GDBusConnection       *connection,
    const gchar           *sender,
    const gchar           *object_path,
    const gchar           *interface_name,
    const gchar           *method_name,
    GVariant              *parameters,
    GDBusMethodInvocation *invocation,
    gpointer               user_data)
{
  GstSwitchUI *ui = GST_SWITCH_UI (user_data);
  GstSwitchUIClass *klass = GST_SWITCH_UI_CLASS (G_OBJECT_GET_CLASS (ui));
  MethodFunc entry = (MethodFunc) g_hash_table_find (klass->methods,
      (GHRFunc) gst_switch_ui_method_match, (gpointer) method_name);
  GVariant *results;

  if (!entry)
    goto error_no_method;

  /*
  INFO ("calling: %s/%s", interface_name, method_name);
  */

  results = (*entry) (G_OBJECT (ui), parameters);
  g_dbus_method_invocation_return_value (invocation, results);
  return;

 error_no_method:
  {
    ERROR ("unsupported method: %s", method_name);
    g_dbus_method_invocation_return_error (invocation, 0, -1,
	"Unsupported call %s", method_name);
  }
}

static GVariant *
gst_switch_ui_do_get_property (
    GDBusConnection  *connection,
    const gchar      *sender,
    const gchar      *object_path,
    const gchar      *interface_name,
    const gchar      *property_name,
    GError          **error,
    gpointer          user_data)
{
  GVariant *ret = NULL;
  INFO ("get: %s", property_name);
  return ret;
}

static gboolean
gst_switch_ui_do_set_property (
    GDBusConnection  *connection,
    const gchar      *sender,
    const gchar      *object_path,
    const gchar      *interface_name,
    const gchar      *property_name,
    GVariant         *value,
    GError          **error,
    gpointer          user_data)
{
  INFO ("set: %s", property_name);
  return FALSE;
}

static const GDBusInterfaceVTable gst_switch_ui_interface_vtable = {
  gst_switch_ui_do_method_call,
  gst_switch_ui_do_get_property,
  gst_switch_ui_do_set_property
};

static void
gst_switchui_connect_controller (GstSwitchUI * ui)
{
  GError *error = NULL;
  gchar *test_result = NULL;
  guint register_id;

  ui->controller = g_dbus_connection_new_for_address_sync (
      SWITCH_CONTROLLER_ADDRESS,
      G_DBUS_SERVER_FLAGS_RUN_IN_THREAD |
      G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT,
      NULL, /* GDBusAuthObserver */
      NULL, /* GCancellable */
      &error);

  if (ui->controller == NULL)
    goto error_new_connection;

  /* Register Object */
  register_id = g_dbus_connection_register_object (ui->controller,
      SWITCH_UI_OBJECT_PATH,
      introspection_data->interfaces[0],
      &gst_switch_ui_interface_vtable,
      ui, /* user_data */
      NULL, /* user_data_free_func */
      NULL /* GError** */);

  g_assert (0 < register_id);

  test_result = gst_switchui_controller_test (ui, "hello, gst-switch");
  if (test_result) {
    INFO ("test: %s\n", test_result);
    g_free (test_result);
  }
  return;

 error_new_connection:
  {
    ERROR ("%s", error->message);
    g_error_free (error);
    return;
  }
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
  gst_switchui_connect_controller (ui);

  gtk_widget_show_all (ui->window);
  gtk_widget_realize (ui->window);
  gst_switchui_prepare_connections (ui);
  gtk_main ();
}

static GVariant *
gst_switch_ui__test (GstSwitchUI *ui, GVariant *parameters)
{
  gchar *s = NULL;
  g_variant_get (parameters, "(&s)", &s);
  INFO ("test: %s", s);
  return g_variant_new ("(s)", "hello, controller");
}

static MethodTableEntry gst_switch_ui_method_table[] = {
  { "test", (MethodFunc) gst_switch_ui__test },
  { NULL, NULL }
};

static void
gst_switchui_class_init (GstSwitchUIClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = (GObjectFinalizeFunc) gst_switchui_finalize;

  klass->methods = g_hash_table_new (g_str_hash, g_str_equal);

  MethodTableEntry *entry = &gst_switch_ui_method_table[0];
  for (; entry->name && entry->func; ++entry) {
    g_hash_table_insert (klass->methods, (gpointer) entry->name,
	(gpointer) entry->func);
  }

  introspection_data = g_dbus_node_info_new_for_xml (introspection_xml, NULL);
  g_assert (introspection_data != NULL);
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
