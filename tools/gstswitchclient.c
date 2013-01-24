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
#include "gstswitchclient.h"
#include "gstswitchcontroller.h"

#define parent_class gst_switch_client_parent_class
G_DEFINE_TYPE (GstSwitchClient, gst_switch_client, G_TYPE_OBJECT);

static GDBusNodeInfo *introspection_data = NULL;
static const gchar introspection_xml[] =
  "<node>"
  "  <interface name='" SWITCH_CLIENT_OBJECT_NAME "'>"
#if ENABLE_TEST
  "    <method name='test'>"
  "      <arg type='s' name='name' direction='in'/>"
  "      <arg type='s' name='result' direction='out'/>"
  "    </method>"
#endif//ENABLE_TEST
  "    <method name='set_audio_port'>"
  "      <arg type='i' name='port' direction='in'/>"
  "    </method>"
  "    <method name='set_compose_port'>"
  "      <arg type='i' name='port' direction='in'/>"
  "    </method>"
  "    <method name='add_preview_port'>"
  "      <arg type='i' name='port' direction='in'/>"
  "      <arg type='i' name='type' direction='in'/>"
  "    </method>"
  "  </interface>"
  "</node>"
  ;

extern gboolean verbose;

static void
gst_switch_client_init (GstSwitchClient * client)
{
}

static void
gst_switch_client_finalize (GstSwitchClient * client)
{
  if (G_OBJECT_CLASS (parent_class)->finalize)
    (*G_OBJECT_CLASS (parent_class)->finalize) (G_OBJECT (client));
}

