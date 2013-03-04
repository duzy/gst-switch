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

#ifndef __GST_SWITCH_SERVER_H__by_Duzy_Chan__
#define __GST_SWITCH_SERVER_H__by_Duzy_Chan__ 1
#include <gio/gio.h>
#include "gstcomposite.h"
#include "gstswitchcontroller.h"
#include "../logutils.h"

#define GST_TYPE_SWITCH_SERVER (gst_switch_server_get_type())
#define GST_SWITCH_SERVER(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), GST_TYPE_SWITCH_SERVER, GstSwitchServer))
#define GST_SWITCH_SERVER_CLASS(class) (G_TYPE_CHECK_CLASS_CAST ((class), GST_TYPE_SWITCH_SERVER, GstSwitchServerClass))
#define GST_IS_SWITCH_SERVER(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), GST_TYPE_SWITCH_SERVER))
#define GST_IS_SWITCH_SERVER_CLASS(class) (G_TYPE_CHECK_CLASS_TYPE ((class), GST_TYPE_SWITCH_SERVER))

#define GST_SWITCH_MIN_SINK_PORT 1
#define GST_SWITCH_MAX_SINK_PORT 65535

typedef struct _GstRecorder GstRecorder;
typedef struct _GstSwitchServerClass GstSwitchServerClass;
typedef struct _GstSwitchServerOpts GstSwitchServerOpts;

/**
 *  GstSwitchServerOpts:
 *  @record_filename: the recording filename
 *  @controller_address: the dbus address for the controller
 *  @video_input_port: the video input TCP port
 *  @audio_input_port: the audio input TCP port
 *  @control_port: (discarded)
 */
struct _GstSwitchServerOpts
{
  gchar *test_switch;
  gchar *record_filename;
  gchar *controller_address;
  gint video_input_port;
  gint audio_input_port;
  gint control_port;
};

/**
 *  GstSwitchServer:
 *  @base: the parent object
 *  @host: the server host name, e.g. localhost
 *  @main_loop: the main loop
 *  @main_loop_lock: the lock for the @main_loop
 *  @exit_code: the exit code in cases of force quit.
 *  @cancellable: 
 *  @video_acceptor_lock: the lock for the video acceptor
 *  @video_acceptor: the video acceptor thread
 *  @video_acceptor_socket: the vidoe acceptor socekt
 *  @video_acceptor_port: the video acceptor port number
 *  @audio_acceptor_lock: the lock for the audio acceptor
 *  @audio_acceptor: the audio acceptor thread
 *  @audio_acceptor_socket: the audio acceptor socket
 *  @audio_acceptor_port: the audio acceptor port
 *  @controller_lock: the lock for controller
 *  @controller_thread: the controller thread (deprecated)
 *  @controller_socket: the controller socket (deprecated)
 *  @controller_port: the controller port number (deprecated)
 *  @controller: the controller instance
 *  @alloc_port_lock: the lock for @alloc_port_count
 *  @alloc_port_count: port allocation counter
 *  @serve_lock: the lock for serving new inputs
 *  @cases_lock: the lock for the @cases
 *  @cases: the case list
 *  @composite: the composite instance
 *  @new_composite_mode: the new composite mode to be applied
 *  @output: the output instance
 *  @recorder_lock: the lock for the @recorder
 *  @recorder: the recorder instance
 *  @pip_lock the lock for PIP
 *  @pip_x: the PIP x position
 *  @pip_y: the PIP y position
 *  @pip_w: the PIP width
 *  @pip_h: the PIP height
 *  @clock_lock: the lock for @clock
 *  @clock: a system clock
 */
struct _GstSwitchServer
{
  GObject base;

  gchar *host;

  GMainLoop *main_loop;
  GMutex main_loop_lock;
  gint exit_code;

  GCancellable *cancellable;
  GMutex video_acceptor_lock;
  GThread *video_acceptor;
  GSocket *video_acceptor_socket;
  gint video_acceptor_port;

  GMutex audio_acceptor_lock;
  GThread *audio_acceptor;
  GSocket *audio_acceptor_socket;
  gint audio_acceptor_port;

  GMutex controller_lock;
  GThread *controller_thread;
  GSocket *controller_socket;
  gint controller_port;
  GstSwitchController *controller;

  GMutex alloc_port_lock;
  gint alloc_port_count;

  GMutex serve_lock;
  GMutex cases_lock;
  GList *cases;

  GstComposite *composite;
  GstCompositeMode new_composite_mode;

  GstWorker *output;

  GMutex recorder_lock;
  GstRecorder *recorder;

  GMutex pip_lock;
  gint pip_x, pip_y, pip_w, pip_h;

  GMutex clock_lock;
  GstClock *clock;
};

/**
 *  GstSwitchServerClass:
 *  @base_calss: the parent class
 */
struct _GstSwitchServerClass
{
  GObjectClass base_class;
};

GType gst_switch_server_get_type (void);

/**
 *  gst_switch_server_get_composite_sink_port:
 *  @srv: the GstSwitchServer instance
 *
 *  Get the composite port.
 *  
 *  @return: the composite sink port
 */
gint gst_switch_server_get_composite_sink_port (GstSwitchServer * srv);

/**
 *  gst_switch_server_get_encode_sink_port:
 *  @srv: the GstSwitchServer instance
 *
 *  Get the encode port.
 *
 *  @return: the encode sink port number
 */
gint gst_switch_server_get_encode_sink_port (GstSwitchServer * srv);

/**
 *  gst_switch_server_get_audio_sink_port:
 *  @srv: the GstSwitchServer instance
 *
 *  Get the audio port.
 *
 *  @return: the audio sink port number.
 */
gint gst_switch_server_get_audio_sink_port (GstSwitchServer * srv);

/**
 *  gst_switch_server_get_preview_sink_ports:
 *  @serves: (output) the preview serve types.
 *  @types: (output) the preview types.
 *
 *  Get the preview ports.
 *
 *  @return: The array of preview ports.
 */
GArray *gst_switch_server_get_preview_sink_ports (GstSwitchServer * srv,
    GArray ** serves, GArray ** types);

/**
 *  gst_switch_server_set_composite_mode:
 *
 *  Change a composite mode.
 *
 *  @return: TRUE if succeeded.
 */
gboolean gst_switch_server_set_composite_mode (GstSwitchServer * srv,
    gint mode);

/**
 *  gst_switch_server_switch:
 *
 *  Switch the channel to the specific port.
 *
 *  @return: TRUE if succeeded.
 */
gboolean gst_switch_server_switch (GstSwitchServer * srv, gint channel,
    gint port);

/**
 *  gst_switch_server_adjust_pip:
 *
 *  Adjust the PIP position and size.
 *
 *  @return: a unsigned number of indicating which compononent (x,y,w,h) has
 *           been changed
 */
guint gst_switch_server_adjust_pip (GstSwitchServer * srv, gint dx, gint dy,
    gint dw, gint dh);

/**
 *  gst_switch_server_new_record:
 *
 *  Start a new recording.
 *  
 *  @return: TRUE if succeeded.
 */
gboolean gst_switch_server_new_record (GstSwitchServer * srv);

extern GstSwitchServerOpts opts;

#endif //__GST_SWITCH_SERVER_H__by_Duzy_Chan__
