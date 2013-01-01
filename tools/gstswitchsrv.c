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
#include <gio/gio.h>
#include <stdlib.h>
#include "gstswitchsrv.h"
#include "gstcase.h"
#include "./gio/gsocketinputstream.h"

#define GETTEXT_PACKAGE "switchsrv"
#define GST_SWITCH_SERVER_DEFAULT_HOST "localhost"
#define GST_SWITCH_SERVER_DEFAULT_ACCEPTOR_PORT 3000
#define GST_SWITCH_SERVER_DEFAULT_CONTROLLER_PORT 5000
#define GST_SWITCH_SERVER_LISTEN_BACKLOG 5 /* client connection queue */

#define GST_SWITCH_SERVER_LOCK_ACCEPTOR(srv) (g_mutex_lock (&(srv)->acceptor_lock))
#define GST_SWITCH_SERVER_UNLOCK_ACCEPTOR(srv) (g_mutex_unlock (&(srv)->acceptor_lock))
#define GST_SWITCH_SERVER_LOCK_CONTROLLER(srv) (g_mutex_lock (&(srv)->controller_lock))
#define GST_SWITCH_SERVER_UNLOCK_CONTROLLER(srv) (g_mutex_unlock (&(srv)->controller_lock))
#define GST_SWITCH_SERVER_LOCK_CASES(srv) (g_mutex_lock (&(srv)->cases_lock))
#define GST_SWITCH_SERVER_UNLOCK_CASES(srv) (g_mutex_unlock (&(srv)->cases_lock))
#define GST_SWITCH_SERVER_LOCK_COMPOSITE(srv) (g_mutex_lock (&(srv)->composite_lock))
#define GST_SWITCH_SERVER_UNLOCK_COMPOSITE(srv) (g_mutex_unlock (&(srv)->composite_lock))

G_DEFINE_TYPE (GstSwitchServer, gst_switchsrv, G_TYPE_OBJECT);

GstSwitchServerOpts opts = {
  NULL,
  GST_SWITCH_SERVER_DEFAULT_ACCEPTOR_PORT,
  GST_SWITCH_SERVER_DEFAULT_CONTROLLER_PORT,
};

gboolean verbose;

static GOptionEntry entries[] = {
  {"verbose", 'v', 0,		G_OPTION_ARG_NONE, &verbose,
       "Prompt more messages", NULL},
  {"test-switch", 't', 0,	G_OPTION_ARG_STRING, &opts.test_switch,
       "Perform switch test", "OUTPUT"},
  {"input-port", 'p', 0,	G_OPTION_ARG_INT, &opts.input_port,
       "Specify the listen port.", "NUM"},
  {"control-port", 'p', 0,	G_OPTION_ARG_INT, &opts.control_port,
       "Specify the control port.", "NUM"},
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
  srv->host = g_strdup (GST_SWITCH_SERVER_DEFAULT_HOST);

  srv->cancellable = g_cancellable_new ();
  srv->acceptor_port = GST_SWITCH_SERVER_DEFAULT_ACCEPTOR_PORT;
  srv->acceptor_socket = NULL;
  srv->acceptor = NULL;
  srv->controller_port = GST_SWITCH_SERVER_DEFAULT_CONTROLLER_PORT;
  srv->controller_socket = NULL;
  srv->controller_thread = NULL;
  srv->main_loop = NULL;
  srv->cases = NULL;
  srv->composite = NULL;
  srv->alloc_port_count = 0;

  g_mutex_init (&srv->acceptor_lock);
  g_mutex_init (&srv->controller_lock);
  g_mutex_init (&srv->cases_lock);
  g_mutex_init (&srv->alloc_port_lock);
  g_mutex_init (&srv->composite_lock);
}

