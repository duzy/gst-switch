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
#define GST_SWITCH_SERVER_LISTEN_BACKLOG 8 /* client connection queue */

#define GST_SWITCH_SERVER_LOCK_VIDEO_ACCEPTOR(srv) (g_mutex_lock (&(srv)->video_acceptor_lock))
#define GST_SWITCH_SERVER_UNLOCK_VIDEO_ACCEPTOR(srv) (g_mutex_unlock (&(srv)->video_acceptor_lock))
#define GST_SWITCH_SERVER_LOCK_AUDIO_ACCEPTOR(srv) (g_mutex_lock (&(srv)->audio_acceptor_lock))
#define GST_SWITCH_SERVER_UNLOCK_AUDIO_ACCEPTOR(srv) (g_mutex_unlock (&(srv)->audio_acceptor_lock))
#define GST_SWITCH_SERVER_LOCK_CONTROLLER(srv) (g_mutex_lock (&(srv)->controller_lock))
#define GST_SWITCH_SERVER_UNLOCK_CONTROLLER(srv) (g_mutex_unlock (&(srv)->controller_lock))
#define GST_SWITCH_SERVER_LOCK_CASES(srv) (g_mutex_lock (&(srv)->cases_lock))
#define GST_SWITCH_SERVER_UNLOCK_CASES(srv) (g_mutex_unlock (&(srv)->cases_lock))
#define GST_SWITCH_SERVER_LOCK_SERVE(srv) (g_mutex_lock (&(srv)->serve_lock))
#define GST_SWITCH_SERVER_UNLOCK_SERVE(srv) (g_mutex_unlock (&(srv)->serve_lock))
#define GST_SWITCH_SERVER_LOCK_PIP(srv) (g_mutex_lock (&(srv)->pip_lock))
#define GST_SWITCH_SERVER_UNLOCK_PIP(srv) (g_mutex_unlock (&(srv)->pip_lock))

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
    ERROR ("option parsing failed: %s", error->message);
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
  srv->controller = NULL;
  srv->main_loop = NULL;
  srv->cases = NULL;
  srv->composite = NULL;
  srv->alloc_port_count = 0;

  srv->pip_x = 0;
  srv->pip_y = 0;
  srv->pip_w = 0;
  srv->pip_h = 0;

  g_mutex_init (&srv->video_acceptor_lock);
  g_mutex_init (&srv->audio_acceptor_lock);
  g_mutex_init (&srv->controller_lock);
  g_mutex_init (&srv->cases_lock);
  g_mutex_init (&srv->alloc_port_lock);
  g_mutex_init (&srv->pip_lock);
}

static void
gst_switch_server_finalize (GstSwitchServer *srv)
{
  g_free (srv->host);
  srv->host = NULL;

  g_main_loop_quit (srv->main_loop);
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
  g_mutex_clear (&srv->pip_lock);

  if (G_OBJECT_CLASS (parent_class)->finalize)
    (*G_OBJECT_CLASS (parent_class)->finalize) (G_OBJECT (srv));

  INFO ("SwitchServer finalized");
}

