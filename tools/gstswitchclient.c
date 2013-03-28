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

/*! @file */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include "gstswitchclient.h"
#include "gstswitchcontroller.h"

#define parent_class gst_switch_client_parent_class
G_DEFINE_TYPE (GstSwitchClient, gst_switch_client, G_TYPE_OBJECT);

#define GST_SWITCH_CLIENT_LOCK_CONTROLLER(c) (g_mutex_lock (&(c)->controller_lock))
#define GST_SWITCH_CLIENT_UNLOCK_CONTROLLER(c) (g_mutex_unlock (&(c)->controller_lock))
#define GST_SWITCH_CLIENT_LOCK_COMPOSITE_MODE(c) (g_mutex_lock (&(c)->composite_mode_lock))
#define GST_SWITCH_CLIENT_UNLOCK_COMPOSITE_MODE(c) (g_mutex_unlock (&(c)->composite_mode_lock))

static GDBusNodeInfo *introspection_data = NULL;
static const gchar introspection_xml[] =
    "<node>" "  <interface name='" SWITCH_CLIENT_OBJECT_NAME "'>"
#if ENABLE_TEST
    "    <method name='test'>"
    "      <arg type='s' name='name' direction='in'/>"
    "      <arg type='s' name='result' direction='out'/>" "    </method>"
#endif //ENABLE_TEST
    "    <method name='set_audio_port'>"
    "      <arg type='i' name='port' direction='in'/>"
    "    </method>"
    "    <method name='set_compose_port'>"
    "      <arg type='i' name='port' direction='in'/>"
    "    </method>"
    "    <method name='set_encode_port'>"
    "      <arg type='i' name='port' direction='in'/>"
    "    </method>"
    "    <method name='add_preview_port'>"
    "      <arg type='i' name='port' direction='in'/>"
    "      <arg type='i' name='serve' direction='in'/>"
    "      <arg type='i' name='type' direction='in'/>"
    "    </method>"
    "    <method name='new_mode_online'>"
    "      <arg type='i' name='mode' direction='in'/>"
    "    </method>" "  </interface>" "</node>";

extern gboolean verbose;

static void
gst_switch_client_init (GstSwitchClient * client)
{
  g_mutex_init (&client->controller_lock);
  g_mutex_init (&client->composite_mode_lock);
}

static void
gst_switch_client_finalize (GstSwitchClient * client)
{
  g_mutex_clear (&client->controller_lock);
  g_mutex_clear (&client->composite_mode_lock);

  if (G_OBJECT_CLASS (parent_class)->finalize)
    (*G_OBJECT_CLASS (parent_class)->finalize) (G_OBJECT (client));
}

static GVariant *
gst_switch_client_call_controller (GstSwitchClient * client,
    const gchar * method_name,
    GVariant * parameters, const GVariantType * reply_type)
{
  GVariant *value = NULL;
  GError *error = NULL;

  GST_SWITCH_CLIENT_LOCK_CONTROLLER (client);
  if (!client->controller)
    goto error_no_controller_connection;

  //INFO ("calling: %s/%s", SWITCH_CONTROLLER_OBJECT_NAME, method_name);

  value = g_dbus_connection_call_sync (client->controller, NULL,        /* bus_name */
      SWITCH_CONTROLLER_OBJECT_PATH, SWITCH_CONTROLLER_OBJECT_NAME, method_name, parameters, reply_type, G_DBUS_CALL_FLAGS_NONE, 5000,  /* timeout_msec */
      NULL /* TODO: cancellable */ ,
      &error);

  if (!value)
    goto error_call_sync;

  GST_SWITCH_CLIENT_UNLOCK_CONTROLLER (client);
  return value;

  /* ERRORS */
error_no_controller_connection:
  {
    ERROR ("No controller connection");
    GST_SWITCH_CLIENT_UNLOCK_CONTROLLER (client);
    return NULL;
  }

error_call_sync:
  {
    ERROR ("%s", error->message);
    g_error_free (error);
    GST_SWITCH_CLIENT_UNLOCK_CONTROLLER (client);
    return NULL;
  }
}

#if ENABLE_TEST
static gchar *
gst_switch_client_controller_test (GstSwitchClient * client, const gchar * s)
{
  gchar *result = NULL;
  GVariant *value = gst_switch_client_call_controller (client, "test",
      g_variant_new ("(s)",
          s),
      G_VARIANT_TYPE ("(s)"));

  if (value) {
    g_variant_get (value, "(&s)", &result);
    if (result)
      result = g_strdup (result);
    g_variant_unref (value);
  }

  return result;
}
#endif //ENABLE_TEST

