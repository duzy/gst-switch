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
  "  <interface name='" SWITCH_CONTROLLER_OBJECT_NAME "'>"
  "    <method name='test'>"
  "      <arg type='s' name='name' direction='in'/>"
  "      <arg type='s' name='result' direction='out'/>"
  "    </method>"
  "  </interface>"
  "</node>";
/*
  "    <property type='s' name='Name' access='readwrite'/>"
  "    <property type='i' name='Num' access='read'/>"
*/

static gboolean
gst_switch_controller_method_match (const gchar *key,
    MethodTableEntry *entry, const gchar *match)
{
  if (g_strcmp0 (key, match) == 0)
    return TRUE;
  return FALSE;
}

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
  GstSwitchControllerClass *klass = GST_SWITCH_CONTROLLER_CLASS (
      G_OBJECT_GET_CLASS (controller));
  MethodFunc entry = (MethodFunc) g_hash_table_find (klass->methods,
      (GHRFunc) gst_switch_controller_method_match, (gpointer) method_name);
  GVariant *results;

  if (!entry)
    goto error_no_method;

  /*
  INFO ("calling: %s/%s", interface_name, method_name);
  */

  results = (*entry) (G_OBJECT (controller), parameters);
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

  (void) controller;

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

static GVariant *
gst_switch_controller_call_ui (GstSwitchController * controller,
    GDBusConnection *connection,
    const gchar *method_name, GVariant *parameters,
    const GVariantType *reply_type);

static void
gst_switch_controller_on_new_connection (GDBusServer *server,
    GDBusConnection *connection, gpointer user_data)
{
  GstSwitchController *controller = GST_SWITCH_CONTROLLER (user_data);
  guint register_id = 0;
  GError *error = NULL;

  g_object_ref (connection);

  register_id = g_dbus_connection_register_object (connection,
      SWITCH_CONTROLLER_OBJECT_PATH,
      introspection_data->interfaces[0],
      &gst_switch_controller_interface_vtable,
      controller, /* user_data */
      NULL, /* user_data_free_func */
      &error);

  if (register_id <= 0) {
    ERROR ("register: %s", error->message);
  }

  g_assert (0 < register_id);

  /*
  INFO ("registered: %d, %s, %s", register_id, SWITCH_CONTROLLER_OBJECT_PATH,
      introspection_data->interfaces[0]->name);
  */

  /*
  gst_switch_controller_call_ui (controller, connection, "test",
      g_variant_new ("(s)", "hi, ui"), G_VARIANT_TYPE ("(s)"));
  */
}

static gboolean
gst_switch_controller_authorize_authenticated_peer (
    GDBusAuthObserver *observer,
    GIOStream         *stream,
    GCredentials      *credentials,
    gpointer           user_data)
{
  INFO ("authorize: %s", g_credentials_to_string (credentials));
  return TRUE;  
}

static void
gst_switch_controller_init (GstSwitchController * controller)
{
  gchar * guid = g_dbus_generate_guid ();
  GDBusServerFlags flags = G_DBUS_SERVER_FLAGS_NONE;
  GDBusAuthObserver *auth_observer;
  GError * error = NULL;

  flags |= G_DBUS_SERVER_FLAGS_RUN_IN_THREAD;
  flags |= G_DBUS_SERVER_FLAGS_AUTHENTICATION_ALLOW_ANONYMOUS;

  auth_observer = g_dbus_auth_observer_new ();
  controller->server = g_dbus_server_new_sync (
      SWITCH_CONTROLLER_ADDRESS,
      flags, guid,
      auth_observer,
      NULL, /* GCancellable */
      &error);

  g_assert_no_error (error);
  g_object_unref (auth_observer);
  g_free (guid);

  if (controller->server == NULL)
    goto error_new_server;

  INFO ("Switch Controller is listening at: %s",
      g_dbus_server_get_client_address (controller->server));

  g_signal_connect (controller->server, "new-connection",
      G_CALLBACK (gst_switch_controller_on_new_connection), controller);

  g_signal_connect (auth_observer,
      "authorize-authenticated-peer",
      G_CALLBACK (gst_switch_controller_authorize_authenticated_peer),
      controller);

  g_dbus_server_start (controller->server);

  // TODO: singleton object
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
  if (controller->server) {
    g_object_unref (controller->server);
    controller->server = NULL;
  }

  if (G_OBJECT_CLASS (gst_switch_controller_parent_class)->finalize)
    (*G_OBJECT_CLASS (gst_switch_controller_parent_class)->finalize)
      (G_OBJECT (controller));

  INFO ("Controller finalized");
}

static GVariant *
gst_switch_controller_call_ui (GstSwitchController * controller,
    GDBusConnection *connection,
    const gchar *method_name, GVariant *parameters,
    const GVariantType *reply_type)
{
  GVariant *value = NULL;
  GError *error = NULL;

  INFO ("calling: %s/%s", SWITCH_CONTROLLER_OBJECT_NAME, method_name);

  value = g_dbus_connection_call_sync (connection, NULL, /* bus_name */
      SWITCH_UI_OBJECT_PATH, SWITCH_UI_OBJECT_NAME,
      method_name, parameters, reply_type,
      G_DBUS_CALL_FLAGS_NONE,
      5000, /* timeout_msec */
      NULL /* TODO: cancellable */,
      &error);

  if (!value)
    goto error_call_sync;

  return value;

  /* ERRORS */
 error_call_sync:
  {
    ERROR ("%s", error->message);
    g_error_free (error);
    return NULL;
  }
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

static GVariant *
gst_switch_controller__test (GstSwitchController *controller,
    GVariant *parameters)
{
  gchar *s = NULL;
  g_variant_get (parameters, "(&s)", &s);
  INFO ("test: %s", s);
  return g_variant_new ("(s)", "hello, ui");
}

static MethodTableEntry gst_switch_controller_method_table[] = {
  { "test", (MethodFunc) gst_switch_controller__test },
  { NULL, NULL }
};

static void
gst_switch_controller_class_init (GstSwitchControllerClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = (GObjectFinalizeFunc) gst_switch_controller_finalize;
  object_class->get_property = (GObjectGetPropertyFunc) gst_switch_controller_get_property;
  object_class->set_property = (GObjectSetPropertyFunc) gst_switch_controller_set_property;

  klass->methods = g_hash_table_new (g_str_hash, g_str_equal);

  MethodTableEntry *entry = &gst_switch_controller_method_table[0];
  for (; entry->name && entry->func; ++entry) {
    g_hash_table_insert (klass->methods, (gpointer) entry->name,
	(gpointer) entry->func);
  }

  introspection_data = g_dbus_node_info_new_for_xml (introspection_xml, NULL);
  g_assert (introspection_data != NULL);
}
