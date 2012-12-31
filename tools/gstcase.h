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

#ifndef __GST_CASE_H__by_Duzy_Chan__
#define __GST_CASE_H__by_Duzy_Chan__ 1
#include "gstworker.h"
#include <gio/gio.h>

#define GST_TYPE_CASE (gst_case_get_type ())
#define GST_CASE(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), GST_TYPE_CASE, GstCase))
#define GST_CASE_CLASS(class) (G_TYPE_CHECK_CLASS_CAST ((class), GST_TYPE_CASE, GstCaseClass))
#define GST_IS_CASE(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), GST_TYPE_CASE, GstCase))
#define GST_IS_CASE_CLASS(class) (G_TYPE_CHECK_CLASS_TYPE ((class), GST_TYPE_CASE, GstCaseClass))

typedef struct _GstCase GstCase;
typedef struct _GstCaseClass GstCaseClass;
typedef struct _GstSwitchServer GstSwitchServer;

typedef enum {
  GST_CASE_COMPOSITE_A,
  GST_CASE_COMPOSITE_B,
  GST_CASE_PREVIEW,
} GstCaseType;

struct _GstCase
{
  GstWorker base;
  GstCaseType type;
  GInputStream *stream;
  gint sink_port;
};

struct _GstCaseClass
{
  GstWorkerClass base_class;

  void (*end_case) (GstCase *cas);
};

GType gst_case_get_type (void);

#endif//__GST_CASE_H__by_Duzy_Chan__
