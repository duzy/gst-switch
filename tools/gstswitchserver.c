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
#define GST_SWITCH_SERVER_LOCK_COMPOSITE(srv) (g_mutex_lock (&(srv)->composite_lock))
#define GST_SWITCH_SERVER_UNLOCK_COMPOSITE(srv) (g_mutex_unlock (&(srv)->composite_lock))
#define GST_SWITCH_SERVER_LOCK_RECORDER(srv) (g_mutex_lock (&(srv)->recorder_lock))
#define GST_SWITCH_SERVER_UNLOCK_RECORDER(srv) (g_mutex_unlock (&(srv)->recorder_lock))
#define GST_SWITCH_SERVER_LOCK_CASES(srv) (g_mutex_lock (&(srv)->cases_lock))
#define GST_SWITCH_SERVER_UNLOCK_CASES(srv) (g_mutex_unlock (&(srv)->cases_lock))
#define GST_SWITCH_SERVER_LOCK_SERVE(srv) (g_mutex_lock (&(srv)->serve_lock))
#define GST_SWITCH_SERVER_UNLOCK_SERVE(srv) (g_mutex_unlock (&(srv)->serve_lock))

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
  srv->recorder = NULL;
  srv->alloc_port_count = 0;

  g_mutex_init (&srv->video_acceptor_lock);
  g_mutex_init (&srv->audio_acceptor_lock);
  g_mutex_init (&srv->controller_lock);
  g_mutex_init (&srv->serve_lock);
  g_mutex_init (&srv->cases_lock);
  g_mutex_init (&srv->alloc_port_lock);
  g_mutex_init (&srv->composite_lock);
  g_mutex_init (&srv->recorder_lock);
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

  if (srv->recorder) {
    g_object_unref (srv->recorder);
    srv->recorder = NULL;
  }

  g_mutex_clear (&srv->video_acceptor_lock);
  g_mutex_clear (&srv->audio_acceptor_lock);
  g_mutex_clear (&srv->controller_lock);
  g_mutex_clear (&srv->serve_lock);
  g_mutex_clear (&srv->cases_lock);
  g_mutex_clear (&srv->alloc_port_lock);
  g_mutex_clear (&srv->composite_lock);
  g_mutex_clear (&srv->recorder_lock);

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
  GstState state = GST_STATE_VOID_PENDING;

  GST_SWITCH_SERVER_LOCK_CASES (srv);
  gst_element_get_state (GST_WORKER (cas)->pipeline, &state, NULL,
      GST_CLOCK_TIME_NONE);
  if (state == GST_STATE_NULL && !cas->switching) {
    caseport = cas->sink_port;
    g_object_unref (cas);
    srv->cases = g_list_remove (srv->cases, cas);
    for (item = srv->cases; item;) {
      GstCase *c = GST_CASE (item->data);
      switch (c->type) {
      case GST_CASE_COMPOSITE_A:
      case GST_CASE_COMPOSITE_B:
      case GST_CASE_COMPOSITE_a:
      case GST_CASE_BRANCH_A:
      case GST_CASE_BRANCH_B:
      case GST_CASE_BRANCH_a:
      case GST_CASE_PREVIEW:
	if (c->sink_port == caseport) {
	  gst_worker_stop (GST_WORKER (c));
	  g_object_unref (G_OBJECT (c));
	  item = g_list_next (item);
	  srv->cases = g_list_remove (srv->cases, c);
	} else {
	  item = g_list_next (item);
	}
	break;
      default:
	item = g_list_next (item);
	break;
      }
    }
  }
  GST_SWITCH_SERVER_UNLOCK_CASES (srv);

  if (caseport)
    gst_switch_server_revoke_port (srv, caseport);

  INFO ("removed %s %p (%d cases left)", GST_WORKER (cas)->name, cas,
      g_list_length (srv->cases));
}

static void
gst_switch_server_end_composite (GstComposite *composite, GstSwitchServer *srv)
{
  INFO ("the composite has ended");
}

