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

/*! @file */

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
 *  @param worker the GstWorker
 *  @param data user defined data pointer
 */
typedef GString *(*GstWorkerGetPipelineString) (GstWorker * worker,
    gpointer data);

/**
 *  GstWorkerGetPipelineStringFunc:
 *  @param worker the GstWorker
 */
typedef GString *(*GstWorkerGetPipelineStringFunc) (GstWorker * worker);

/**
 *  GstWorkerPrepareFunc:
 *  @param worker the GstWorker
 */
typedef gboolean (*GstWorkerPrepareFunc) (GstWorker * worker);

/**
 *  GstWorkerMessageFunc:
 *  @param worker the GstWorker
 *  @param m the message
 */
typedef gboolean (*GstWorkerMessageFunc) (GstWorker * worker, GstMessage *m);

/**
 *  GstWorkerNullFunc:
 *  @param worker the GstWorker
 */
typedef GstWorkerNullReturn (*GstWorkerNullFunc) (GstWorker * worker);

/**
 *  GstWorkerAliveFunc:
 *  @param worker the GstWorker
 */
typedef void (*GstWorkerAliveFunc) (GstWorker * worker);

/**
 *  GstWorker:
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
 *  @class GstWorkerClass
 */
struct _GstWorkerClass
{
  GObjectClass base_class; /*!< the parent class */

  /**
   *  @brief Signal handler when "prepare-worker" emitted.
   */
  void (*prepare_worker) (GstWorker * worker);

  /**
   *  @brief Signal handler when "start-worker" emitted.
   */
  void (*start_worker) (GstWorker * worker);

  /**
   * @brief Signal handler when "end-worker" emitted.
   */
  void (*end_worker) (GstWorker * worker);

  /**
   * @brief Signal handler when "worker-null" emitted.
   */
  void (*worker_null) (GstWorker * worker);

  /**
   *  @brief virtual function called when "missing plugin" discovered.
   */
  gboolean (*missing) (GstWorker * worker, gchar ** elements);

  /**
   *  @brief virtual function called on per message.
   */
  gboolean (*message) (GstWorker * worker, GstMessage * message);

  /**
   *  @brief Callback function for getting the pipeline
   *         string for the worker.
   */
  GString *(*get_pipeline_string) (GstWorker * worker);

  /**
   *  @brief Virtual function called when new pipeline is requested for
   *         creation.
   */
  GstElement *(*create_pipeline) (GstWorker * worker);

  /**
   *  @brief Virtual function called when the worker is prepared.
   */
  gboolean (*prepare) (GstWorker * worker);

  /**
   *  @brief Virtual function called when the pipeline is online.
   */
  void (*alive) (GstWorker * worker);

  /**
   *  @brief Virtual function called when the worker is getting null.
   */
  GstWorkerNullReturn (*null) (GstWorker * worker);

  /**
   *  @brief Reset reset the worker's pipeline.
   */
  gboolean (*reset) (GstWorker * worker);
};

GType gst_worker_get_type (void);

/**
 *  gst_worker_start:
 *  @param worker the GstWorker instance
 *
 *  Start the worker. This will call the derived create_pipeline and the
 *  virtual "prepare" function.
 *
 *  @param return TRUE if worker prepared and started.
 */
gboolean gst_worker_start (GstWorker * worker);

/**
 *  gst_worker_stop_force:
 *  @param worker the GstWorker instance
 *  @param force Force stopping the pipeline if TRUE.
 *
 *  Stop the pipeline, Pass TRUE to the second argument to make it force stop.
 *
 *  @param return TRUE if stop request sent.
 */
gboolean gst_worker_stop_force (GstWorker * worker, gboolean force);

/**
 *  gst_worker_stop:
 *  @param worker the GstWorker instance
 *
 *  Same as gst_worker_stop_force (worker, FALSE).
 *
 *  @param return TRUE if stop request sent.
 */
#define gst_worker_stop(worker) (gst_worker_stop_force ((worker), FALSE))

/**
 *  gst_worker_get_element_unlocked:
 *  @param worker the GstWorker instance
 *
 *  Get element by name without locking the pipeline.
 *
 *  Not MT safe.
 *
 *  @param return the element of the name or NULL if not found.
 */
GstElement *gst_worker_get_element_unlocked (GstWorker *, const gchar *);

/**
 *  gst_worker_get_element:
 *  @param worker the GstWorker instance
 *  @param name the name of the element
 *
 *  Get element by name with locking the pipeline.
 *
 *  MT safe.
 *
 *  @param return the element of the name or NULL if not found.
 */
GstElement *gst_worker_get_element (GstWorker *, const gchar * name);

#endif //__GST_WORKER_H__by_Duzy_Chan__