static void
gst_switchsrv_finalize (GstSwitchServer *srv)
{
  g_free (srv->host);
  srv->host = NULL;

  g_main_loop_quit (srv->main_loop);
  g_main_loop_unref (srv->main_loop);
  srv->main_loop = NULL;

  if (srv->cancellable) {
    g_object_unref (srv->cancellable);
    srv->cancellable = NULL;
  }

  if (srv->acceptor_socket) {
    g_object_unref (srv->acceptor_socket);
    srv->acceptor_socket = NULL;
  }

  if (srv->controller_socket) {
    g_object_unref (srv->controller_socket);
    srv->controller_socket = NULL;
  }

  if (srv->acceptor) {
    g_thread_join (srv->acceptor);
  }

  if (srv->controller_thread) {
    g_thread_join (srv->controller_thread);
  }

  if (srv->cases) {
    g_list_free_full (srv->cases, (GDestroyNotify) g_object_unref);
    srv->cases = NULL;
  }

  if (srv->composite) {
    g_object_unref (srv->composite);
    srv->composite = NULL;
  }

  g_mutex_clear (&srv->acceptor_lock);
  g_mutex_clear (&srv->controller_lock);
  g_mutex_clear (&srv->cases_lock);
  g_mutex_clear (&srv->alloc_port_lock);
  g_mutex_clear (&srv->composite_lock);

  if (G_OBJECT_CLASS (gst_switchsrv_parent_class)->finalize)
    (*G_OBJECT_CLASS (gst_switchsrv_parent_class)->finalize) (G_OBJECT (srv));
}

static void
gst_switchsrv_class_init (GstSwitchServerClass *klass)
{
  GObjectClass * object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = (GObjectFinalizeFunc) gst_switchsrv_finalize;
}

static void
gst_switchsrv_end_case (GstCase *cas, GstSwitchServer *srv)
{
  GST_SWITCH_SERVER_LOCK_CASES (srv);
  g_object_unref (cas);
  srv->cases = g_list_remove (srv->cases, cas);
  GST_SWITCH_SERVER_UNLOCK_CASES (srv);

  INFO ("removed case %p (%d cases left)", cas, g_list_length (srv->cases));
}

static void
gst_switchsrv_end_composite (GstComposite *composite, GstSwitchServer *srv)
{
  GST_SWITCH_SERVER_LOCK_COMPOSITE (srv);
  g_object_unref (srv->composite);
  srv->composite = NULL;
  GST_SWITCH_SERVER_UNLOCK_COMPOSITE (srv);
}

static gint
gst_switchsrv_alloc_port (GstSwitchServer *srv)
{
  gint port;
  g_mutex_lock (&srv->alloc_port_lock);
  srv->alloc_port_count += 1;
  port = srv->acceptor_port + srv->alloc_port_count;

  // TODO: new policy for port allocation

  g_mutex_unlock (&srv->alloc_port_lock);
  return port;
}

static void
gst_switchsrv_revoke_port (GstSwitchServer *srv, int port)
{
  g_mutex_lock (&srv->alloc_port_lock);
  //srv->alloc_port_count -= 1;

  // TODO: new policy for port allocation

  g_mutex_unlock (&srv->alloc_port_lock);
}

static void
gst_switchsrv_serve (GstSwitchServer *srv, GSocket *client)
{
  GSocketInputStream *stream = G_SOCKET_INPUT_STREAM (g_object_new (
	  G_TYPE_SOCKET_INPUT_STREAM, "socket", client, NULL));
  gint port = gst_switchsrv_alloc_port (srv);
  GstCase *workcase;
  GstCaseType type;
  gchar *name;
  gint num = g_list_length (srv->cases);

  switch (num) {
  case 0:  type = GST_CASE_COMPOSITE_A; break;
  case 1:  type = GST_CASE_COMPOSITE_B; break;
  default: type = GST_CASE_PREVIEW; break;
  }

  name = g_strdup_printf ("case-%d", num);
  workcase = GST_CASE (g_object_new (GST_TYPE_CASE, "name", name,
	  "type", type, "port", port, "stream", stream, NULL));
  g_free (name);

  g_object_unref (client);
  g_object_unref (stream);

  if (!gst_worker_prepare (GST_WORKER (workcase)))
    goto error_prepare_workcase;

  g_signal_connect (workcase, "end-case",
      G_CALLBACK (gst_switchsrv_end_case), srv);

  gst_worker_start (GST_WORKER (workcase));

  GST_SWITCH_SERVER_LOCK_CASES (srv);
  srv->cases = g_list_append (srv->cases, workcase);
  GST_SWITCH_SERVER_UNLOCK_CASES (srv);

  INFO ("New client sink to %d (%d cases) (%p)",
      port, g_list_length (srv->cases), srv);
  return;

  /* Errors Handling */

 error_prepare_workcase:
  {
    ERROR ("can't serve new client");
    g_object_unref (workcase);
    gst_switchsrv_revoke_port (srv, port);
    return;
  }
}