static void
gst_switch_server_end_recorder (GstRecorder *recorder, GstSwitchServer *srv)
{
  GST_SWITCH_SERVER_LOCK_RECORDER (srv);
  g_object_unref (srv->recorder);
  srv->recorder = NULL;
  GST_SWITCH_SERVER_UNLOCK_RECORDER (srv);
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
    INFO ("case: %d, %d, %d", cas->sink_port, cas->type, cas->serve_type);
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
  GstCaseType branchtype = GST_CASE_UNKNOWN;
  GstCaseType type = GST_CASE_UNKNOWN;
  gint num_cases = g_list_length (srv->cases);
  GstCase *branch = NULL, *workcase;
  gchar *name;
  gint port = 0;

  GST_SWITCH_SERVER_LOCK_CASES (srv);
  type = gst_switch_server_suggest_case_type (srv, serve_type);

  switch (type) {
  case GST_CASE_COMPOSITE_A: branchtype = GST_CASE_BRANCH_A; break;
  case GST_CASE_COMPOSITE_B: branchtype = GST_CASE_BRANCH_B; break;
  case GST_CASE_COMPOSITE_a: branchtype = GST_CASE_BRANCH_a; break;
  case GST_CASE_PREVIEW:     branchtype = GST_CASE_BRANCH_p; break;
  default: goto error_unknown_case_type;
  }

  port = gst_switch_server_alloc_port (srv);

  INFO ("case-type: %d, %d, %d", type, branchtype, port);

  name = g_strdup_printf ("branch_%d", port);
  branch = GST_CASE (g_object_new (GST_TYPE_CASE, "name", name,
	  "type", branchtype, "port", port, "serve", serve_type, NULL));
  g_free (name);

  name = g_strdup_printf ("case-%d", num_cases);
  workcase = GST_CASE (g_object_new (GST_TYPE_CASE, "name", name,
	  "type", type, "port", port, "stream", stream, "branch", branch,
	  "serve", serve_type, NULL));
  g_object_unref (client);
  g_object_unref (stream);
  g_free (name);

  srv->cases = g_list_append (srv->cases, branch);
  srv->cases = g_list_append (srv->cases, workcase);
  GST_SWITCH_SERVER_UNLOCK_CASES (srv);

  if (serve_type == GST_SERVE_VIDEO_STREAM) {
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

  if (!gst_worker_prepare (GST_WORKER (branch)))
    goto error_prepare_branch;
  if (!gst_worker_prepare (GST_WORKER (workcase)))
    goto error_prepare_workcase;

  g_signal_connect (workcase, "end-worker",
      G_CALLBACK (gst_switch_server_end_case), srv);

  gst_worker_start (GST_WORKER (branch));
  gst_worker_start (GST_WORKER (workcase));

  switch (type) {
  case GST_CASE_COMPOSITE_A:
  case GST_CASE_COMPOSITE_B:
    gst_switch_controller_tell_compose_port (srv->controller,
	srv->composite->sink_port);
    goto preview;
  case GST_CASE_COMPOSITE_a:
    gst_switch_controller_tell_audio_port (srv->controller, port);
    /* fallthrough */
  preview:
  case GST_CASE_PREVIEW:
    gst_switch_controller_tell_preview_port (srv->controller,
	port, serve_type);
    break;
  default:
    ERROR ("unknown case");
    break;
  }
  return;

  /* Errors Handling */

 error_unknown_case_type:
  {
    ERROR ("unknown case type (serve type %d)", serve_type);
    g_object_unref (stream);
    return;
  }

 error_prepare_branch:
 error_prepare_workcase:
  {
    ERROR ("can't serve new client");
    GST_SWITCH_SERVER_LOCK_CASES (srv);
    srv->cases = g_list_remove (srv->cases, branch);
    srv->cases = g_list_remove (srv->cases, workcase);
    GST_SWITCH_SERVER_UNLOCK_CASES (srv);
    g_object_unref (stream);
    g_object_unref (branch);
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

    GST_SWITCH_SERVER_LOCK_SERVE (srv);
    gst_switch_server_serve (srv, socket, GST_SERVE_VIDEO_STREAM);
    GST_SWITCH_SERVER_UNLOCK_SERVE (srv);
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

    GST_SWITCH_SERVER_LOCK_SERVE (srv);
    gst_switch_server_serve (srv, socket, GST_SERVE_AUDIO_STREAM);
    GST_SWITCH_SERVER_UNLOCK_SERVE (srv);
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

gint
gst_switch_server_get_encode_sink_port (GstSwitchServer * srv)
{
  gint port = 0;
  if (srv->recorder) {
    GST_SWITCH_SERVER_LOCK_RECORDER (srv);
    port = srv->recorder->sink_port;
    GST_SWITCH_SERVER_UNLOCK_RECORDER (srv);
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
gst_switch_server_switch (GstSwitchServer * srv, gint channel, gint port)
{
  GList *item;
  GstCase *compose_case, *candidate_case;
  gboolean result = FALSE;

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
    if (cas->sink_port == port /*&& cas->type == GST_CASE_PREVIEW*/) {
      candidate_case = cas;
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

  GInputStream *stream1 = NULL, *stream2 = NULL;
  //GSocket *input1 = NULL, *input2 = NULL;
  INFO ("switching: %s (%d), %s (%d)",
      GST_WORKER (compose_case)->name, compose_case->type,
      GST_WORKER (candidate_case)->name, candidate_case->type);

  if (candidate_case->serve_type != compose_case->serve_type) {
    ERROR ("stream type not matched");
    goto end;
  }
      
  /*
    for (item = srv->cases; item; item = g_list_next (item)) {
    GstCase *cas = GST_CASE (item->data);
    if (cas->stream == candidate_case) {
    gst_worker_stop (GST_WORKER (cas));
    } else if (cas->stream == compose_case) {
    gst_worker_stop (GST_WORKER (cas));
    }
    }

    for (item = srv->cases; item; item = g_list_next (item)) {
    GstCase *cas = GST_CASE (item->data);
    if (cas->stream == candidate_case) {
    g_object_set (G_OBJECT (cas), "stream", compose_case, NULL);
    gst_worker_restart (GST_WORKER (cas));
    } else if (cas->stream == compose_case) {
    g_object_set (G_OBJECT (cas), "stream", candidate_case, NULL);
    gst_worker_restart (GST_WORKER (cas));
    }
    }
  */

  compose_case->switching = TRUE;
  candidate_case->switching = TRUE;

  /*
    gst_worker_pause (GST_WORKER (compose_case));
    gst_worker_pause (GST_WORKER (candidate_case));
    stream1 = compose_case->stream;
    stream2 = candidate_case->stream;
    g_object_get (G_OBJECT (stream1), "socket", &input1, NULL);
    g_object_get (G_OBJECT (stream2), "socket", &input2, NULL);
    g_object_set (G_OBJECT (stream1), "socket", input2, NULL);
    g_object_set (G_OBJECT (stream2), "socket", input1, NULL);
    gst_worker_resume (GST_WORKER (compose_case));
    gst_worker_resume (GST_WORKER (candidate_case));
  */
  gint compose_port = compose_case->sink_port;
  gint compose_type = compose_case->type;
  gint compose_serve_type = compose_case->serve_type;
  gint candidate_port = candidate_case->sink_port;
  gint candidate_type = candidate_case->type;
  gint candidate_serve_type = candidate_case->serve_type;
  stream1 = compose_case->stream;
  stream2 = candidate_case->stream;
  g_object_ref (stream1);
  g_object_ref (stream2);
  gst_worker_stop (GST_WORKER (compose_case));
  gst_worker_stop (GST_WORKER (candidate_case));

  gint num_cases = g_list_length (srv->cases);
  gchar *name1 = g_strdup_printf ("case-%d", num_cases);
  gchar *name2 = g_strdup_printf ("case-%d", num_cases + 1);
  GstCase *work1 = GST_CASE (g_object_new (GST_TYPE_CASE, "name", name1,
	  "type", compose_type, "port", compose_port, "stream", stream2,
	  "serve", compose_serve_type, NULL));
  GstCase *work2 = GST_CASE (g_object_new (GST_TYPE_CASE, "name", name2,
	  "type", candidate_type, "port", candidate_port,
	  "stream", stream2, "serve", candidate_serve_type, NULL));
  g_free (name1);
  g_free (name2);
  g_object_unref (stream1);
  g_object_unref (stream2);

  gst_worker_prepare (GST_WORKER (work1));
  gst_worker_prepare (GST_WORKER (work2));

  g_signal_connect (work1, "end-worker",
      G_CALLBACK (gst_switch_server_end_case), srv);
  g_signal_connect (work2, "end-worker",
      G_CALLBACK (gst_switch_server_end_case), srv);

  gst_worker_start (GST_WORKER (work1));
  gst_worker_start (GST_WORKER (work2));

  srv->cases = g_list_append (srv->cases, work1);
  srv->cases = g_list_append (srv->cases, work2);

  //compose_case->switching = FALSE;
  //candidate_case->switching = FALSE;

  result = TRUE;
  INFO ("switched: %s -> %s, %s -> %s",
      GST_WORKER (compose_case)->name, GST_WORKER (work1)->name,
      GST_WORKER (candidate_case)->name, GST_WORKER (work2)->name);

 end:
  GST_SWITCH_SERVER_UNLOCK_CASES (srv);
  return result;
}

static void
gst_switch_server_create_composite (GstSwitchServer * srv)
{
  gint port = gst_switch_server_alloc_port (srv);

  INFO ("Compose sink to %d", port);

  GST_SWITCH_SERVER_LOCK_COMPOSITE (srv);
  srv->composite = GST_COMPOSITE (g_object_new (GST_TYPE_COMPOSITE,
	  "name", "composite", "port", port, NULL));
  GST_SWITCH_SERVER_UNLOCK_COMPOSITE (srv);
}

static void
gst_switch_server_create_recorder (GstSwitchServer * srv)
{
  gint port = gst_switch_server_alloc_port (srv);
  guint w, h;

  g_object_get (srv->composite, "width", &w, "height", &h, NULL);

  INFO ("Recorder sink to %d (%dx%d)", port, w, h);

  GST_SWITCH_SERVER_LOCK_RECORDER (srv);
  srv->recorder = GST_RECORDER (g_object_new (GST_TYPE_RECORDER,
	  "name", "recorder", "port", port, "width", w, "height", h, NULL));
  GST_SWITCH_SERVER_UNLOCK_RECORDER (srv);
}

static gboolean
gst_switch_server_launch_recorder (GstSwitchServer * srv)
{
  if (!gst_worker_prepare (GST_WORKER (srv->recorder)))
    return FALSE;

  g_signal_connect (srv->recorder, "end-worker",
      G_CALLBACK (gst_switch_server_end_recorder), srv);

  gst_worker_start (GST_WORKER (srv->recorder));
  return TRUE;
}

static gboolean
gst_switch_server_launch_composite (GstSwitchServer * srv)
{
  if (!gst_worker_prepare (GST_WORKER (srv->composite)))
    return FALSE;

  g_signal_connect (srv->composite, "end-worker",
      G_CALLBACK (gst_switch_server_end_composite), srv);

  gst_worker_start (GST_WORKER (srv->composite));
  return TRUE;
}

static gboolean
gst_switch_server_prepare (GstSwitchServer * srv)
{
  gst_switch_server_create_composite (srv);
  gst_switch_server_create_recorder (srv);

  if (!gst_switch_server_launch_recorder (srv)) {
    ERROR ("failed to prepare recorder");
    goto error_launch;
  }

  if (!gst_switch_server_launch_composite (srv)) {
    ERROR ("failed to prepare composite");
    goto error_launch;
  }

  return TRUE;

 error_launch:
  if (srv->recorder) {
    GST_SWITCH_SERVER_LOCK_RECORDER (srv);
    g_object_unref (srv->recorder);
    srv->recorder = NULL;
    GST_SWITCH_SERVER_UNLOCK_RECORDER (srv);
  }
  if (srv->composite) {
    GST_SWITCH_SERVER_LOCK_COMPOSITE (srv);
    g_object_unref (srv->composite);
    srv->composite = NULL;
    GST_SWITCH_SERVER_UNLOCK_COMPOSITE (srv);
  }
  return FALSE;
}

static void
gst_switch_server_run (GstSwitchServer * srv)
{
  srv->main_loop = g_main_loop_new (NULL, TRUE);

  if (!gst_switch_server_prepare (srv))
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
