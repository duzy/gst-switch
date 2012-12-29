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

#ifndef __GST_SWITCH_SRV_H__
#define __GST_SWITCH_SRV_H__ 1
#include <gst/gst.h>
#include "../logutils.h"

typedef struct _GstCompositor GstCompositor;
typedef struct _GstSwitcher GstSwitcher;
typedef struct _GstSwitchServer GstSwitchServer;
typedef struct _GstSwitchSrvOpts GstSwitchSrvOpts;

struct _GstSwitchSrvOpts
{
  gboolean verbose;
  gchar * test_switch;
  gint port;
};

struct _GstSwitcher
{
  GstSwitchServer *server;

  GstElement *pipeline;
  GstBus *bus;
  GMainLoop *main_loop;

  GstElement *source_element;
  GstElement *sink_element;
  GstElement *switch_element;
  GstElement *conv_element;
  GstElement *sink1_element;
  GstElement *sink2_element;

  gboolean paused_for_buffering;
  guint timer_id;
};

struct _GstCompositor
{
  GstSwitchServer *server;

  GstElement *pipeline;
  GstBus *bus;
  GMainLoop *main_loop;

  GstElement *source_element;
  GstElement *sink_element;

  gboolean paused_for_buffering;
  guint timer_id;
};

struct _GstSwitchServer
{
  GstSwitcher *switcher;
  GstCompositor *compositor;
};

GstSwitcher *gst_switcher_new (GstSwitchServer *server);
void gst_switcher_free (GstSwitcher *switcher);
void gst_switcher_prepare (GstSwitcher * switcher);
void gst_switcher_start (GstSwitcher * switcher);
void gst_switcher_stop (GstSwitcher * switcher);

GstCompositor *gst_compositor_new (GstSwitchServer *server);
void gst_compositor_free (GstCompositor * compositor);
void gst_compositor_prepare (GstCompositor * compositor);
void gst_compositor_start (GstCompositor * compositor);
void gst_compositor_stop (GstCompositor * compositor);

extern GstSwitchSrvOpts opts;

#endif//__GST_SWITCH_SRV_H__