static void
gst_switchsrv_allow_tcp_control (GstSwitchServer *srv, GSocket *client)
{
  ERROR ("control via TCP not implemented");
  g_object_unref (client);
}

static GSocket *
gst_switchsrv_listen (GstSwitchServer *srv, gint port,
    gint *bound_port)
{
  GError *err = NULL;
  GInetAddress *addr;
  GSocket *socket = NULL;
  GSocketAddress *saddr;
  GResolver *resolver;
  gchar *ip;

  *bound_port = 0;

  /* look up name if we need to */
  addr = g_inet_address_new_from_string (srv->host);
  if (!addr) {
    GList *results;

    resolver = g_resolver_get_default ();
    results = g_resolver_lookup_by_name (resolver, srv->host,
        srv->cancellable, &err);
    if (!results)
      goto resolve_no_name;

    addr = G_INET_ADDRESS (g_object_ref (results->data));

    g_resolver_free_addresses (results);
    g_object_unref (resolver);
  }

  ip = g_inet_address_to_string (addr);
  saddr = g_inet_socket_address_new (addr, port);
  g_object_unref (addr);

  /* create the server listener socket */
  socket = g_socket_new (g_socket_address_get_family (saddr),
      G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_TCP, &err);
  if (!socket)
    goto socket_new_failed;
  
  /* bind it */
  if (!g_socket_bind (socket, saddr, TRUE, &err))
    goto socket_bind_failed;

  g_object_unref (saddr);

  /* listen on the socket */
  g_socket_set_listen_backlog (socket,
      GST_SWITCH_SERVER_LISTEN_BACKLOG);
  if (!g_socket_listen (socket, &err))
    goto socket_listen_failed;

  if (port == 0) {
    saddr = g_socket_get_local_address (socket, NULL);
    *bound_port = g_inet_socket_address_get_port ((GInetSocketAddress *) saddr);
    g_object_unref (saddr);
  } else {
    *bound_port = port;
  }

  INFO ("Listening on %s (%s:%d)", srv->host, ip, *bound_port);

  g_free (ip);

  //g_atomic_int_set (&srv->bound_port, bound_port);
  //g_object_notify (G_OBJECT (src), "bound-port");
  return socket;

  /* Errors Handling */

 resolve_no_name:
  {
    ERROR ("resolve: %s", err->message);
    g_object_unref (resolver);
    g_object_unref (addr);
    return NULL;
  }

 socket_new_failed:
  {
    ERROR ("new socket: %s", err->message);
    g_clear_error (&err);
    g_object_unref (saddr);
    g_free (ip);
    return NULL;
  }

 socket_bind_failed:
  {
    ERROR ("bind socket: %s", err->message);
    g_clear_error (&err);
    g_object_unref (saddr);
    g_free (ip);
    return NULL;
  }

 socket_listen_failed:
  {
    ERROR ("listen socket: %s", err->message);
    g_clear_error (&err);
    g_object_unref (saddr);
    g_free (ip);
    return NULL;
  }
}

static gpointer
gst_switchsrv_acceptor (GstSwitchServer *srv)
{
  GSocket *socket;
  GError *error;
  gint bound_port;

  srv->acceptor_socket = gst_switchsrv_listen (srv,
      srv->acceptor_port, &bound_port);
  if (!srv->acceptor_socket) {
    return NULL;
  }

  while (srv->acceptor && srv->acceptor_socket && srv->cancellable) {
    socket = g_socket_accept (srv->acceptor_socket, srv->cancellable, &error);
    if (!socket) {
      ERROR ("accept: %s", error->message);
      continue;
    }

    gst_switchsrv_serve (srv, socket);
  }

  GST_SWITCH_SERVER_LOCK_ACCEPTOR (srv);
  g_thread_unref (srv->acceptor);
  srv->acceptor = NULL;
  GST_SWITCH_SERVER_UNLOCK_ACCEPTOR (srv);
  return NULL;
}

