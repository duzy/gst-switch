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

#ifndef __GST_WORKER_H__by_Duzy_Chan__
#define __GST_WORKER_H__by_Duzy_Chan__ 1
#include <gst/gst.h>
#include "../logutils.h"

#define GST_TYPE_WORKER (gst_worker_get_type ())
#define GST_WORKER(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), GST_TYPE_WORKER, GstWorker))
#define GST_WORKER_CLASS(class) (G_TYPE_CHECK_CLASS_CAST ((class), GST_TYPE_WORKER, GstWorkerClass))
#define GST_IS_WORKER(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), GST_TYPE_WORKER, GstWorker))
#define GST_IS_WORKER_CLASS(class) (G_TYPE_CHECK_CLASS_TYPE ((class), GST_TYPE_WORKER, GstWorkerClass))

typedef struct _GstWorker GstWorker;
typedef struct _GstWorkerClass GstWorkerClass;
typedef struct _GstSwitchServer GstSwitchServer;

typedef GstElement *(*GstWorkerCreatePipelineFunc) (GstWorker *worker);
typedef gboolean (*GstWorkerPrepareFunc) (GstWorker *worker);
typedef void (*GstWorkerNullStateFunc) (GstWorker *worker);

struct _GstWorker
{
  GObject base;

  gchar *name;

  GstSwitchServer *server;

  GstBus *bus;
  GstElement *pipeline;
  GstElement *source;
  GstElement *sink;

  gboolean paused_for_buffering;
  guint timer_id;
};

struct _GstWorkerClass
{
  GObjectClass base_class;

  GstElement *(*create_pipeline) (GstWorker *worker);
  gboolean (*prepare) (GstWorker *worker);
  void (*null_state) (GstWorker *worker);
};

gboolean gst_worker_prepare (GstWorker *worker);
void gst_worker_start (GstWorker *worker);
void gst_worker_stop (GstWorker *worker);

GType gst_worker_get_type (void);

#endif//__GST_WORKER_H__by_Duzy_Chan__
