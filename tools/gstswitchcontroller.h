/* GstSwitch
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

#ifndef __GST_SWITCH_CONTROLLER_H__by_Duzy_Chan__
#define __GST_SWITCH_CONTROLLER_H__by_Duzy_Chan__ 1
#include <gio/gio.h>
#include "../logutils.h"

#define GST_TYPE_SWITCH_CONTROLLER (gst_switch_controller_get_type ())
#define GST_SWITCH_CONTROLLER(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), GST_TYPE_SWITCH_CONTROLLER, GstSwitchController))
#define GST_SWITCH_CONTROLLER_CLASS(class) (G_TYPE_CHECK_CLASS_CAST ((class), GST_TYPE_SWITCH_CONTROLLER, GstSwitchControllerClass))
#define GST_IS_SWITCH_CONTROLLER(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), GST_TYPE_SWITCH_CONTROLLER))
#define GST_IS_SWITCH_CONTROLLER_CLASS(class) (G_TYPE_CHECK_CLASS_TYPE ((class), GST_TYPE_SWITCH_CONTROLLER))

#define SWITCH_CONTROLLER_ADDRESS	"unix:abstract=gstswitch"
#define SWITCH_CONTROLLER_OBJECT_NAME	 "info.duzy.gst_switch.SwitchControllerInterface"
#define SWITCH_CONTROLLER_OBJECT_PATH	"/info/duzy/gst_switch/SwitchController"
#define SWITCH_CLIENT_OBJECT_NAME	 "info.duzy.gst_switch.SwitchClientInterface"
#define SWITCH_CLIENT_OBJECT_PATH	"/info/duzy/gst_switch/SwitchClient"

typedef struct _GstSwitchController GstSwitchController;
typedef struct _GstSwitchControllerClass GstSwitchControllerClass;
typedef struct _GstSwitchServer GstSwitchServer;

typedef GVariant * (*MethodFunc) (GObject *, GDBusConnection *, GVariant *);
typedef struct _MethodTableEntry MethodTableEntry;
struct _MethodTableEntry {
  const gchar *name;
  MethodFunc func;
};

struct _GstSwitchController
{
  GObject base;

  GstSwitchServer *server;
  GDBusServer *bus_server;
  GMutex uis_lock;
  GList *uis;
};

struct _GstSwitchControllerClass
{
  GObjectClass base_class;

  GHashTable *methods;
};

GType gst_switch_controller_get_type (void);

void gst_switch_controller_tell_audio_port (GstSwitchController *, gint port);
void gst_switch_controller_tell_compose_port (GstSwitchController *, gint port);
void gst_switch_controller_tell_preview_port (GstSwitchController *, gint port, gint type);

#endif//__GST_SWITCH_CONTROLLER_H__by_Duzy_Chan__
