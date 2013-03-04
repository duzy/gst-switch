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

#ifndef __GST_SWITCH_CLIENT_H__by_Duzy_Chan__
#define __GST_SWITCH_CLIENT_H__by_Duzy_Chan__ 1
#include <gst/gst.h>
#include <gio/gio.h>

#define GST_TYPE_SWITCH_CLIENT (gst_switch_client_get_type ())
#define GST_SWITCH_CLIENT(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), GST_TYPE_SWITCH_CLIENT, GstSwitchClient))
#define GST_SWITCH_CLIENT_CLASS(class) (G_TYPE_CHECK_CLASS_CAST ((class), GST_TYPE_SWITCH_CLIENT, GstSwitchClientClass))
#define GST_IS_SWITCH_CLIENT(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), GST_TYPE_SWITCH_CLIENT))
#define GST_IS_SWITCH_CLIENT_CLASS(class) (G_TYPE_CHECK_CLASS_TYPE ((class), GST_TYPE_SWITCH_CLIENT))

typedef struct _GstSwitchClient GstSwitchClient;
typedef struct _GstSwitchClientClass GstSwitchClientClass;

typedef void (*GstSwitchClientConnectionClosedFunc) (GstSwitchClient * client,
    GError * error);
typedef void (*GstSwitchClientSetComposePortFunc) (GstSwitchClient * client,
    gint port);
typedef void (*GstSwitchClientSetEncodePortFunc) (GstSwitchClient * client,
    gint port);
typedef void (*GstSwitchClientSetAudioPortFunc) (GstSwitchClient * client,
    gint port);
typedef void (*GstSwitchClientAddPreviewPortFunc) (GstSwitchClient * client,
    gint port, gint serve, gint type);
typedef void (*GstSwitchClientNewModeOnlineFunc) (GstSwitchClient * client,
    gint port);

/**
 *  GstSwitchClient:
 *  
 */
struct _GstSwitchClient
{
  GObject base;

  GMutex controller_lock;
  GDBusConnection *controller;

  GMutex composite_mode_lock;
  gboolean changing_composite_mode;
};

/**
 *  GstSwitchClientClass:
 *  
 */
struct _GstSwitchClientClass
{
  GObjectClass base_class;

  GHashTable *methods;

  void (*connection_closed) (GstSwitchClient * client, GError * error);
  void (*set_audio_port) (GstSwitchClient * client, gint port);
  void (*set_compose_port) (GstSwitchClient * client, gint port);
  void (*set_encode_port) (GstSwitchClient * client, gint port);
  void (*add_preview_port) (GstSwitchClient * client, gint port, gint serve,
      gint type);
  void (*new_mode_online) (GstSwitchClient * client, gint mode);
};

GType gst_switch_client_get_type (void);

/**
 *  gst_switch_client_is_connected:
 *  @client: the GstSwitchClient instance
 *
 *  Check if the client is connected to the gst-switch server.
 *  
 *  @return: TRUE when requested.
 */
gboolean gst_switch_client_is_connected (GstSwitchClient * client);

/**
 *  gst_switch_client_connect:
 *  @client: the GstSwitchClient instance
 *
 *  Connect the client with the gst-switch server.
 *  
 *  @return: TRUE when requested.
 */
gboolean gst_switch_client_connect (GstSwitchClient * client);

/**
 *  gst_switch_client_get_compose_port:
 *  @client: the GstSwitchClient instance
 *
 *  Get the compose port number.
 *  
 *  @return: the compose port number
 */
gint gst_switch_client_get_compose_port (GstSwitchClient * client);

/**
 *  gst_switch_client_get_encode_port:
 *  @client: the GstSwitchClient instance
 *
 *  Get the encode port number.
 *  
 *  @return: the encode port number
 */
gint gst_switch_client_get_encode_port (GstSwitchClient * client);

/**
 *  gst_switch_client_get_audio_port:
 *  @client: the GstSwitchClient instance
 *
 *  The the audio port number.
 *  
 *  @return: the audio port number
 */
gint gst_switch_client_get_audio_port (GstSwitchClient * client);

/**
 *  gst_switch_client_get_preview_ports:
 *  @client: the GstSwitchClient instance
 *
 *  The all preview ports.
 *
 *  @return: The preview ports of type GVariant
 */
GVariant *gst_switch_client_get_preview_ports (GstSwitchClient * client);

/**
 *  gst_switch_client_switch:
 *  @client: the GstSwitchClient instance
 *  @channel: The channel to be switched, 'A', 'B', 'a'
 *  @port: The target port number
 *
 *  Switch the channel to the target port.
 *  
 *  @return: TRUE when requested.
 */
gboolean gst_switch_client_switch (GstSwitchClient * client, gint channel,
    gint port);

/**
 *  gst_switch_client_set_composite_mode:
 *  @client: the GstSwitchClient instance
 *
 *  Set the current composite mode.
 *  
 *  @return: TRUE when requested.
 */
gboolean gst_switch_client_set_composite_mode (GstSwitchClient * client,
    gint mode);

/**
 *  gst_switch_client_new_record:
 *  @client: the GstSwitchClient instance
 *
 *  Start a new recording.
 *
 *  @return: TRUE when requested.
 */
gboolean gst_switch_client_new_record (GstSwitchClient * client);

/**
 *  gst_switch_client_adjust_pip:
 *  @client: the GstSwitchClient instance
 *  @dx: x position to be adjusted
 *  @dy: y position to be adjusted
 *  @dw: w position to be adjusted
 *  @dh: h position to be adjusted
 *
 *  Adjust the PIP.
 *
 *  @return: a unsign integer indicating which components are changed of per
 *           bit.
 */
guint gst_switch_client_adjust_pip (GstSwitchClient * client, gint dx,
    gint dy, gint dw, gint dh);

#endif //__GST_SWITCH_CLIENT_H__by_Duzy_Chan__
