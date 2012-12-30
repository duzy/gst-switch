/* GstSwitcher
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
#include "gstswitcher.h"
#include "gstswitchsrv.h"

G_DEFINE_TYPE (GstSwitcher, gst_switcher, GST_TYPE_WORKER);

static void
gst_switcher_init (GstSwitcher * switcher)
{
}

static void
gst_switcher_finalize (GstSwitcher * switcher)
{
  if (G_OBJECT_CLASS (gst_switcher_parent_class)->finalize)
    (*G_OBJECT_CLASS (gst_switcher_parent_class)->finalize) (G_OBJECT (switcher));
}

static GstElement *
gst_switcher_create_pipeline (GstSwitcher * switcher)
{
  GString *desc;
  GstElement *pipeline;
  GError *error = NULL;

  INFO ("Listenning on port %d", opts.port);

  desc = g_string_new ("");

  g_string_append_printf (desc, "tcpmixsrc name=source mode=loop "
      "fill=none autosink=convert port=%d ", opts.port);
  g_string_append_printf (desc, "convbin name=convert "
      "converter=gdpdepay autosink=switch ");
  g_string_append_printf (desc, "switch name=switch ");
  g_string_append_printf (desc, "funnel name=compose_a ");
  g_string_append_printf (desc, "funnel name=compose_b ");
  g_string_append_printf (desc, "source. ! convert. ");
  g_string_append_printf (desc, "compose_a. ! gdppay ! tcpserversink port=3001 ");
  g_string_append_printf (desc, "compose_b. ! gdppay ! tcpserversink port=3002 ");
  g_string_append_printf (desc, "switch.src_1 ! compose_b. ");
  g_string_append_printf (desc, "switch.src_0 ! compose_a. ");

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

static void
on_source_pad_added (GstElement * element, GstPad * pad,
    GstSwitcher * switcher)
{
  INFO ("source-pad-added: %s.%s", GST_ELEMENT_NAME (element),
      GST_PAD_NAME (pad));
}

static void
on_switch_pad_added (GstElement * element, GstPad * pad,
    GstSwitcher * switcher)
{
  INFO ("switch-pad-added: %s.%s", GST_ELEMENT_NAME (element),
      GST_PAD_NAME (pad));
}

static void
on_convert_pad_added (GstElement * element, GstPad * pad,
    GstSwitcher * switcher)
{
  INFO ("convert-pad-added: %s.%s", GST_ELEMENT_NAME (element),
      GST_PAD_NAME (pad));
}

static void
on_source_new_client (GstElement * element, GstPad * pad,
    GstSwitcher * switcher)
{
  INFO ("new client on %s.%s", GST_ELEMENT_NAME (element),
      GST_PAD_NAME (pad));
}

static gboolean
gst_switcher_prepare (GstSwitcher *switcher)
{
  GstWorker *worker = GST_WORKER (switcher);

  if (worker->source) {
    g_signal_connect (worker->source, "pad-added",
	G_CALLBACK (on_source_pad_added), switcher);

    g_signal_connect (worker->source, "new-client",
	G_CALLBACK (on_source_new_client), switcher);
  }

  /*
  if (switcher->switch_element) {
    GstElement * swit = switcher->switch_element;
    GList * item = GST_ELEMENT_PADS (swit);
    GstPad * pad;
    for (; item; item = g_list_next (item)) {
      pad = GST_PAD (item->data);
      INFO ("switch-pad: %s", GST_PAD_NAME (pad));
    }

    g_signal_connect (switcher->switch_element, "pad-added",
	G_CALLBACK (on_switch_pad_added), switcher);
  }

  if (switcher->sink1_element) {
    g_signal_connect (switcher->sink1_element, "pad-added",
	G_CALLBACK (on_sink1_pad_added), switcher);
  }

  if (switcher->sink2_element) {
    g_signal_connect (switcher->sink2_element, "pad-added",
	G_CALLBACK (on_sink2_pad_added), switcher);
  }
  */

  return TRUE;
}

#if 0
gboolean
have_element (const gchar * element_name)
{
  GstPluginFeature *feature;

  feature = gst_default_registry_find_feature (element_name,
      GST_TYPE_ELEMENT_FACTORY);
  if (feature) {
    g_object_unref (feature);
    return TRUE;
  }
  return FALSE;
}
#endif

static void
gst_switcher_class_init (GstSwitcherClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstWorkerClass *worker_class = GST_WORKER_CLASS (klass);
  object_class->finalize = (GObjectFinalizeFunc) gst_switcher_finalize;
  worker_class->create_pipeline = (GstWorkerCreatePipelineFunc)
    gst_switcher_create_pipeline;
  worker_class->prepare = (GstWorkerPrepareFunc) gst_switcher_prepare;
}
