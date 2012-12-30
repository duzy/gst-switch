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
#define GST_SWITCH_SERVER_DEFAULT_PORT 3000
#define GST_SWITCH_SERVER_DEFAULT_HOST "localhost"
#define GST_SWITCH_SERVER_LISTEN_BACKLOG 5 /* client connection queue */

#define GST_SWITCH_SERVER_LOCK_ACCEPTOR(srv) (g_mutex_lock (&(srv)->acceptor_lock))
#define GST_SWITCH_SERVER_UNLOCK_ACCEPTOR(srv) (g_mutex_unlock (&(srv)->acceptor_lock))
#define GST_SWITCH_SERVER_LOCK_CASES(srv) (g_mutex_lock (&(srv)->cases_lock))
#define GST_SWITCH_SERVER_UNLOCK_CASES(srv) (g_mutex_unlock (&(srv)->cases_lock))

G_DEFINE_TYPE (GstSwitchServer, gst_switchsrv, G_TYPE_OBJECT);

GstSwitchServerOpts opts = {
  0, NULL, 3000,
};

static GOptionEntry entries[] = {
  {"verbose", 'v', 0,		G_OPTION_ARG_NONE, &opts.verbose,
       "Prompt more messages", NULL},
  {"test-switch", 't', 0,	G_OPTION_ARG_STRING, &opts.test_switch,
       "Perform switch test", "OUTPUT"},
  {"port", 'p', 0,		G_OPTION_ARG_INT, &opts.port,
       "Specify the listen port.", "NUM"},
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
  srv->port = GST_SWITCH_SERVER_DEFAULT_PORT;
  srv->host = g_strdup (GST_SWITCH_SERVER_DEFAULT_HOST);

  srv->cancellable = g_cancellable_new ();
  srv->server_socket = NULL;
  srv->acceptor = NULL;
  srv->main_loop = NULL;
  srv->cases = NULL;

  g_mutex_init (&srv->acceptor_lock);
  g_mutex_init (&srv->cases_lock);
}

static void
gst_switchsrv_finalize (GstSwitchServer *srv)
{
  g_free (srv->host);
  srv->host = NULL;

  g_mutex_clear (&srv->acceptor_lock);
  g_mutex_clear (&srv->cases_lock);

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
gst_switchsrv_serve (GstSwitchServer *srv, GSocket *client)
{
  GSocketInputStream *stream = G_SOCKET_INPUT_STREAM (g_object_new (
	  G_TYPE_SOCKET_INPUT_STREAM, "socket", client, NULL));
  GstCase *workcase;
  gchar *name;

  name = g_strdup_printf ("case-%d", g_list_length (srv->cases));
  workcase = GST_CASE (g_object_new (GST_TYPE_CASE, "name", name,
	  "stream", stream, NULL));
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

  INFO ("New client added (%d cases) (%p)", g_list_length (srv->cases), srv);
  return;

  /* Errors Handling */

 error_prepare_workcase:
  {
    ERROR ("can't serve new client");
    g_object_unref (workcase);
    return;
  }
}

static gboolean
gst_switchsrv_listen (GstSwitchServer *srv)
{
  GError *err = NULL;
  GInetAddress *addr;
  GSocketAddress *saddr;
  GResolver *resolver;
  gint bound_port = 0;
  gchar *ip;

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
  saddr = g_inet_socket_address_new (addr, srv->port);
  g_object_unref (addr);

  /* create the server listener socket */
  srv->server_socket = g_socket_new (g_socket_address_get_family (saddr),
      G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_TCP, &err);
  if (!srv->server_socket)
    goto socket_new_failed;
  
  /* bind it */
  if (!g_socket_bind (srv->server_socket, saddr, TRUE, &err))
    goto socket_bind_failed;

  g_object_unref (saddr);

  /* listen on the socket */
  g_socket_set_listen_backlog (srv->server_socket,
      GST_SWITCH_SERVER_LISTEN_BACKLOG);
  if (!g_socket_listen (srv->server_socket, &err))
    goto socket_listen_failed;

  if (srv->port == 0) {
    saddr = g_socket_get_local_address (srv->server_socket, NULL);
    bound_port = g_inet_socket_address_get_port ((GInetSocketAddress *) saddr);
    g_object_unref (saddr);
  } else {
    bound_port = srv->port;
  }

  INFO ("Listening on %s (%s:%d)", srv->host, ip, bound_port);

  g_free (ip);

  //g_atomic_int_set (&srv->bound_port, bound_port);
  //g_object_notify (G_OBJECT (src), "bound-port");
  return TRUE;

  /* Errors Handling */

 resolve_no_name:
  {
    ERROR ("resolve: %s", err->message);
    g_object_unref (resolver);
    g_object_unref (addr);
    return FALSE;
  }

 socket_new_failed:
  {
    ERROR ("new socket: %s", err->message);
    g_clear_error (&err);
    g_object_unref (saddr);
    g_free (ip);
    return FALSE;
  }

 socket_bind_failed:
  {
    ERROR ("bind socket: %s", err->message);
    g_clear_error (&err);
    g_object_unref (saddr);
    g_free (ip);
    return FALSE;
  }

 socket_listen_failed:
  {
    ERROR ("listen socket: %s", err->message);
    g_clear_error (&err);
    g_object_unref (saddr);
    g_free (ip);
    return FALSE;
  }
}

static gpointer
gst_switchsrv_acceptor (GstSwitchServer *srv)
{
  GSocket *socket;
  GError *error;

  if (!gst_switchsrv_listen (srv)) {
    return NULL;
  }

  while (srv->server_socket && srv->cancellable) {
    socket = g_socket_accept (srv->server_socket, srv->cancellable, &error);
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

static void
gst_switchsrv_run (GstSwitchServer * srv)
{
  srv->main_loop = g_main_loop_new (NULL, TRUE);

  srv->acceptor = g_thread_new ("switch-server-acceptor",
      (GThreadFunc) gst_switchsrv_acceptor, srv);

  g_main_loop_run (srv->main_loop);

  g_thread_join (srv->acceptor);
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
