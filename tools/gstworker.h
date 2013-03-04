/* GstSwitch
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
#define GST_IS_WORKER(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), GST_TYPE_WORKER))
#define GST_IS_WORKER_CLASS(class) (G_TYPE_CHECK_CLASS_TYPE ((class), GST_TYPE_WORKER))

typedef struct _GstWorker GstWorker;
typedef struct _GstWorkerClass GstWorkerClass;
typedef struct _GstSwitchServer GstSwitchServer;

typedef enum
{
  GST_WORKER_NR_END,
  GST_WORKER_NR_REPLAY,
} GstWorkerNullReturn;

/**
 *  GstWorkerGetPipelineString:
 *  @worker: the GstWorker
 *  @data: user defined data pointer
 */
typedef GString *(*GstWorkerGetPipelineString) (GstWorker * worker,
    gpointer data);

/**
 *  GstWorkerGetPipelineStringFunc:
 *  @worker: the GstWorker
 */
typedef GString *(*GstWorkerGetPipelineStringFunc) (GstWorker * worker);

/**
 *  GstWorkerPrepareFunc:
 *  @worker: the GstWorker
 */
typedef gboolean (*GstWorkerPrepareFunc) (GstWorker * worker);

/**
 *  GstWorkerMessageFunc:
 *  @worker: the GstWorker
 *  @m: the message
 */
typedef gboolean (*GstWorkerMessageFunc) (GstWorker * worker, GstMessage *m);

/**
 *  GstWorkerNullFunc:
 *  @worker: the GstWorker
 */
typedef GstWorkerNullReturn (*GstWorkerNullFunc) (GstWorker * worker);

/**
 *  GstWorkerAliveFunc:
 *  @worker: the GstWorker
 */
typedef void (*GstWorkerAliveFunc) (GstWorker * worker);

/**
 *  GstWorker:
 *  @base: the parent object
 */
struct _GstWorker
{
  GObject base;

  gchar *name;

  GstSwitchServer *server;

  GMutex pipeline_lock;
  GstElement *pipeline;
  GstBus *bus;

  GstWorkerGetPipelineString pipeline_func;
  gpointer pipeline_func_data;
  GString *pipeline_string;

  gboolean auto_replay;
  gboolean paused_for_buffering;
  guint watch;
};

/**
 *  GstWorkerClass:
 *  @base_clas: the parent class
 *  @prepare_worker: signal handler when "prepare-worker" emitted
 *  @start_worker: signal handler when "start-worker" emitted
 *  @end_worker: signal handler when "end-worker" emitted
 *  @worker_null: signal handler when "worker-null" emitted
 *  @missing: virtual function called when "missing plugin" discovered
 *  @message: virtual function called on per message
 *  @get_pipeline_string: callback function for getting the pipeline string
 *	for the worker
 *  @create_pipeline: virtual function called when new pipeline is requested
 *	for creation
 *  @prepare: virtual function called when the worker is prepared
 *  @alive: virtual function called when the pipeline is online
 *  @null: virtual function called when the worker is getting null
 *  @reset: reset the worker's pipeline
 */
struct _GstWorkerClass
{
  GObjectClass base_class;

  void (*prepare_worker) (GstWorker * worker);
  void (*start_worker) (GstWorker * worker);
  void (*end_worker) (GstWorker * worker);
  void (*worker_null) (GstWorker * worker);

  gboolean (*missing) (GstWorker * worker, gchar ** elements);
  gboolean (*message) (GstWorker * worker, GstMessage * message);

  GString *(*get_pipeline_string) (GstWorker * worker);
  GstElement *(*create_pipeline) (GstWorker * worker);
  gboolean (*prepare) (GstWorker * worker);
  void (*alive) (GstWorker * worker);
  GstWorkerNullReturn (*null) (GstWorker * worker);

  gboolean (*reset) (GstWorker * worker);
};

GType gst_worker_get_type (void);

/**
 *  gst_worker_start:
 *  @worker: the GstWorker instance
 *
 *  Start the worker. This will call the derived create_pipeline and the
 *  virtual "prepare" function.
 *
 *  @return: TRUE if worker prepared and started.
 */
gboolean gst_worker_start (GstWorker * worker);

/**
 *  gst_worker_stop_force:
 *  @worker: the GstWorker instance
 *  @force: Force stopping the pipeline if TRUE.
 *
 *  Stop the pipeline, Pass TRUE to the second argument to make it force stop.
 *
 *  @return: TRUE if stop request sent.
 */
gboolean gst_worker_stop_force (GstWorker * worker, gboolean force);

/**
 *  gst_worker_stop:
 *  @worker: the GstWorker instance
 *
 *  Same as gst_worker_stop_force (worker, FALSE).
 *
 *  @return: TRUE if stop request sent.
 */
#define gst_worker_stop(worker) (gst_worker_stop_force ((worker), FALSE))

/**
 *  gst_worker_get_element_unlocked:
 *  @worker: the GstWorker instance
 *
 *  Get element by name without locking the pipeline.
 *
 *  Not MT safe.
 *
 *  @return: the element of the name or NULL if not found.
 */
GstElement *gst_worker_get_element_unlocked (GstWorker *, const gchar *);

/**
 *  gst_worker_get_element:
 *  @worker: the GstWorker instance
 *  @name: the name of the element
 *
 *  Get element by name with locking the pipeline.
 *
 *  MT safe.
 *
 *  @return: the element of the name or NULL if not found.
 */
GstElement *gst_worker_get_element (GstWorker *, const gchar * name);

#endif //__GST_WORKER_H__by_Duzy_Chan__