gint
gst_switch_client_get_compose_port (GstSwitchClient * client)
{
  gint port = 0;
  GVariant *value =
      gst_switch_client_call_controller (client, "get_compose_port",
      NULL, G_VARIANT_TYPE ("(i)"));
  if (value) {
    g_variant_get (value, "(i)", &port);
  }
  return port;
}

/**
 * gst_switch_client_get_encode_port:
 *  @param client the GstSwitchClient instance
 *  @param return the encode port number
 *
 *  Get the encode port number.
 *  
 */
gint
gst_switch_client_get_encode_port (GstSwitchClient * client)
{
  gint port = 0;
  GVariant *value =
      gst_switch_client_call_controller (client, "get_encode_port",
      NULL, G_VARIANT_TYPE ("(i)"));
  if (value) {
    g_variant_get (value, "(i)", &port);
  }
  return port;
}

/**
 * gst_switch_client_get_audio_port:
 *  @param client the GstSwitchClient instance
 *  @param return the audio port number
 *
 *  The the audio port number.
 *  
 */
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

/**
 * gst_switch_client_get_preview_ports:
 *  @param client the GstSwitchClient instance
 *  @param return The preview ports of type GVariant
 *
 *  The all preview ports.
 *
 */
GVariant *
gst_switch_client_get_preview_ports (GstSwitchClient * client)
{
  return gst_switch_client_call_controller (client, "get_preview_ports",
      NULL, G_VARIANT_TYPE ("(s)"));
}

/**
 * gst_switch_client_switch:
 *  @param client the GstSwitchClient instance
 *  @param channel The channel to be switched, 'A', 'B', 'a'
 *  @param port The target port number
 *  @param return TRUE when requested.
 *
 *  Switch the channel to the target port.
 *  
 */
gboolean
gst_switch_client_switch (GstSwitchClient * client, gint channel, gint port)
{
  gboolean result = FALSE;
  GVariant *value = gst_switch_client_call_controller (client, "switch",
      g_variant_new ("(ii)",
          channel,
          port),
      G_VARIANT_TYPE ("(b)"));
  if (value) {
    g_variant_get (value, "(b)", &result);
  }
  return result;
}

/**
 * gst_switch_client_set_composite_mode:
 *  @param client the GstSwitchClient instance
 *  @param return TRUE when requested.
 *
 *  Set the current composite mode.
 *  
 */
gboolean
gst_switch_client_set_composite_mode (GstSwitchClient * client, gint mode)
{
  gboolean result = FALSE;
  GVariant *value = NULL;

  //INFO ("changing: %d", client->changing_composite_mode);

  /* Only commit the change-composite-mode request once a time.
   */
  if (!client->changing_composite_mode) {
    GST_SWITCH_CLIENT_LOCK_COMPOSITE_MODE (client);
    if (!client->changing_composite_mode) {
      value = gst_switch_client_call_controller (client,
          "set_composite_mode",
          g_variant_new ("(i)", mode), G_VARIANT_TYPE ("(b)"));
      if (value) {
        g_variant_get (value, "(b)", &result);
        client->changing_composite_mode = result;
      }
    }
    GST_SWITCH_CLIENT_UNLOCK_COMPOSITE_MODE (client);
  }

  return result;
}

/**
 * gst_switch_client_new_record:
 *  @param client the GstSwitchClient instance
 *  @param return TRUE when requested.
 *
 *  Start a new recording.
 *
 */
gboolean
gst_switch_client_new_record (GstSwitchClient * client)
{
  gboolean result = FALSE;
  GVariant *value = gst_switch_client_call_controller (client,
      "new_record",
      g_variant_new ("()"),
      G_VARIANT_TYPE ("(b)"));
  if (value) {
    g_variant_get (value, "(b)", &result);
  }
  return result;
}

/**
 * gst_switch_client_adjust_pip:
 *  @param client the GstSwitchClient instance
 *  @param dx x position to be adjusted
 *  @param dy y position to be adjusted
 *  @param dw w position to be adjusted
 *  @param dh h position to be adjusted
 *  @param return a unsign integer indicating which components are changed of per
 *           bit.
 *
 *  Adjust the PIP.
 *
 */
