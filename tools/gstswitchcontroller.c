/* GstSwitchSrv
 * Copyright (C) 2013 Duzy Chan <code@duzy.info>
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

#include "gstswitchcontroller.h"

enum
{
  PROP_0,
};

G_DEFINE_TYPE (GstSwitchController, gst_switch_controller, G_TYPE_OBJECT);

static GDBusNodeInfo *introspection_data = NULL;
static const gchar introspection_xml[] =
  "<node>"
  "  <interface name='"SWITCH_CONTROLLER_OBJECT_NAME"'>"
  "    <method name='test'>"
  "      <arg type='i' name='num' direction='in'/>"
  "    </method>"
  "  </interface>"
  "</node>";
/*
  "    <property type='s' name='Name' access='readwrite'/>"
  "    <property type='i' name='Num' access='read'/>"
*/

static void
gst_switch_controller_do_method_call (
    GDBusConnection       *connection,
    const gchar           *sender,
    const gchar           *object_path,
    const gchar           *interface_name,
    const gchar           *method_name,
    GVariant              *parameters,
    GDBusMethodInvocation *invocation,
    gpointer               user_data)
{
  GstSwitchController *controller = GST_SWITCH_CONTROLLER (user_data);

  INFO ("call: %s", method_name);

  if (g_strcmp0 (method_name, "test") == 0) {
    gint num = 0;
    g_variant_get (parameters, "(i)", &num);
    num *= 2; // simple test function
    g_dbus_method_invocation_return_value (invocation,
	g_variant_new ("(i)", num));
  }
}

static GVariant *
gst_switch_controller_do_get_property (
    GDBusConnection  *connection,
    const gchar      *sender,
    const gchar      *object_path,
    const gchar      *interface_name,
    const gchar      *property_name,
    GError          **error,
    gpointer          user_data)
{
  GstSwitchController *controller = GST_SWITCH_CONTROLLER (user_data);
  GVariant *ret = NULL;

  INFO ("get: %s", property_name);

  if (g_strcmp0 (property_name, "num") == 0) {
  }

  return ret;
}

