/* GstCase
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include "gstswitchsrv.h"
#include "gstcase.h"

enum
{
  PROP_0,
  PROP_STREAM,
};

G_DEFINE_TYPE (GstCase, gst_case, GST_WORKER_TYPE);

static void
gst_case_init (GstCase * cas)
{
}

static void
gst_case_finalize (GstCase * cas)
{
  if (cas->stream) {
    g_object_unref (cas->stream);
    cas->stream = NULL;
  }

  if (G_OBJECT_CLASS (gst_case_parent_class)->finalize)
    (*G_OBJECT_CLASS (gst_case_parent_class)->finalize) (G_OBJECT (cas));

  INFO ("Case finalized");
}

static void
gst_case_get_property (GstCase *cas, guint property_id,
    GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
    case PROP_STREAM:
      g_value_set_object (value, cas->stream);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (cas, property_id, pspec);
      break;
  }
}

static void
gst_case_set_property (GstCase *cas, guint property_id,
    const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  case PROP_STREAM: {
    GObject *stream = g_value_dup_object (value);
    if (cas->stream)
      g_object_unref (cas->stream);
    cas->stream = G_INPUT_STREAM (stream);
    break;
  }
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (cas), property_id, pspec);
    break;
  }
}

static GstElement *
gst_case_create_pipeline (GstCase * cas)
{
  GString *desc;
  GstElement *pipeline;
  GError *error = NULL;

  desc = g_string_new ("");

  g_string_append_printf (desc, "giostreamsrc name=source ");
  g_string_append_printf (desc, "fdsink name=sink fd=2 ");
  g_string_append_printf (desc, "source. ! sink. ");

  if (opts.verbose)
    g_print ("pipeline: %s\n", desc->str);

  pipeline = (GstElement *) gst_parse_launch (desc->str, &error);
  g_string_free (desc, FALSE);

  if (error) {
    ERROR ("pipeline parsing error: %s", error->message);
    gst_object_unref (pipeline);
    return NULL;
  }

  return pipeline;
}

static gboolean
gst_case_prepare (GstCase *cas)
{
  GstWorker *worker = GST_WORKER (cas);

  if (!cas->stream) {
    ERROR ("no stream for new case");
    return FALSE;
  }

  if (!worker->source) {
    ERROR ("no source");
    return FALSE;
  }

  g_object_set (worker->source, "stream", cas->stream, NULL);

  return TRUE;
}

static void
gst_case_null (GstCase *cas)
{
  GstWorker *worker = GST_WORKER (cas);
  INFO ("null: %s", worker->name);
}

static void
gst_case_class_init (GstCaseClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstWorkerClass *worker_class = GST_WORKER_CLASS (klass);

  object_class->finalize = (GObjectFinalizeFunc) gst_case_finalize;
  object_class->set_property = (GObjectSetPropertyFunc) gst_case_set_property;
  object_class->get_property = (GObjectGetPropertyFunc) gst_case_get_property;

  g_object_class_install_property (object_class, PROP_STREAM,
      g_param_spec_object ("stream", "Stream", "Stream to read from",
          G_TYPE_INPUT_STREAM, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  worker_class->null_state = (GstWorkerNullState) gst_case_null;
  worker_class->prepare = (GstWorkerPrepareFunc) gst_case_prepare;
  worker_class->create_pipeline = (GstWorkerCreatePipelineFunc)
    gst_case_create_pipeline;
}
