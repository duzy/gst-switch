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

#ifndef __GST_AUDIO_VISUAL_H__by_Duzy_Chan__
#define __GST_AUDIO_VISUAL_H__by_Duzy_Chan__ 1
#include "gstworker.h"

#define GST_TYPE_AUDIO_VISUAL (gst_audio_visual_get_type ())
#define GST_AUDIO_VISUAL(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), GST_TYPE_AUDIO_VISUAL, GstAudioVisual))
#define GST_AUDIO_VISUAL_CLASS(class) (G_TYPE_CHECK_CLASS_CAST ((class), GST_TYPE_AUDIO_VISUAL, GstAudioVisualClass))
#define GST_IS_AUDIO_VISUAL(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), GST_TYPE_AUDIO_VISUAL))
#define GST_IS_AUDIO_VISUAL_CLASS(class) (G_TYPE_CHECK_CLASS_TYPE ((class), GST_TYPE_AUDIO_VISUAL))

typedef struct _GstAudioVisual GstAudioVisual;
typedef struct _GstAudioVisualClass GstAudioVisualClass;

struct _GstAudioVisual
{
  GstWorker base;

  gint port;
  gulong handle;
  gboolean active;
  gboolean renewing;

  GMutex endtime_lock;
  GstClockTime endtime;
  GMutex value_lock;
  gdouble value;
};

struct _GstAudioVisualClass
{
  GstWorkerClass base_class;
};

GType gst_audio_visual_get_type (void);

GstClockTime gst_audio_visual_get_endtime (GstAudioVisual *visual);
gdouble gst_audio_visual_get_value (GstAudioVisual *visual);

#endif//__GST_AUDIO_VISUAL_H__by_Duzy_Chan__