static gboolean
gst_switch_controller_do_set_property (
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

static const GDBusInterfaceVTable gst_switch_controller_interface_vtable = {
  gst_switch_controller_do_method_call,
  gst_switch_controller_do_get_property,
  gst_switch_controller_do_set_property
};

#if 0
static void
gst_switch_controller_send_property_change (GstSwitchController *controller,
    GParamSpec *pspec, GDBusConnection *connection)
{
  INFO ();
}

static void
gst_switch_controller_bus_acquired (GDBusConnection *connection,
    const gchar *name, gpointer data)
{
  GstSwitchController * controller = GST_SWITCH_CONTROLLER (data);
  guint register_id;

  INFO ("bus acquired: %s", name);

  g_signal_connect (controller, "notify",
      G_CALLBACK (gst_switch_controller_send_property_change), connection);

  register_id = g_dbus_connection_register_object (connection,
      SWITCH_CONTROLLER_OBJECT_PATH,
      introspection_data->interfaces[0],
      &gst_switch_controller_interface_vtable,
      controller, /* user_data */
      NULL, /* user_data_free_func */
      NULL /* GError** */);

  g_assert (0 < register_id);
}

static void
gst_switch_controller_name_acquired (GDBusConnection *connection,
    const gchar *name, gpointer data)
{
  INFO ("name acquired: %s", name);
}

static void
gst_switch_controller_name_lost (GDBusConnection *connection,
    const gchar *name, gpointer data)
{
  INFO ("name lost: %s", name);
}

static void
gst_switch_controller_export (GstSwitchController * controller)
{
  introspection_data = g_dbus_node_info_new_for_xml (introspection_xml, NULL);
  g_assert (introspection_data != NULL);

  controller->owner_id = g_bus_own_name (G_BUS_TYPE_SESSION,
      SWITCH_CONTROLLER_OBJECT_NAME,
      G_BUS_NAME_OWNER_FLAGS_NONE,
      gst_switch_controller_bus_acquired,
      gst_switch_controller_name_acquired,
      gst_switch_controller_name_lost,
      controller, NULL);
  
  INFO ("TODO: export");
}
#endif

static void
gst_switch_controller_on_new_connection (GDBusServer *server,
    GDBusConnection *connection, gpointer user_data)
{
  GstSwitchController *controller = GST_SWITCH_CONTROLLER (user_data);
  GCredentials *credentials = g_dbus_connection_get_peer_credentials (
      connection);
  gchar *credentials_str = g_credentials_to_string (credentials);
  guint register_id;

  register_id = g_dbus_connection_register_object (connection,
      SWITCH_CONTROLLER_OBJECT_PATH,
      introspection_data->interfaces[0],
      &gst_switch_controller_interface_vtable,
      controller, /* user_data */
      NULL, /* user_data_free_func */
      NULL /* GError** */);

  g_assert (0 < register_id);

  //g_object_unref (connection);

  INFO ("new connection: %p, %d, %s (%s, %s)", connection, register_id,
      g_dbus_connection_get_unique_name (connection),
      g_dbus_connection_get_guid (connection), credentials_str);

  g_free (credentials_str);
}

static void
gst_switch_controller_init (GstSwitchController * controller)
{
  gchar * guid = g_dbus_generate_guid ();
  GDBusServerFlags flags = G_DBUS_SERVER_FLAGS_NONE;
  GError * error = NULL;

  /*
  flags |= G_DBUS_SERVER_FLAGS_AUTHENTICATION_ALLOW_ANONYMOUS;
  */

  controller->server = g_dbus_server_new_sync (
      SWITCH_CONTROLLER_ADDRESS,
      flags, guid,
      NULL, /* GDBusAuthObserver */
      NULL, /* GCancellable */
      &error);

  g_dbus_server_start (controller->server);
  g_free (guid);

  if (controller->server == NULL)
    goto error_new_server;

  INFO ("Switch Controller is listening at: %s",
      g_dbus_server_get_client_address (controller->server));

  introspection_data = g_dbus_node_info_new_for_xml (introspection_xml, NULL);
  g_assert (introspection_data != NULL);

  g_signal_connect (controller->server, "new-connection",
      G_CALLBACK (gst_switch_controller_on_new_connection),
      controller);

  // TODO: singleton object

  //controller->owner_id = 0;
  //gst_switch_controller_export (controller);

  return;

  /* Errors Handling */
 error_new_server:
  {
    ERROR ("%s", error->message);
    return;
  }
}

static void
gst_switch_controller_finalize (GstSwitchController * controller)
{
  /*
  g_bus_unown_name (controller->owner_id);
  controller->owner_id = 0;
  */

  if (introspection_data) {
    g_dbus_node_info_unref (introspection_data);
    introspection_data = NULL;
  }

  if (G_OBJECT_CLASS (gst_switch_controller_parent_class)->finalize)
    (*G_OBJECT_CLASS (gst_switch_controller_parent_class)->finalize)
      (G_OBJECT (controller));

  INFO ("Controller finalized");
}

static void
gst_switch_controller_get_property(GstSwitchController * controller,
    guint prop_id, GValue *value, GParamSpec *pspec)
{
  switch (prop_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (controller), prop_id, pspec);
    break;
  }
}

static void
gst_switch_controller_set_property(GstSwitchController * controller,
    guint prop_id, const GValue *value, GParamSpec *pspec)
{
  switch (prop_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (controller), prop_id, pspec);
    break;
  }
}

static void
gst_switch_controller_class_init (GstSwitchControllerClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = (GObjectFinalizeFunc) gst_switch_controller_finalize;
  object_class->get_property = (GObjectGetPropertyFunc) gst_switch_controller_get_property;
  object_class->set_property = (GObjectSetPropertyFunc) gst_switch_controller_set_property;
}

