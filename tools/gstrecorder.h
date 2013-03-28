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

/*! @file */

#ifndef __GST_RECORDER_H__by_Duzy_Chan__
#define __GST_RECORDER_H__by_Duzy_Chan__ 1
#include "gstworker.h"
#include "gstcomposite.h"
#include <gio/gio.h>

#define GST_TYPE_RECORDER (gst_recorder_get_type ())
#define GST_RECORDER(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), GST_TYPE_RECORDER, GstRecorder))
#define GST_RECORDER_CLASS(class) (G_TYPE_CHECK_CLASS_CAST ((class), GST_TYPE_RECORDER, GstRecorderClass))
#define GST_IS_RECORDER(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), GST_TYPE_RECORDER))
#define GST_IS_RECORDER_CLASS(class) (G_TYPE_CHECK_CLASS_TYPE ((class), GST_TYPE_RECORDER))

typedef struct _GstRecorderClass GstRecorderClass;

/**
 *  GstRecorder:
 *  @param base the parent object
 *  @param sink_port the encode sink port
 *  @param width the video width
 *  @param height the video height
 *  @param mode the composite mode which is the same as in GstComposite
 */
struct _GstRecorder
{
  GstWorker base;

  gint sink_port;
  guint width;
  guint height;

  GstCompositeMode mode;
};

/**
 *  GstRecorderClass:
 *  @param base_class the parent class
 *  @param end_recorder the "end-recorder" signal handler
 */
struct _GstRecorderClass
{
  GstWorkerClass base_class;

  void (*end_recorder) (GstRecorder * cas);
};

GType gst_recorder_get_type (void);

#endif //__GST_RECORDER_H__by_Duzy_Chan__