guint
gst_switch_client_adjust_pip (GstSwitchClient * client, gint dx, gint dy,
    gint dw, gint dh)
{
  guint result = 0;
  GVariant *value = gst_switch_client_call_controller (client,
      "adjust_pip",
      g_variant_new ("(iiii)", dx, dy, dw,
          dh),
      G_VARIANT_TYPE ("(u)"));
  if (value) {
    g_variant_get (value, "(u)", &result);
  }
  return result;
}

/**
 * gst_switch_client_method_match:
 *
 * Predictor for matching the remoting method names.
 */
static gboolean
gst_switch_client_method_match (const gchar * key, MethodTableEntry * entry,
    const gchar * match)
{
  if (g_strcmp0 (key, match) == 0)
    return TRUE;
  return FALSE;
}

/**
 * gst_switch_client_do_method_call:
 *
 * Performing a remoting method call (typically invoked by the gst-switch-srv)
 */
static void
gst_switch_client_do_method_call (GDBusConnection * connection,
    const gchar * sender,
    const gchar * object_path,
    const gchar * interface_name,
    const gchar * method_name,
    GVariant * parameters,
    GDBusMethodInvocation * invocation, gpointer user_data)
{
  GstSwitchClient *client = GST_SWITCH_CLIENT (user_data);
  GstSwitchClientClass *klass =
      GST_SWITCH_CLIENT_CLASS (G_OBJECT_GET_CLASS (client));
  MethodFunc entry = (MethodFunc) g_hash_table_find (klass->methods,
      (GHRFunc)
      gst_switch_client_method_match,
      (gpointer) method_name);
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

/**
 * gst_switch_client_do_get_property:
 *
 * Fetching property remotely (it's useless currently).
 */
static GVariant *
gst_switch_client_do_get_property (GDBusConnection * connection,
    const gchar * sender,
    const gchar * object_path,
    const gchar * interface_name,
    const gchar * property_name, GError ** error, gpointer user_data)
{
  GVariant *ret = NULL;
  INFO ("get: %s", property_name);
  return ret;
}

/**
 * gst_switch_client_do_set_property:
 *
 * Setting property remotely (it's currently useless).
 */
static gboolean
gst_switch_client_do_set_property (GDBusConnection * connection,
    const gchar * sender,
    const gchar * object_path,
    const gchar * interface_name,
    const gchar * property_name,
    GVariant * value, GError ** error, gpointer user_data)
{
  INFO ("set: %s", property_name);
  return FALSE;
}

static const GDBusInterfaceVTable gst_switch_client_interface_vtable = {
  gst_switch_client_do_method_call,
  gst_switch_client_do_get_property,
  gst_switch_client_do_set_property
};

/**
 * gst_switch_client_on_signal_received:
 *
 * Remote signal handler (useless currently).
 */
static void
gst_switch_client_on_signal_received (GDBusConnection * connection,
    const gchar * sender_name,
    const gchar * object_path,
    const gchar * interface_name,
    const gchar * signal_name, GVariant * parameters, gpointer user_data)
{
  GstSwitchClient *client = GST_SWITCH_CLIENT (user_data);

  (void) client;

  INFO ("signal: %s, %s", sender_name, signal_name);
}

/**
 * gst_switch_client_on_controller_closed:
 *
 * Invoked when the remote controller is closed.
 */
static void
gst_switch_client_on_controller_closed (GDBusConnection * connection,
    gboolean vanished, GError * error, gpointer user_data)
{
  GstSwitchClient *client = GST_SWITCH_CLIENT (user_data);
  GstSwitchClientClass *klass =
      GST_SWITCH_CLIENT_CLASS (G_OBJECT_GET_CLASS (client));
  if (klass->connection_closed)
    (*klass->connection_closed) (client, error);
}

/**
 * gst_switch_client_connect_controller:
 *
 * Invoked when the remote controller is connected.
 */
static void
gst_switch_client_connect_controller (GstSwitchClient * client)
{
  GError *error = NULL;
  guint id;

  GST_SWITCH_CLIENT_LOCK_CONTROLLER (client);

  client->controller = g_dbus_connection_new_for_address_sync (SWITCH_CONTROLLER_ADDRESS, G_DBUS_SERVER_FLAGS_RUN_IN_THREAD | G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT, NULL,      /* GDBusAuthObserver */
      NULL,                     /* GCancellable */
      &error);

  if (client->controller == NULL)
    goto error_new_connection;

  g_signal_connect (client->controller, "closed",
      G_CALLBACK (gst_switch_client_on_controller_closed), client);

  /* Register Object */
  id = g_dbus_connection_register_object (client->controller, SWITCH_CLIENT_OBJECT_PATH, introspection_data->interfaces[0], &gst_switch_client_interface_vtable, client,        /* user_data */
      NULL,                     /* user_data_free_func */
      &error);

  if (id <= 0)
    goto error_register_object;

  id = g_dbus_connection_signal_subscribe (client->controller, NULL,    /* sender */
      SWITCH_CONTROLLER_OBJECT_NAME, NULL,      /* member */
      SWITCH_CONTROLLER_OBJECT_PATH, NULL,      /* arg0 */
      G_DBUS_SIGNAL_FLAGS_NONE,
      gst_switch_client_on_signal_received, client, NULL
      /* user_data, user_data_free_func */
      );

  if (id <= 0)
    goto error_subscribe;

  GST_SWITCH_CLIENT_UNLOCK_CONTROLLER (client);
  return;

error_new_connection:
  {
    ERROR ("%s", error->message);
    g_error_free (error);
    GST_SWITCH_CLIENT_UNLOCK_CONTROLLER (client);
    return;
  }

error_register_object:
  {
    ERROR ("%s", error->message);
    g_error_free (error);
    GST_SWITCH_CLIENT_UNLOCK_CONTROLLER (client);
    return;
  }

error_subscribe:
  {
    ERROR ("failed to subscribe signals");
    GST_SWITCH_CLIENT_UNLOCK_CONTROLLER (client);
    return;
  }
}

/**
 * gst_switch_client_is_connected:
 *  @param client the GstSwitchClient instance
 *  @param return TRUE when requested.
 *
 *  Check if the client is connected to the gst-switch server.
 *  
 */
gboolean
gst_switch_client_is_connected (GstSwitchClient * client)
{
  gboolean result = FALSE;
  GST_SWITCH_CLIENT_LOCK_CONTROLLER (client);
  if (client->controller)
    result = TRUE;
  GST_SWITCH_CLIENT_UNLOCK_CONTROLLER (client);
  return result;
}

/**
 * gst_switch_client_connect:
 *  @param client the GstSwitchClient instance
 *  @param return TRUE when requested.
 *
 *  Connect the client with the gst-switch server.
 *  
 */
gboolean
gst_switch_client_connect (GstSwitchClient * client)
{
  gst_switch_client_connect_controller (client);

#if ENABLE_TEST
  {
    gchar *test_result = NULL;
    test_result =
        gst_switch_client_controller_test (client, "hello, controller");
    if (test_result) {
      INFO ("%s", test_result);
      g_free (test_result);
    }
  }
#endif

  return gst_switch_client_is_connected (client);
}

/**
 * gst_switch_client_get_compose_port:
 *  @param client the GstSwitchClient instance
 *  @param return the compose port number
 *
 *  Get the compose port number.
 *  
 */
static void
gst_switch_client_set_compose_port (GstSwitchClient * client, gint port)
{
  GstSwitchClientClass *klass =
      GST_SWITCH_CLIENT_CLASS (G_OBJECT_GET_CLASS (client));

  if (klass->set_compose_port)
    (*klass->set_compose_port) (client, port);
}

/**
 * gst_switch_client_set_encode_port:
 *
 * Setting the encode port number remotely.
 */
static void
gst_switch_client_set_encode_port (GstSwitchClient * client, gint port)
{
  GstSwitchClientClass *klass =
      GST_SWITCH_CLIENT_CLASS (G_OBJECT_GET_CLASS (client));
  if (klass->set_encode_port)
    (*klass->set_encode_port) (client, port);
}

/**
 * gst_switch_client_set_audio_port:
 *
 * Setting the audio port number remotely.
 */
static void
gst_switch_client_set_audio_port (GstSwitchClient * client, gint port)
{
  GstSwitchClientClass *klass =
      GST_SWITCH_CLIENT_CLASS (G_OBJECT_GET_CLASS (client));
  if (klass->set_audio_port)
    (*klass->set_audio_port) (client, port);
}

/**
 * gst_switch_client_add_preview_port:
 *
 * The remote controller is adding a new preview port.
 */
static void
gst_switch_client_add_preview_port (GstSwitchClient * client, gint port,
    gint serve, gint type)
{
  GstSwitchClientClass *klass =
      GST_SWITCH_CLIENT_CLASS (G_OBJECT_GET_CLASS (client));
  if (klass->add_preview_port)
    (*klass->add_preview_port) (client, port, serve, type);
}

/**
 * gst_switch_client_new_mode_online:
 *
 * The remote controller is telling about the new composite mode online.
 */
static void
gst_switch_client_new_mode_online (GstSwitchClient * client, gint mode)
{
  GstSwitchClientClass *klass =
      GST_SWITCH_CLIENT_CLASS (G_OBJECT_GET_CLASS (client));

  //INFO ("New composite mode: %d", mode);

  /* When a new composite mode changed, the server will inform us that it's
   * online, and when we receive that message, shall we release unset
   * changing_composite_mode.
   */
  if (client->changing_composite_mode) {
    GST_SWITCH_CLIENT_LOCK_COMPOSITE_MODE (client);
    if (client->changing_composite_mode) {
      client->changing_composite_mode = FALSE;
    }
    GST_SWITCH_CLIENT_UNLOCK_COMPOSITE_MODE (client);
  }

  if (klass->new_mode_online)
    (*klass->new_mode_online) (client, mode);
}

#if ENABLE_TEST
static GVariant *
gst_switch_client__test (GstSwitchClient * client,
    GDBusConnection * connection, GVariant * parameters)
{
  gchar *s = NULL;
  g_variant_get (parameters, "(&s)", &s);
  INFO ("%s", s);
  return g_variant_new ("(s)", "hello, controller");
}
#endif //ENABLE_TEST

/**
 * gst_switch_client__set_audio_port:
 *
 * Remoting method stub of "set_audio_port".
 */
static GVariant *
gst_switch_client__set_audio_port (GstSwitchClient * client,
    GDBusConnection * connection, GVariant * parameters)
{
  gint port = 0;
  g_variant_get (parameters, "(i)", &port);
  //INFO ("audio: %d", port);
  gst_switch_client_set_audio_port (client, port);
  return NULL;
}

/**
 * gst_switch_client__set_compose_port:
 *
 * Remoting method stub of "set_compose_port".
 */
static GVariant *
gst_switch_client__set_compose_port (GstSwitchClient * client,
    GDBusConnection * connection, GVariant * parameters)
{
  gint port = 0;
  g_variant_get (parameters, "(i)", &port);
  //INFO ("compose: %d", port);
  gst_switch_client_set_compose_port (client, port);
  return NULL;
}

/**
 * gst_switch_client__set_encode_port:
 *
 * Remoting method stub of "set_encode_port".
 */
static GVariant *
gst_switch_client__set_encode_port (GstSwitchClient * client,
    GDBusConnection * connection, GVariant * parameters)
{
  gint port = 0;
  g_variant_get (parameters, "(i)", &port);
  //INFO ("compose: %d", port);
  gst_switch_client_set_encode_port (client, port);
  return NULL;
}

/**
 * gst_switch_client__add_preview_port:
 *
 * Remoting method stub of "add_preview_port".
 */
static GVariant *
gst_switch_client__add_preview_port (GstSwitchClient * client,
    GDBusConnection * connection, GVariant * parameters)
{
  gint port = 0;
  gint serve = 0;
  gint type = 0;
  g_variant_get (parameters, "(iii)", &port, &serve, &type);
  //INFO ("preview: %d, %d, %d", port, serve, type);
  gst_switch_client_add_preview_port (client, port, serve, type);
  return NULL;
}

/**
 * gst_switch_client__new_mode_online:
 *
 * Remoting method stub of "new_mode_online".
 */
static GVariant *
gst_switch_client__new_mode_online (GstSwitchClient * client,
    GDBusConnection * connection, GVariant * parameters)
{
  gint mode = 0;
  g_variant_get (parameters, "(i)", &mode);
  //INFO ("compose: %d", port);
  gst_switch_client_new_mode_online (client, mode);
  return NULL;
}

/**
 * gst_switch_client_method_table:
 *
 * Remoting method table.
 */
static MethodTableEntry gst_switch_client_method_table[] = {
#if ENABLE_TEST
  {"test", (MethodFunc) gst_switch_client__test},
#endif //ENABLE_TEST
  {"set_audio_port", (MethodFunc) gst_switch_client__set_audio_port},
  {"set_compose_port", (MethodFunc) gst_switch_client__set_compose_port},
  {"set_encode_port", (MethodFunc) gst_switch_client__set_encode_port},
  {"add_preview_port", (MethodFunc) gst_switch_client__add_preview_port},
  {"new_mode_online", (MethodFunc) gst_switch_client__new_mode_online},
  {NULL, NULL}
};

/**
 * gst_switch_client_class_init:
 *
 * Initialize the GstSwitchClientClass.
 */
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
