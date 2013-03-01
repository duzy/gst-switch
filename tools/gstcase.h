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

#ifndef __GST_CASE_H__by_Duzy_Chan__
#define __GST_CASE_H__by_Duzy_Chan__ 1
#include "gstworker.h"
#include <gio/gio.h>

#define GST_TYPE_CASE (gst_case_get_type ())
#define GST_CASE(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), GST_TYPE_CASE, GstCase))
#define GST_CASE_CLASS(class) (G_TYPE_CHECK_CLASS_CAST ((class), GST_TYPE_CASE, GstCaseClass))
#define GST_IS_CASE(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), GST_TYPE_CASE))
#define GST_IS_CASE_CLASS(class) (G_TYPE_CHECK_CLASS_TYPE ((class), GST_TYPE_CASE))

typedef struct _GstCase GstCase;
typedef struct _GstCaseClass GstCaseClass;

typedef enum
{
  GST_CASE_UNKNOWN,
  GST_CASE_COMPOSITE_A,
  GST_CASE_COMPOSITE_B,
  GST_CASE_COMPOSITE_a,         /* composite audio */
  GST_CASE_PREVIEW,
  GST_CASE_INPUT_a,             /* audio input */
  GST_CASE_INPUT_v,             /* video input */
  GST_CASE_BRANCH_A,            /* video branch */
  GST_CASE_BRANCH_B,            /* video branch */
  GST_CASE_BRANCH_a,            /* audio branch */
  GST_CASE_BRANCH_p,            /* preview branch */
  GST_CASE__LAST_TYPE = GST_CASE_BRANCH_p
} GstCaseType;

typedef enum
{
  GST_SERVE_NOTHING,
  GST_SERVE_VIDEO_STREAM,
  GST_SERVE_AUDIO_STREAM,
} GstSwitchServeStreamType;

struct _GstCase
{
  GstWorker base;
  GstCaseType type;
  GInputStream *stream;
  GstCase *input;
  GstCase *branch;
  GstSwitchServeStreamType serve_type;
  gboolean switching;
  gint sink_port;
  guint width;
  guint height;
  guint a_width;
  guint a_height;
  guint b_width;
  guint b_height;
};

struct _GstCaseClass
{
  GstWorkerClass base_class;
};

GType gst_case_get_type (void);

#endif //__GST_CASE_H__by_Duzy_Chan__