static gpointer
gst_switchsrv_controller (GstSwitchServer *srv)
{
  GSocket *socket;
  GError *error;
  gint bound_port;

  srv->controller_socket = gst_switchsrv_listen (srv,
      srv->controller_port, &bound_port);
  if (!srv->controller_socket) {
    return NULL;
  }

  while (srv->controller_thread && srv->controller_socket && srv->cancellable) {
    socket = g_socket_accept (srv->controller_socket, srv->cancellable, &error);
    if (!socket) {
      ERROR ("accept: %s", error->message);
      continue;
    }

    gst_switchsrv_allow_tcp_control (srv, socket);
  }

  GST_SWITCH_SERVER_LOCK_CONTROLLER (srv);
  g_thread_unref (srv->controller_thread);
  srv->controller_thread = NULL;
  GST_SWITCH_SERVER_UNLOCK_CONTROLLER (srv);
  return NULL;
}

static void
gst_switchsrv_prepare_bus_controller (GstSwitchServer * srv)
{
  if (!srv->controller) {
    GST_SWITCH_SERVER_LOCK_CONTROLLER (srv);
    srv->controller = GST_SWITCH_CONTROLLER (g_object_new (
	    GST_TYPE_SWITCH_CONTROLLER,
	    NULL));
    GST_SWITCH_SERVER_UNLOCK_CONTROLLER (srv);
  }

  /*
  GDBusMessage *m = g_dbus_message_new_method_call (
      "info.duzy.GstSwitchController",
      "/info/duzy/GstSwitchController",
      "info.duzy.GstSwitchController",
      "test");
  //g_dbus_message_set ()
  */
}

static gboolean
gst_switchsrv_prepare_composite (GstSwitchServer * srv)
{
  gint port = gst_switchsrv_alloc_port (srv);
  port = gst_switchsrv_alloc_port (srv);

  GST_SWITCH_SERVER_LOCK_COMPOSITE (srv);
  srv->composite = GST_COMPOSITE (g_object_new (GST_TYPE_COMPOSITE,
	  "name", "composite", "port", port, NULL));
  GST_SWITCH_SERVER_UNLOCK_COMPOSITE (srv);

  if (!gst_worker_prepare (GST_WORKER (srv->composite)))
    goto error_prepare_composite;

  g_signal_connect (srv->composite, "end-composite",
      G_CALLBACK (gst_switchsrv_end_composite), srv);

  gst_worker_start (GST_WORKER (srv->composite));

  return TRUE;

  /* Errors Handling */

 error_prepare_composite:
  {
    ERROR ("failed to prepare composite");
    GST_SWITCH_SERVER_LOCK_COMPOSITE (srv);
    g_object_unref (srv->composite);
    srv->composite = NULL;
    GST_SWITCH_SERVER_UNLOCK_COMPOSITE (srv);
    return FALSE;
  }
}

static void
gst_switchsrv_run (GstSwitchServer * srv)
{
  srv->main_loop = g_main_loop_new (NULL, TRUE);

  srv->acceptor = g_thread_new ("switch-server-acceptor",
      (GThreadFunc) gst_switchsrv_acceptor, srv);

  srv->controller_thread = g_thread_new ("switch-server-controller",
      (GThreadFunc) gst_switchsrv_controller, srv);

  gst_switchsrv_prepare_bus_controller (srv);

  if (!gst_switchsrv_prepare_composite (srv))
    goto error_prepare_composite;

  g_main_loop_run (srv->main_loop);

  g_thread_join (srv->acceptor);
  g_thread_join (srv->controller_thread);
  return;

  /* Errors Handling */
 error_prepare_composite:
  {
    ERROR ("failed to prepare composite");
    return;
  }
}

int
main (int argc, char *argv[])
{
  GstSwitchServer *srv;

  gst_switchsrv_parse_args (&argc, &argv);

  srv = GST_SWITCH_SERVER (g_object_new (GST_TYPE_SWITCH_SERVER, NULL));

  gst_switchsrv_run (srv);

  g_object_unref (G_OBJECT (srv));
  return 0;
}