static GVariant *
gst_switch_client_call_controller (GstSwitchClient * client, const gchar *method_name,
    GVariant *parameters, const GVariantType *reply_type)
{
  GVariant *value = NULL;
  GError *error = NULL;

  if (!client->controller)
    goto error_no_controller_connection;

  //INFO ("calling: %s/%s", SWITCH_CONTROLLER_OBJECT_NAME, method_name);

  value = g_dbus_connection_call_sync (client->controller, NULL, /* bus_name */
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

#if ENABLE_TEST
static gchar *
gst_switch_client_controller_test (GstSwitchClient * client, const gchar *s)
{
  gchar *result = NULL;
  GVariant *value = gst_switch_client_call_controller (client, "test",
      g_variant_new ("(s)", s), G_VARIANT_TYPE ("(s)"));

  if (value) {
    g_variant_get (value, "(&s)", &result);
    if (result) result = g_strdup (result);
    g_variant_unref (value);
  }

  return result;
}
#endif//ENABLE_TEST

gint
gst_switch_client_get_compose_port (GstSwitchClient * client)
{
  gint port = 0;
  GVariant *value = gst_switch_client_call_controller (client, "get_compose_port",
      NULL, G_VARIANT_TYPE ("(i)"));
  if (value) {
    g_variant_get (value, "(i)", &port);
  }
  return port;
}

gint
gst_switch_client_get_encode_port (GstSwitchClient * client)
{
  gint port = 0;
  GVariant *value = gst_switch_client_call_controller (client, "get_encode_port",
      NULL, G_VARIANT_TYPE ("(i)"));
  if (value) {
    g_variant_get (value, "(i)", &port);
  }
  return port;
}

gint
gst_switch_client_get_audio_port (GstSwitchClient * client)
{
  gint port = 0;
  GVariant *value = gst_switch_client_call_controller (client, "get_audio_port",
      NULL, G_VARIANT_TYPE ("(i)"));
  if (value) {
    g_variant_get (value, "(i)", &port);
  }
  return port;
}

GVariant *
gst_switch_client_get_preview_ports (GstSwitchClient * client)
{
  return gst_switch_client_call_controller (client, "get_preview_ports",
      NULL, G_VARIANT_TYPE ("(s)"));
}

gboolean
gst_switch_client_switch (GstSwitchClient * client, gint channel, gint port)
{
  gboolean result = FALSE;
  GVariant *value = gst_switch_client_call_controller (client, "switch",
      g_variant_new ("(ii)", channel, port), G_VARIANT_TYPE ("(b)"));
  if (value) {
    g_variant_get (value, "(b)", &result);
  }
  return result;
}

gboolean
gst_switch_client_set_composite_mode (GstSwitchClient * client, gint mode)
{
  gboolean result = FALSE;
  GVariant *value = gst_switch_client_call_controller (client,
      "set_composite_mode", g_variant_new ("(i)", mode),
      G_VARIANT_TYPE ("(b)"));
  if (value) {
    g_variant_get (value, "(b)", &result);
  }
  return result;
}

gboolean
gst_switch_client_new_record (GstSwitchClient * client)
{
  gboolean result = FALSE;
  GVariant *value = gst_switch_client_call_controller (client,
      "new_record", g_variant_new ("()"), G_VARIANT_TYPE ("(b)"));
  if (value) {
    g_variant_get (value, "(b)", &result);
  }
  return result;
}

guint
gst_switch_client_adjust_pip (GstSwitchClient * client, gint dx, gint dy,
    gint dw, gint dh)
{
  guint result = 0;
  GVariant *value = gst_switch_client_call_controller (client,
      "adjust_pip", g_variant_new ("(iiii)", dx, dy, dw, dh),
      G_VARIANT_TYPE ("(u)"));
  if (value) {
    g_variant_get (value, "(u)", &result);
  }
  return result;
}

static gboolean
gst_switch_client_method_match (const gchar *key, MethodTableEntry *entry,
    const gchar *match)
{
  if (g_strcmp0 (key, match) == 0)
    return TRUE;
  return FALSE;
}

static void
gst_switch_client_do_method_call (
    GDBusConnection       *connection,
    const gchar           *sender,
    const gchar           *object_path,
    const gchar           *interface_name,
    const gchar           *method_name,
    GVariant              *parameters,
    GDBusMethodInvocation *invocation,
    gpointer               user_data)
{
  GstSwitchClient *client = GST_SWITCH_CLIENT (user_data);
  GstSwitchClientClass *klass = GST_SWITCH_CLIENT_CLASS (
      G_OBJECT_GET_CLASS (client));
  MethodFunc entry = (MethodFunc) g_hash_table_find (klass->methods,
      (GHRFunc) gst_switch_client_method_match, (gpointer) method_name);
  GVariant *results;

  if (!entry)
    goto error_no_method;

  /*
  INFO ("calling: %s/%s", interface_name, method_name);
  */

  results = (*entry) (G_OBJECT (client), connection, parameters);
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
gst_switch_client_do_get_property (
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
gst_switch_client_do_set_property (
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

static const GDBusInterfaceVTable gst_switch_client_interface_vtable = {
  gst_switch_client_do_method_call,
  gst_switch_client_do_get_property,
  gst_switch_client_do_set_property
};

static void
gst_switch_client_on_signal_received (
    GDBusConnection *connection,
    const gchar     *sender_name,
    const gchar     *object_path,
    const gchar     *interface_name,
    const gchar     *signal_name,
    GVariant        *parameters,
    gpointer         user_data)
{
  GstSwitchClient *client = GST_SWITCH_CLIENT (user_data);

  (void) client;

  INFO ("signal: %s, %s", sender_name, signal_name);
}

static void
gst_switch_client_on_controller_closed (GDBusConnection *connection,
    gboolean vanished, GError *error, gpointer user_data)
{
  GstSwitchClient * client = GST_SWITCH_CLIENT (user_data);
  GstSwitchClientClass *klass = GST_SWITCH_CLIENT_CLASS (
      G_OBJECT_GET_CLASS (client));
  if (klass->connection_closed)
    (*klass->connection_closed) (client, error);
}

static void
gst_switch_client_connect_controller (GstSwitchClient * client)
{
  GError *error = NULL;
  guint id;

  client->controller = g_dbus_connection_new_for_address_sync (
      SWITCH_CONTROLLER_ADDRESS,
      G_DBUS_SERVER_FLAGS_RUN_IN_THREAD |
      G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT,
      NULL, /* GDBusAuthObserver */
      NULL, /* GCancellable */
      &error);

  if (client->controller == NULL)
    goto error_new_connection;

  g_signal_connect (client->controller, "closed",
      G_CALLBACK (gst_switch_client_on_controller_closed), client);

  /* Register Object */
  id = g_dbus_connection_register_object (client->controller,
      SWITCH_CLIENT_OBJECT_PATH,
      introspection_data->interfaces[0],
      &gst_switch_client_interface_vtable,
      client, /* user_data */
      NULL, /* user_data_free_func */
      &error);

  if (id <= 0)
    goto error_register_object;

  id = g_dbus_connection_signal_subscribe (client->controller,
      NULL, /* sender */
      SWITCH_CONTROLLER_OBJECT_NAME,
      NULL, /* member */
      SWITCH_CONTROLLER_OBJECT_PATH,
      NULL, /* arg0 */
      G_DBUS_SIGNAL_FLAGS_NONE,
      gst_switch_client_on_signal_received,
      client, NULL/* user_data, user_data_free_func */);

  if (id <= 0)
    goto error_subscribe;
  
  return;

 error_new_connection:
  {
    ERROR ("%s", error->message);
    g_error_free (error);
    return;
  }

 error_register_object:
  {
    ERROR ("%s", error->message);
    g_error_free (error);
    return;
  }

 error_subscribe:
  {
    ERROR ("failed to subscribe signals");
    return;
  }
}

gboolean
gst_switch_client_connect (GstSwitchClient * client)
{
  gst_switch_client_connect_controller (client);

#if ENABLE_TEST
  {
    gchar *test_result = NULL;
    test_result = gst_switch_client_controller_test (client, "hello, controller");
    if (test_result) {
      INFO ("%s", test_result);
      g_free (test_result);
    }
  }
#endif

  return client->controller ? TRUE : FALSE;
}

static void
gst_switch_client_set_compose_port (GstSwitchClient *client, gint port)
{
  GstSwitchClientClass *klass = GST_SWITCH_CLIENT_CLASS (
      G_OBJECT_GET_CLASS (client));
  if (klass->set_compose_port)
    (*klass->set_compose_port) (client, port);
}

static void
gst_switch_client_set_audio_port (GstSwitchClient *client, gint port)
{
  GstSwitchClientClass *klass = GST_SWITCH_CLIENT_CLASS (
      G_OBJECT_GET_CLASS (client));
  if (klass->set_audio_port)
    (*klass->set_audio_port) (client, port);
}

static void
gst_switch_client_add_preview_port (GstSwitchClient *client, gint port, gint type)
{
  GstSwitchClientClass *klass = GST_SWITCH_CLIENT_CLASS (
      G_OBJECT_GET_CLASS (client));
  if (klass->add_preview_port)
    (*klass->add_preview_port) (client, port, type);
}

#if ENABLE_TEST
static GVariant *
gst_switch_client__test (GstSwitchClient *client, GDBusConnection *connection,
    GVariant *parameters)
{
  gchar *s = NULL;
  g_variant_get (parameters, "(&s)", &s);
  INFO ("%s", s);
  return g_variant_new ("(s)", "hello, controller");
}
#endif//ENABLE_TEST

static GVariant *
gst_switch_client__set_audio_port (GstSwitchClient *client,
    GDBusConnection *connection, GVariant *parameters)
{
  gint port = 0;
  g_variant_get (parameters, "(i)", &port);
  INFO ("audio: %d", port);
  gst_switch_client_set_audio_port (client, port);
  return NULL;
}

static GVariant *
gst_switch_client__set_compose_port (GstSwitchClient *client,
    GDBusConnection *connection, GVariant *parameters)
{
  gint port = 0;
  g_variant_get (parameters, "(i)", &port);
  INFO ("compose: %d", port);
  gst_switch_client_set_compose_port (client, port);
  return NULL;
}

static GVariant *
gst_switch_client__add_preview_port (GstSwitchClient *client,
    GDBusConnection *connection, GVariant *parameters)
{
  gint port = 0;
  gint type = 0;
  g_variant_get (parameters, "(ii)", &port, &type);
  INFO ("preview: %d, %d", port, type);
  gst_switch_client_add_preview_port (client, port, type);
  return NULL;
}

static MethodTableEntry gst_switch_client_method_table[] = {
#if ENABLE_TEST
  { "test", (MethodFunc) gst_switch_client__test },
#endif//ENABLE_TEST
  { "set_audio_port", (MethodFunc) gst_switch_client__set_audio_port },
  { "set_compose_port", (MethodFunc) gst_switch_client__set_compose_port },
  { "add_preview_port", (MethodFunc) gst_switch_client__add_preview_port },
  { NULL, NULL }
};

static void
gst_switch_client_class_init (GstSwitchClientClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = (GObjectFinalizeFunc) gst_switch_client_finalize;

  klass->methods = g_hash_table_new (g_str_hash, g_str_equal);

  MethodTableEntry *entry = &gst_switch_client_method_table[0];
  for (; entry->name && entry->func; ++entry) {
    g_hash_table_insert (klass->methods, (gpointer) entry->name,
	(gpointer) entry->func);
  }

  introspection_data = g_dbus_node_info_new_for_xml (introspection_xml, NULL);
  g_assert (introspection_data != NULL);
}
