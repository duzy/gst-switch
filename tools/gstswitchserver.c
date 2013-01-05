/* GstSwitchServer
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
#include "gstswitchserver.h"
#include "gstcase.h"
#include "./gio/gsocketinputstream.h"

#define GST_SWITCH_SERVER_DEFAULT_HOST "localhost"
#define GST_SWITCH_SERVER_DEFAULT_VIDEO_ACCEPTOR_PORT	3000
#define GST_SWITCH_SERVER_DEFAULT_AUDIO_ACCEPTOR_PORT	4000
#define GST_SWITCH_SERVER_DEFAULT_CONTROLLER_PORT	5000
#define GST_SWITCH_SERVER_LISTEN_BACKLOG 5 /* client connection queue */

#define GST_SWITCH_SERVER_LOCK_VIDEO_ACCEPTOR(srv) (g_mutex_lock (&(srv)->video_acceptor_lock))
#define GST_SWITCH_SERVER_UNLOCK_VIDEO_ACCEPTOR(srv) (g_mutex_unlock (&(srv)->video_acceptor_lock))
#define GST_SWITCH_SERVER_LOCK_AUDIO_ACCEPTOR(srv) (g_mutex_lock (&(srv)->audio_acceptor_lock))
#define GST_SWITCH_SERVER_UNLOCK_AUDIO_ACCEPTOR(srv) (g_mutex_unlock (&(srv)->audio_acceptor_lock))
#define GST_SWITCH_SERVER_LOCK_CONTROLLER(srv) (g_mutex_lock (&(srv)->controller_lock))
#define GST_SWITCH_SERVER_UNLOCK_CONTROLLER(srv) (g_mutex_unlock (&(srv)->controller_lock))
#define GST_SWITCH_SERVER_LOCK_CASES(srv) (g_mutex_lock (&(srv)->cases_lock))
#define GST_SWITCH_SERVER_UNLOCK_CASES(srv) (g_mutex_unlock (&(srv)->cases_lock))
#define GST_SWITCH_SERVER_LOCK_COMPOSITE(srv) (g_mutex_lock (&(srv)->composite_lock))
#define GST_SWITCH_SERVER_UNLOCK_COMPOSITE(srv) (g_mutex_unlock (&(srv)->composite_lock))

#define gst_switch_server_parent_class parent_class
G_DEFINE_TYPE (GstSwitchServer, gst_switch_server, G_TYPE_OBJECT);

GstSwitchServerOpts opts = {
  NULL, NULL, NULL,
  GST_SWITCH_SERVER_DEFAULT_VIDEO_ACCEPTOR_PORT,
  GST_SWITCH_SERVER_DEFAULT_AUDIO_ACCEPTOR_PORT,
  GST_SWITCH_SERVER_DEFAULT_CONTROLLER_PORT,
};

gboolean verbose;

static GOptionEntry entries[] = {
  {"verbose",		'v', 0,	G_OPTION_ARG_NONE,   &verbose,
       "Prompt more messages", NULL},
  {"test-switch",	't', 0,	G_OPTION_ARG_STRING, &opts.test_switch,
       "Perform switch test", "OUTPUT"},
  {"record",		'r', 0,	G_OPTION_ARG_STRING, &opts.record_filename,
       "Enable recorder and record into the specified FILENAME",
       "FILENAME"},
  {"video-input-port",	'p', 0,	G_OPTION_ARG_INT,    &opts.video_input_port,
       "Specify the video input listen port.", "NUM"},
  {"audio-input-port",	'a', 0,	G_OPTION_ARG_INT,    &opts.audio_input_port,
       "Specify the audio input listen port.", "NUM"},
  {"control-port",	'p', 0,	G_OPTION_ARG_INT,    &opts.control_port,
       "Specify the control port.", "NUM"},
  { NULL }
};

