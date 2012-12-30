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

#ifndef __GST_SWITCH_SRV_H__by_Duzy_Chan__
#define __GST_SWITCH_SRV_H__by_Duzy_Chan__ 1
#include "gstswitcher.h"
#include "gstcompositor.h"
#include "../logutils.h"

#define GST_SWITCH_SERVER_TYPE (gst_switchsrv_get_type())
#define GST_SWITCH_SERVER(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), GST_SWITCH_SERVER_TYPE, GstSwitchServer))
#define GST_SWITCH_SERVER_CLASS(class) (G_TYPE_CHECK_CLASS_CAST ((class), GST_SWITCH_SERVER_TYPE, GstSwitchServerClass))
#define GST_IS_SWITCH_SERVER(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), GST_SWITCH_SERVER_TYPE))
#define GST_IS_SWITCH_SERVER_CLASS(class) (G_TYPE_CHECK_CLASS_TYPE ((class), GST_SWITCH_SERVER_TYPE))

typedef struct _GstSwitchServer GstSwitchServer;
typedef struct _GstSwitchServerClass GstSwitchServerClass;
typedef struct _GstSwitchServerOpts GstSwitchServerOpts;

struct _GstSwitchServerOpts
{
  gboolean verbose;
  gchar * test_switch;
  gint port;
};

struct _GstSwitchServer
{
  GObject base;
  GstSwitcher *switcher;
  GstCompositor *compositor;

  GMainLoop *main_loop;
};

struct _GstSwitchServerClass
{
  GObjectClass base_class;
};

GType gst_switchsrv_get_type (void);

extern GstSwitchServerOpts opts;

#endif//__GST_SWITCH_SRV_H__by_Duzy_Chan__
