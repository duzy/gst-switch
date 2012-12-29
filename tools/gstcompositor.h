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

#ifndef __GST_COMPOSITOR_H__by_Duzy_Chan__
#define __GST_COMPOSITOR_H__by_Duzy_Chan__ 1
#include <gst/gst.h>
#include "../logutils.h"

#define GST_COMPOSITOR_TYPE (gst_compositor_get_type ())

typedef struct _GstCompositor GstCompositor;
typedef struct _GstCompositorClass GstCompositorClass;
typedef struct _GstSwitchServer GstSwitchServer;

struct _GstCompositor
{
  GObject base;
  GstSwitchServer *server;

  GstElement *pipeline;
  GstBus *bus;
  GMainLoop *main_loop;

  GstElement *source_element;
  GstElement *sink_element;

  gboolean paused_for_buffering;
  guint timer_id;
};

struct _GstCompositorClass
{
  GObjectClass base_class;
};

GType gst_compositor_get_type (void);

void gst_compositor_prepare (GstCompositor * compositor);
void gst_compositor_start (GstCompositor * compositor);
void gst_compositor_stop (GstCompositor * compositor);

#endif//__GST_COMPOSITOR_H__by_Duzy_Chan__