static void
gst_switch_server_parse_args (int *argc, char **argv[])
{
  GError *error = NULL;
  GOptionContext *context;

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
gst_switch_server_init (GstSwitchServer *srv)
{
  srv->host = g_strdup (GST_SWITCH_SERVER_DEFAULT_HOST);

  srv->cancellable = g_cancellable_new ();
  srv->video_acceptor_port = opts.video_input_port;
  srv->video_acceptor_socket = NULL;
  srv->video_acceptor = NULL;
  srv->audio_acceptor_port = opts.audio_input_port;
  srv->audio_acceptor_socket = NULL;
  srv->audio_acceptor = NULL;
  srv->controller_port = opts.control_port;
  srv->controller_socket = NULL;
  srv->controller_thread = NULL;
  srv->main_loop = NULL;
  srv->cases = NULL;
  srv->composite = NULL;
  srv->alloc_port_count = 0;

  g_mutex_init (&srv->video_acceptor_lock);
  g_mutex_init (&srv->audio_acceptor_lock);
  g_mutex_init (&srv->controller_lock);
  g_mutex_init (&srv->cases_lock);
  g_mutex_init (&srv->alloc_port_lock);
  g_mutex_init (&srv->composite_lock);
}

static void
gst_switch_server_finalize (GstSwitchServer *srv)
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

  if (srv->video_acceptor_socket) {
    g_object_unref (srv->video_acceptor_socket);
    srv->video_acceptor_socket = NULL;
  }

  if (srv->controller_socket) {
    g_object_unref (srv->controller_socket);
    srv->controller_socket = NULL;
  }

  if (srv->video_acceptor) {
    g_thread_join (srv->video_acceptor);
  }

  if (srv->audio_acceptor) {
    g_thread_join (srv->audio_acceptor);
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

  g_mutex_clear (&srv->video_acceptor_lock);
  g_mutex_clear (&srv->audio_acceptor_lock);
  g_mutex_clear (&srv->controller_lock);
  g_mutex_clear (&srv->cases_lock);
  g_mutex_clear (&srv->alloc_port_lock);
  g_mutex_clear (&srv->composite_lock);

  if (G_OBJECT_CLASS (parent_class)->finalize)
    (*G_OBJECT_CLASS (parent_class)->finalize) (G_OBJECT (srv));
}

static void
gst_switch_server_class_init (GstSwitchServerClass *klass)
{
  GObjectClass * object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = (GObjectFinalizeFunc) gst_switch_server_finalize;
}

static void
gst_switch_server_end_case (GstCase *cas, GstSwitchServer *srv)
{
  gint caseport = 0;

  GST_SWITCH_SERVER_LOCK_CASES (srv);
  caseport = cas->sink_port;
  g_object_unref (cas);
  srv->cases = g_list_remove (srv->cases, cas);
  GST_SWITCH_SERVER_UNLOCK_CASES (srv);

  if (caseport)
    gst_switch_server_revoke_port (caseport);

  INFO ("removed case %p (%d cases left)", cas, g_list_length (srv->cases));
}

static void
gst_switch_server_end_composite (GstComposite *composite, GstSwitchServer *srv)
{
  GST_SWITCH_SERVER_LOCK_COMPOSITE (srv);
  g_object_unref (srv->composite);
  srv->composite = NULL;
  GST_SWITCH_SERVER_UNLOCK_COMPOSITE (srv);
}

static gint
gst_switch_server_alloc_port (GstSwitchServer *srv)
{
  gint port;
  g_mutex_lock (&srv->alloc_port_lock);
  srv->alloc_port_count += 1;
  port = srv->video_acceptor_port + srv->alloc_port_count;

  // TODO: new policy for port allocation

  g_mutex_unlock (&srv->alloc_port_lock);
  return port;
}

static void
gst_switch_server_revoke_port (GstSwitchServer *srv, int port)
{
  g_mutex_lock (&srv->alloc_port_lock);
  //srv->alloc_port_count -= 1;

  // TODO: new policy for port allocation

  g_mutex_unlock (&srv->alloc_port_lock);
}

static void
gst_switch_server_serve (GstSwitchServer *srv, GSocket *client)
{
  GSocketInputStreamX *stream = G_SOCKET_INPUT_STREAM (g_object_new (
	  G_TYPE_SOCKET_INPUT_STREAM, "socket", client, NULL));
  GstCase *workcase;
  GstCaseType type;
  gchar *name;
  gint port;
  gint num = g_list_length (srv->cases);

  // TODO switching policy
  switch (num) {
  case 0:  type = GST_CASE_COMPOSITE_A; break;
  case 1:  type = GST_CASE_COMPOSITE_B; break;
  default: type = GST_CASE_PREVIEW_VIDEO; break;
  }

  switch (type) {
  case GST_CASE_COMPOSITE_A:
  case GST_CASE_COMPOSITE_B:
    port = gst_switch_server_alloc_port (srv);
    break;
  default:
    port = srv->composite->sink_port;
    break;
  }

  name = g_strdup_printf ("case-%d", num);
  workcase = GST_CASE (g_object_new (GST_TYPE_CASE, "name", name,
	  "type", type, "port", port, "stream", stream,
	  NULL));
  g_free (name);
  g_object_unref (client);
  g_object_unref (stream);

  switch (type) {
  case GST_CASE_COMPOSITE_A:
  case GST_CASE_COMPOSITE_B:
    g_object_set (workcase,
	"awidth",  srv->composite->a_width,
	"aheight", srv->composite->a_height,
	"bwidth",  srv->composite->b_width,
	"bheight", srv->composite->b_height,
	NULL);
  default: break;
  }

  if (!gst_worker_prepare (GST_WORKER (workcase)))
    goto error_prepare_workcase;

  g_signal_connect (workcase, "end-case",
      G_CALLBACK (gst_switch_server_end_case), srv);

  gst_worker_start (GST_WORKER (workcase));

  GST_SWITCH_SERVER_LOCK_CASES (srv);
  srv->cases = g_list_append (srv->cases, workcase);
  GST_SWITCH_SERVER_UNLOCK_CASES (srv);

  switch (type) {
  case GST_CASE_COMPOSITE_A:
  case GST_CASE_COMPOSITE_B:
    gst_switch_controller_tell_compose_port (srv->controller, port);
    /* fallthrough */
  case GST_CASE_PREVIEW_VIDEO:
    gst_switch_controller_tell_preview_port (srv->controller, port);
    INFO ("New client sink to %d (%d cases) (%p)", port,
	g_list_length (srv->cases), srv);
    break;
  }
  return;

  /* Errors Handling */

 error_prepare_workcase:
  {
    ERROR ("can't serve new client");
    g_object_unref (stream);
    g_object_unref (workcase);
    gst_switch_server_revoke_port (srv, port);
    return;
  }
}

static void
gst_switch_server_allow_tcp_control (GstSwitchServer *srv, GSocket *client)
{
  ERROR ("control via TCP not implemented");
  g_object_unref (client);
}

static GSocket *
gst_switch_server_listen (GstSwitchServer *srv, gint port,
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
  g_socket_set_listen_backlog (socket, GST_SWITCH_SERVER_LISTEN_BACKLOG);
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
gst_switch_server_video_acceptor (GstSwitchServer *srv)
{
  GSocket *socket;
  GError *error;
  gint bound_port;

  srv->video_acceptor_socket = gst_switch_server_listen (srv,
      srv->video_acceptor_port, &bound_port);
  if (!srv->video_acceptor_socket) {
    return NULL;
  }

  while (srv->video_acceptor && srv->video_acceptor_socket && srv->cancellable) {
    socket = g_socket_accept (srv->video_acceptor_socket, srv->cancellable, &error);
    if (!socket) {
      ERROR ("accept: %s", error->message);
      continue;
    }

    gst_switch_server_serve (srv, socket);
  }

  GST_SWITCH_SERVER_LOCK_VIDEO_ACCEPTOR (srv);
  g_thread_unref (srv->video_acceptor);
  srv->video_acceptor = NULL;
  GST_SWITCH_SERVER_UNLOCK_VIDEO_ACCEPTOR (srv);
  return NULL;
}

static gpointer
gst_switch_server_audio_acceptor (GstSwitchServer *srv)
{
  GSocket *socket;
  GError *error;
  gint bound_port;

  srv->audio_acceptor_socket = gst_switch_server_listen (srv,
      srv->audio_acceptor_port, &bound_port);
  if (!srv->audio_acceptor_socket) {
    return NULL;
  }

  while (srv->audio_acceptor && srv->audio_acceptor_socket && srv->cancellable) {
    socket = g_socket_accept (srv->audio_acceptor_socket, srv->cancellable, &error);
    if (!socket) {
      ERROR ("accept: %s", error->message);
      continue;
    }

    gst_switch_server_serve (srv, socket);
  }

  GST_SWITCH_SERVER_LOCK_AUDIO_ACCEPTOR (srv);
  g_thread_unref (srv->audio_acceptor);
  srv->audio_acceptor = NULL;
  GST_SWITCH_SERVER_UNLOCK_AUDIO_ACCEPTOR (srv);
  return NULL;
}

static gpointer
gst_switch_server_controller (GstSwitchServer *srv)
{
  GSocket *socket;
  GError *error;
  gint bound_port;

  srv->controller_socket = gst_switch_server_listen (srv,
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

    gst_switch_server_allow_tcp_control (srv, socket);
  }

  GST_SWITCH_SERVER_LOCK_CONTROLLER (srv);
  g_thread_unref (srv->controller_thread);
  srv->controller_thread = NULL;
  GST_SWITCH_SERVER_UNLOCK_CONTROLLER (srv);
  return NULL;
}

static void
gst_switch_server_prepare_bus_controller (GstSwitchServer * srv)
{
  if (!srv->controller) {
    GST_SWITCH_SERVER_LOCK_CONTROLLER (srv);
    srv->controller = GST_SWITCH_CONTROLLER (g_object_new (
	    GST_TYPE_SWITCH_CONTROLLER, NULL));
    srv->controller->server = srv;
    GST_SWITCH_SERVER_UNLOCK_CONTROLLER (srv);
  }
}

static gboolean
gst_switch_server_prepare_composite (GstSwitchServer * srv)
{
  gint port = gst_switch_server_alloc_port (srv);

  INFO ("Compose sink to %d", port);
  GST_SWITCH_SERVER_LOCK_COMPOSITE (srv);
  srv->composite = GST_COMPOSITE (g_object_new (GST_TYPE_COMPOSITE,
	  "name", "composite", "port", port, NULL));
  GST_SWITCH_SERVER_UNLOCK_COMPOSITE (srv);

  if (!gst_worker_prepare (GST_WORKER (srv->composite)))
    goto error_prepare_composite;

  g_signal_connect (srv->composite, "end-composite",
      G_CALLBACK (gst_switch_server_end_composite), srv);

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

gint
gst_switch_server_get_composite_sink_port (GstSwitchServer * srv)
{
  gint port = 0;
  if (srv->composite) {
    GST_SWITCH_SERVER_LOCK_COMPOSITE (srv);
    port = srv->composite->sink_port;
    GST_SWITCH_SERVER_UNLOCK_COMPOSITE (srv);
  }
  return port;
}

GArray *gst_switch_server_get_preview_sink_ports (GstSwitchServer * srv)
{
  GArray *a = g_array_new (FALSE, TRUE, sizeof (gint));
  GList *item;
  GST_SWITCH_SERVER_LOCK_CASES (srv);
  for (item = srv->cases; item; item = g_list_next (item)) {
    a = g_array_append_val (a, GST_CASE (item->data)->sink_port);
  }
  GST_SWITCH_SERVER_UNLOCK_CASES (srv);
  return a;
}

static void
gst_switch_server_run (GstSwitchServer * srv)
{
  srv->main_loop = g_main_loop_new (NULL, TRUE);

  if (!gst_switch_server_prepare_composite (srv))
    goto error_prepare_composite;

  srv->video_acceptor = g_thread_new ("switch-server-video-acceptor",
      (GThreadFunc) gst_switch_server_video_acceptor, srv);

  srv->audio_acceptor = g_thread_new ("switch-server-audio-acceptor",
      (GThreadFunc) gst_switch_server_audio_acceptor, srv);

  srv->controller_thread = g_thread_new ("switch-server-controller",
      (GThreadFunc) gst_switch_server_controller, srv);

  gst_switch_server_prepare_bus_controller (srv);

  g_main_loop_run (srv->main_loop);

  g_thread_join (srv->video_acceptor);
  g_thread_join (srv->audio_acceptor);
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

  gst_switch_server_parse_args (&argc, &argv);

  srv = GST_SWITCH_SERVER (g_object_new (GST_TYPE_SWITCH_SERVER, NULL));

  gst_switch_server_run (srv);

  g_object_unref (G_OBJECT (srv));
  return 0;
}