static void
gst_switch_server_class_init (GstSwitchServerClass *klass)
{
  GObjectClass * object_class = G_OBJECT_CLASS (klass);
  object_class->finalize = (GObjectFinalizeFunc) gst_switch_server_finalize;
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
gst_switch_server_end_case (GstCase *cas, GstSwitchServer *srv)
{
  gint caseport = 0;
  GList *item;

  GST_SWITCH_SERVER_LOCK_CASES (srv);

  switch (cas->type) {
  default:
    srv->cases = g_list_remove (srv->cases, cas);
    INFO ("Removed %s (%p) (%d cases left)", GST_WORKER (cas)->name, cas,
	g_list_length (srv->cases));
    caseport = cas->sink_port;
    g_object_unref (cas);
    break;
  case GST_CASE_INPUT_a:
  case GST_CASE_INPUT_v:
    srv->cases = g_list_remove (srv->cases, cas);
    INFO ("removed %s %p (%d cases left)", GST_WORKER (cas)->name, cas,
	g_list_length (srv->cases));
    caseport = cas->sink_port;
    g_object_unref (cas);
    for (item = srv->cases; item;) {
      GstCase *c = GST_CASE (item->data);
      if (c->sink_port == caseport) {
	gst_worker_stop (GST_WORKER (c));
	item = g_list_next (item);
	/*
	srv->cases = g_list_remove (srv->cases, c);
	g_object_unref (G_OBJECT (c));
	*/
      } else {
	item = g_list_next (item);
      }
    }
    break;
  }

  GST_SWITCH_SERVER_UNLOCK_CASES (srv);

  if (caseport)
    gst_switch_server_revoke_port (srv, caseport);
}

static void
gst_switch_server_start_case (GstCase *cas, GstSwitchServer *srv)
{
  gboolean is_branch = FALSE;
  switch (cas->type) {
  case GST_CASE_BRANCH_A:
  case GST_CASE_BRANCH_B:
  case GST_CASE_BRANCH_a:
  case GST_CASE_BRANCH_p:
    is_branch = TRUE;
  default:
    break;
  }
  
  if (srv->controller && is_branch) {
    gst_switch_controller_tell_preview_port (srv->controller,
	cas->sink_port, cas->serve_type);

    if (cas->type == GST_CASE_BRANCH_a) {
      gst_switch_controller_tell_audio_port (srv->controller, cas->sink_port);
    }
  }
}

static GstCaseType
gst_switch_server_suggest_case_type (GstSwitchServer *srv,
    GstSwitchServeStreamType serve_type)
{
  GstCaseType type = GST_CASE_UNKNOWN;
  gboolean has_composite_A = FALSE;
  gboolean has_composite_B = FALSE;
  gboolean has_composite_a = FALSE;
  GList *item = srv->cases;

  for (; item; item = g_list_next (item)) {
    GstCase *cas = GST_CASE (item->data);
#if 0
    switch (cas->serve_type) {
    case GST_SERVE_VIDEO_STREAM:
    {
      switch (cas->type) {
      case GST_CASE_COMPOSITE_A: has_composite_A = TRUE; break;
      case GST_CASE_COMPOSITE_B: has_composite_B = TRUE; break;
      default: break;
      }
    } break;
    case GST_SERVE_AUDIO_STREAM:
    {
      if (cas->type == GST_CASE_COMPOSITE_a)
	has_composite_a = TRUE;
    } break;
    case GST_SERVE_NOTHING: break;
    }
#else
    switch (cas->type) {
    case GST_CASE_COMPOSITE_A: has_composite_A = TRUE; break;
    case GST_CASE_COMPOSITE_B: has_composite_B = TRUE; break;
    case GST_CASE_COMPOSITE_a: has_composite_a = TRUE; break;
    default: break;
    }
#endif
    //INFO ("case: %d, %d, %d", cas->sink_port, cas->type, cas->serve_type);
  }

  switch (serve_type) {
  case GST_SERVE_VIDEO_STREAM:
    if (!has_composite_A)	type = GST_CASE_COMPOSITE_A;
    else if (!has_composite_B)	type = GST_CASE_COMPOSITE_B;
    else			type = GST_CASE_PREVIEW;
    break;
  case GST_SERVE_AUDIO_STREAM:
    if (!has_composite_a)	type = GST_CASE_COMPOSITE_a;
    else			type = GST_CASE_PREVIEW;
    break;
  case GST_SERVE_NOTHING: break;
  }

  // TODO: better switching policy?

  return type;
}

static void
gst_switch_server_serve (GstSwitchServer *srv, GSocket *client,
    GstSwitchServeStreamType serve_type)
{
  GSocketInputStreamX *stream = G_SOCKET_INPUT_STREAM (g_object_new (
	  G_TYPE_SOCKET_INPUT_STREAM, "socket", client, NULL));
  GstCaseType type = GST_CASE_UNKNOWN;
  GstCaseType inputtype = GST_CASE_UNKNOWN;
  GstCaseType branchtype = GST_CASE_UNKNOWN;
  gint num_cases = g_list_length (srv->cases);
  GstCase *input = NULL, *branch = NULL, *workcase = NULL;
  gchar *name;
  gint port = 0;
  GCallback start_callback = G_CALLBACK (gst_switch_server_start_case);
  GCallback end_callback = G_CALLBACK (gst_switch_server_end_case);

  GST_SWITCH_SERVER_LOCK_SERVE (srv);
  GST_SWITCH_SERVER_LOCK_CASES (srv);
  switch (serve_type) {
  case GST_SERVE_AUDIO_STREAM: inputtype = GST_CASE_INPUT_a; break;
  case GST_SERVE_VIDEO_STREAM: inputtype = GST_CASE_INPUT_v; break;
  default: goto error_unknown_serve_type;
  }

  type = gst_switch_server_suggest_case_type (srv, serve_type);
  switch (type) {
  case GST_CASE_COMPOSITE_A: branchtype = GST_CASE_BRANCH_A; break;
  case GST_CASE_COMPOSITE_B: branchtype = GST_CASE_BRANCH_B; break;
  case GST_CASE_COMPOSITE_a: branchtype = GST_CASE_BRANCH_a; break;
  case GST_CASE_PREVIEW:     branchtype = GST_CASE_BRANCH_p; break;
  default: goto error_unknown_case_type;
  }

  port = gst_switch_server_alloc_port (srv);

  //INFO ("case-type: %d, %d, %d", type, branchtype, port);

  name = g_strdup_printf ("input_%d", port);
  input = GST_CASE (g_object_new (GST_TYPE_CASE, "name", name,
	  "type", inputtype, "port", port, "serve", serve_type,
	  "stream", stream, NULL));
  g_object_unref (stream);
  g_object_unref (client);
  g_free (name);

  name = g_strdup_printf ("branch_%d", port);
  branch = GST_CASE (g_object_new (GST_TYPE_CASE, "name", name,
	  "type", branchtype, "port", port, "serve", serve_type, NULL));
  g_free (name);

  name = g_strdup_printf ("case-%d", num_cases);
  workcase = GST_CASE (g_object_new (GST_TYPE_CASE, "name", name,
	  "type", type, "port", port, "serve", serve_type,
	  "input", input, "branch", branch, NULL));
  g_free (name);

  srv->cases = g_list_append (srv->cases, input);
  srv->cases = g_list_append (srv->cases, branch);
  srv->cases = g_list_append (srv->cases, workcase);
  GST_SWITCH_SERVER_UNLOCK_CASES (srv);

  if (serve_type == GST_SERVE_VIDEO_STREAM) {
    g_object_set (input,
	"awidth",  srv->composite->a_width,
	"aheight", srv->composite->a_height,
	"bwidth",  srv->composite->b_width,
	"bheight", srv->composite->b_height,
	NULL);
    g_object_set (branch,
	"awidth",  srv->composite->a_width,
	"aheight", srv->composite->a_height,
	"bwidth",  srv->composite->b_width,
	"bheight", srv->composite->b_height,
	NULL);
    g_object_set (workcase,
	"awidth",  srv->composite->a_width,
	"aheight", srv->composite->a_height,
	"bwidth",  srv->composite->b_width,
	"bheight", srv->composite->b_height,
	NULL);
  }

  g_signal_connect (branch, "start-worker", start_callback, srv);
  g_signal_connect (input, "end-worker", end_callback, srv);
  g_signal_connect (branch, "end-worker", end_callback, srv);
  g_signal_connect (workcase, "end-worker", end_callback, srv);

  if (!gst_worker_start (GST_WORKER (input)))
    goto error_start_branch;
  if (!gst_worker_start (GST_WORKER (branch)))
    goto error_start_branch;
  if (!gst_worker_start (GST_WORKER (workcase)))
    goto error_start_workcase;

  GST_SWITCH_SERVER_UNLOCK_SERVE (srv);
  return;

  /* Errors Handling */
 error_unknown_serve_type:
  {
    ERROR ("unknown serve type %d", serve_type);
    g_object_unref (stream);
    g_object_unref (client);
    GST_SWITCH_SERVER_UNLOCK_CASES (srv);
    GST_SWITCH_SERVER_UNLOCK_SERVE (srv);
    return;
  }

 error_unknown_case_type:
  {
    ERROR ("unknown case type (serve type %d)", serve_type);
    g_object_unref (stream);
    g_object_unref (client);
    GST_SWITCH_SERVER_UNLOCK_CASES (srv);
    GST_SWITCH_SERVER_UNLOCK_SERVE (srv);
    return;
  }

 error_start_branch:
 error_start_workcase:
  {
    ERROR ("failed serving new client");
    GST_SWITCH_SERVER_LOCK_CASES (srv);
    srv->cases = g_list_remove (srv->cases, branch);
    srv->cases = g_list_remove (srv->cases, workcase);
    GST_SWITCH_SERVER_UNLOCK_CASES (srv);
    g_object_unref (stream);
    g_object_unref (branch);
    g_object_unref (workcase);
    gst_switch_server_revoke_port (srv, port);
    GST_SWITCH_SERVER_UNLOCK_SERVE (srv);
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

    gst_switch_server_serve (srv, socket, GST_SERVE_VIDEO_STREAM);
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

    gst_switch_server_serve (srv, socket, GST_SERVE_AUDIO_STREAM);
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
  if (srv->controller == NULL) {
    GST_SWITCH_SERVER_LOCK_CONTROLLER (srv);
    if (srv->controller == NULL) {
      srv->controller = GST_SWITCH_CONTROLLER (g_object_new (
	      GST_TYPE_SWITCH_CONTROLLER, NULL));
      srv->controller->server = srv;
    }
    GST_SWITCH_SERVER_UNLOCK_CONTROLLER (srv);
  }
}

gint
gst_switch_server_get_composite_sink_port (GstSwitchServer * srv)
{
  gint port = 0;
  if (srv->composite) {
    //gst_composite_lock (srv->composite);
    port = srv->composite->sink_port;
    //gst_composite_unlock (srv->composite);
  }
  return port;
}

gint
gst_switch_server_get_encode_sink_port (GstSwitchServer * srv)
{
  gint port = 0;
  if (srv->composite) {
    //gst_composite_lock (srv->composite);
    port = srv->composite->encode_sink_port;
    //gst_composite_unlock (srv->composite);
  }
  return port;
}

gint
gst_switch_server_get_audio_sink_port (GstSwitchServer * srv)
{
  gint port = 0;
  GList *item;
  GST_SWITCH_SERVER_LOCK_CASES (srv);
  for (item = srv->cases; item; item = g_list_next (item)) {
    if (GST_CASE (item->data)->type == GST_CASE_COMPOSITE_a) {
      port = GST_CASE (item->data)->sink_port;
      break;
    }
  }
  GST_SWITCH_SERVER_UNLOCK_CASES (srv);
  return port;
}

GArray *
gst_switch_server_get_preview_sink_ports (GstSwitchServer * srv, GArray **t)
{
  GArray *a = g_array_new (FALSE, TRUE, sizeof (gint));
  GList *item;

  if (t) *t = g_array_new (FALSE, TRUE, sizeof (gint));
  
  GST_SWITCH_SERVER_LOCK_CASES (srv);
  for (item = srv->cases; item; item = g_list_next (item)) {
    switch (GST_CASE (item->data)->type) {
    case GST_CASE_BRANCH_A:
    case GST_CASE_BRANCH_B:
    case GST_CASE_BRANCH_a:
    case GST_CASE_PREVIEW:
      a = g_array_append_val (a, GST_CASE (item->data)->sink_port);
      if (t) *t = g_array_append_val (*t, GST_CASE (item->data)->serve_type);
    default: break;
    }
  }
  GST_SWITCH_SERVER_UNLOCK_CASES (srv);
  return a;
}

gboolean
gst_switch_server_set_composite_mode (GstSwitchServer * srv, gint mode)
{
  gboolean result = FALSE;

  GST_SWITCH_SERVER_LOCK_PIP (srv);

  //INFO ("set composite mode: %d", mode);

  if (mode == srv->composite->mode) {
    WARN ("same composite mode");
    goto end;
  }

  g_object_set (srv->composite, "mode", mode, NULL);

  result = (mode == srv->composite->mode);

  if (result) {
    srv->pip_x = srv->composite->b_x;
    srv->pip_y = srv->composite->b_y;
    srv->pip_w = srv->composite->b_width;
    srv->pip_h = srv->composite->b_height;
  }

 end:
  GST_SWITCH_SERVER_UNLOCK_PIP (srv);
  return result;
}

static void
gst_switch_server_start_audio (GstCase *cas, GstSwitchServer *srv)
{
  INFO ("audio %d started", cas->sink_port);
  gst_switch_controller_tell_audio_port (srv->controller, cas->sink_port);
}

gboolean
gst_switch_server_new_record (GstSwitchServer * srv)
{
  g_return_val_if_fail (GST_IS_COMPOSITE (srv->composite), FALSE);

  return gst_composite_new_record (srv->composite);
}

guint
gst_switch_server_adjust_pip (GstSwitchServer * srv,
    gint dx, gint dy, gint dw, gint dh)
{
  guint result = 0;

  g_return_val_if_fail (GST_IS_COMPOSITE (srv->composite), 0);

  GST_SWITCH_SERVER_LOCK_PIP (srv);

  srv->pip_x += dx, srv->pip_y += dy;
  srv->pip_w += dw, srv->pip_h += dh;
  if (srv->pip_x < 0) srv->pip_x = 0;
  if (srv->pip_y < 0) srv->pip_y = 0;
  if (srv->pip_w < GST_SWITCH_COMPOSITE_MIN_PIP_W)
    srv->pip_w   = GST_SWITCH_COMPOSITE_MIN_PIP_W;
  if (srv->pip_h < GST_SWITCH_COMPOSITE_MIN_PIP_H)
    srv->pip_h   = GST_SWITCH_COMPOSITE_MIN_PIP_H;

  result = gst_composite_adjust_pip (srv->composite,
      srv->pip_x, srv->pip_y, srv->pip_w, srv->pip_h);

  if (dx != 0) result |= (1 << 0);
  if (dy != 0) result |= (1 << 1);
  if (dw != 0) result |= (1 << 2);
  if (dh != 0) result |= (1 << 3);

  GST_SWITCH_SERVER_UNLOCK_PIP (srv);
  return result;
}

gboolean
gst_switch_server_switch (GstSwitchServer * srv, gint channel, gint port)
{
  GList *item;
  gboolean result = FALSE;
  GstCase *compose_case, *candidate_case;
  GstCase *work1, *work2;
  GCallback callback = G_CALLBACK (gst_switch_server_end_case);
  gchar *name;

  compose_case = NULL;
  candidate_case = NULL;

  GST_SWITCH_SERVER_LOCK_CASES (srv);

  for (item = srv->cases; item; item = g_list_next (item)) {
    GstCase *cas = GST_CASE (item->data);
    switch (channel) {
    case 'A':
      if (cas->type == GST_CASE_COMPOSITE_A) goto get_target_stream;
      break;      
    case 'B':
      if (cas->type == GST_CASE_COMPOSITE_B) goto get_target_stream;
      break;
    case 'a':
      if (cas->type == GST_CASE_COMPOSITE_a) goto get_target_stream;
      break;
    default:
      WARN ("unknown channel %c", (gchar) channel);
      break;
    get_target_stream:
      if (compose_case == NULL) {
	compose_case = cas;
      }
    }
    switch (cas->type) {
    case GST_CASE_COMPOSITE_A:
    case GST_CASE_COMPOSITE_B:
    case GST_CASE_COMPOSITE_a:
    case GST_CASE_PREVIEW:
      if (cas->sink_port == port) {
	candidate_case = cas;
      }
    default:
      break;
    }
  }

  if (!candidate_case) {
    ERROR ("no stream for port %d", port);
    goto end;
  }

  if (candidate_case == compose_case) {
    ERROR ("stream on %d already at %c", port, (gchar) channel);
    goto end;
  }

  INFO ("switching: %s (%d), %s (%d)",
      GST_WORKER (compose_case)->name, compose_case->type,
      GST_WORKER (candidate_case)->name, candidate_case->type);

  if (candidate_case->serve_type != compose_case->serve_type) {
    ERROR ("stream type not matched");
    goto end;
  }

  name = g_strdup (GST_WORKER (compose_case)->name);
  work1 = GST_CASE (g_object_new (GST_TYPE_CASE, "name", name,
	  "type",    compose_case->type,
	  "serve",   compose_case->serve_type,
	  "port",    candidate_case->sink_port,
	  "input",   candidate_case->input,
	  "branch",  candidate_case->branch,
	  NULL));
  g_free (name);

  name = g_strdup (GST_WORKER (candidate_case)->name);
  work2 = GST_CASE (g_object_new (GST_TYPE_CASE, "name", name,
	  "type",    candidate_case->type,
	  "serve",   candidate_case->serve_type,
	  "port",    compose_case->sink_port,
	  "input",   compose_case->input,
	  "branch",  compose_case->branch,
	  NULL));
  g_free (name);

  if (compose_case->serve_type == GST_SERVE_VIDEO_STREAM) {
    g_object_set (work1,
	"awidth",  compose_case->a_width,
	"aheight", compose_case->a_height,
	"bwidth",  compose_case->b_width,
	"bheight", compose_case->b_height,
	NULL);
    g_object_set (work2,
	"awidth",  candidate_case->a_width,
	"aheight", candidate_case->a_height,
	"bwidth",  candidate_case->b_width,
	"bheight", candidate_case->b_height,
	NULL);
  } else {
    g_signal_connect (G_OBJECT (work1), "start-worker",
	G_CALLBACK (gst_switch_server_start_audio), srv);
  }

  compose_case->switching = TRUE;
  candidate_case->switching = TRUE;
  gst_worker_stop (GST_WORKER (compose_case));
  gst_worker_stop (GST_WORKER (candidate_case));

  g_signal_connect (work1, "end-worker", callback, srv);
  g_signal_connect (work2, "end-worker", callback, srv);

  if (!gst_worker_start (GST_WORKER (work1)))
    goto error_start_work;
  if (!gst_worker_start (GST_WORKER (work2)))
    goto error_start_work;

  srv->cases = g_list_append (srv->cases, work1);
  srv->cases = g_list_append (srv->cases, work2);

  result = TRUE;

  INFO ("switched: %s <-> %s",
      GST_WORKER (work1)->name, GST_WORKER (work2)->name);

 end:
  GST_SWITCH_SERVER_UNLOCK_CASES (srv);
  return result;

 error_start_work:
  {
    ERROR ("failed to start works");
    g_object_unref (work1);
    g_object_unref (work2);
    GST_SWITCH_SERVER_UNLOCK_CASES (srv);
    return result;
  }
}

static void
gst_switch_server_start_output (GstWorker *worker, GstSwitchServer * srv)
{
  g_return_if_fail (GST_IS_WORKER (worker));

  GST_SWITCH_SERVER_LOCK_CONTROLLER (srv);
  if (srv->controller) {
    gst_switch_controller_tell_compose_port (srv->controller,
	srv->composite->sink_port);
  }
  GST_SWITCH_SERVER_UNLOCK_CONTROLLER (srv);
}

static void
gst_switch_server_start_recorder (GstWorker *worker, GstSwitchServer * srv)
{
  g_return_if_fail (GST_IS_WORKER (worker));

  GST_SWITCH_SERVER_LOCK_CONTROLLER (srv);
  if (srv->controller) {
#if 0
    gst_switch_controller_tell_encode_port (srv->controller,
	srv->composite->encode_sink_port);
#endif
  }
  GST_SWITCH_SERVER_UNLOCK_CONTROLLER (srv);
}

static gboolean
gst_switch_server_prepare_composite (GstSwitchServer * srv,
    GstCompositeMode mode)
{
  gint port, encode;

  if (srv->composite) {
    return TRUE;
  }

  port = gst_switch_server_alloc_port (srv);
  encode = gst_switch_server_alloc_port (srv);

  INFO ("Compose sink to %d, %d", port, encode);

  srv->composite = GST_COMPOSITE (g_object_new (GST_TYPE_COMPOSITE,
	  "name", "composite", "port", port, "encode", encode,
	  "mode", mode, NULL));

  g_signal_connect (srv->composite, "start-output",
      G_CALLBACK (gst_switch_server_start_output), srv);
  g_signal_connect (srv->composite, "start-recorder",
      G_CALLBACK (gst_switch_server_start_recorder), srv);

  GST_SWITCH_SERVER_LOCK_PIP (srv);
  srv->pip_x = srv->composite->b_x;
  srv->pip_y = srv->composite->b_y;
  srv->pip_w = srv->composite->b_width;
  srv->pip_h = srv->composite->b_height;
  GST_SWITCH_SERVER_UNLOCK_PIP (srv);

  if (!gst_worker_start (GST_WORKER (srv->composite)))
    goto error_start_composite;

  return TRUE;

 error_start_composite:
  {
    g_object_unref (srv->composite);
    srv->composite = NULL;
    return FALSE;
  }
}

static void
gst_switch_server_run (GstSwitchServer * srv)
{
  srv->main_loop = g_main_loop_new (NULL, TRUE);

  if (!gst_switch_server_prepare_composite (srv, DEFAULT_COMPOSE_MODE))
    goto error_prepare;

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
 error_prepare:
  {
    ERROR ("error preparing server");
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
