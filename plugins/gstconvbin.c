/* GStreamer
 * Copyright (C) 2012 Duzy Chan <code@duzy.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */

/**
 * SECTION:element-gstconvbin
 *
 * The convbin element is designed to convert N inputs at once using a
 * specified converter.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v videotestsrc ! convbin converter=gdppay ! filesink location=a.gdp
 * ]|
 *
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstconvbin.h"

GST_DEBUG_CATEGORY_STATIC (gst_conv_bin_debug);
#define GST_CAT_DEFAULT gst_conv_bin_debug

static GstStaticPadTemplate gst_conv_bin_sink_factory =
  GST_STATIC_PAD_TEMPLATE ("sink_%u",
      GST_PAD_SINK,
      GST_PAD_REQUEST,
      GST_STATIC_CAPS_ANY);

static GstStaticPadTemplate gst_conv_bin_src_factory =
  GST_STATIC_PAD_TEMPLATE ("src_%u",
      GST_PAD_SRC,
      GST_PAD_REQUEST,
      GST_STATIC_CAPS_ANY);

enum
{
  PROP_0,
  PROP_CONVERTER,
};

G_DEFINE_TYPE (GstConvBin, gst_conv_bin, GST_TYPE_BIN);

static void
gst_conv_bin_set_property (GstConvBin *conv, guint prop_id,
    const GValue *value, GParamSpec * spec)
{
  switch (prop_id) {
  case PROP_CONVERTER:
    g_print ("%s:%d: set_property: converter=%s\n", __FILE__, __LINE__,
	g_value_get_string (value));

    g_free (conv->converter);
    conv->converter = g_strdup (g_value_get_string (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (conv), prop_id, spec);
    break;
  }
}

static void
gst_conv_bin_get_property (GstConvBin *conv, guint prop_id,
    GValue *value, GParamSpec * spec)
{
  switch (prop_id) {
  case PROP_CONVERTER:
    g_value_set_string (value, conv->converter);
    g_print ("%s:%d: get_property: converter=%s\n", __FILE__, __LINE__,
	g_value_get_string (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (conv), prop_id, spec);
    break;
  }
}

static void
gst_conv_bin_finalize (GstConvBin *conv)
{
  g_free (conv->converter);
  G_OBJECT_CLASS (conv)->finalize (G_OBJECT (conv));
}

static void
gst_conv_bin_init (GstConvBin *conv)
{
  conv->converter = NULL;
}

static void
gst_conv_bin_add_paired_src_pad (GstConvBin * conv, GstPad * basepad)
{
  GstElement * baseconv = GST_ELEMENT (GST_PAD_PARENT (basepad));
  GstElement * linked_element = NULL;
  GstPad * basesrcpad = gst_element_get_static_pad (baseconv, "src");
  GstPad * srcpad = NULL;
  GList * item = NULL;
  gchar * name = NULL;
  gint num;

  if (gst_pad_is_linked (basepad) || 
      GST_OBJECT_FLAG_IS_SET (basepad, GST_CONV_BIN_PAD_FLAG_GHOSTED)) {
    return;
  }

  item = GST_ELEMENT_PADS (conv);
  for (num = 0; item; item = g_list_next (item)) {
    if (GST_PAD_IS_SRC (GST_PAD (item->data))) {
      GstPad * pp = GST_PAD_PEER (GST_PAD (item->data));

      if (!linked_element && pp)
	linked_element = GST_ELEMENT (GST_PAD_PARENT (pp));

      ++num;
    }
  }

  name = g_strdup_printf ("src_%u", num);
  srcpad = gst_ghost_pad_new (name, basesrcpad);
  gst_object_unref (basesrcpad);
  g_free (name);

  gst_pad_set_active (srcpad, TRUE);

  if (gst_element_add_pad (GST_ELEMENT (conv), srcpad)) {
    GST_OBJECT_FLAG_SET (basesrcpad, GST_CONV_BIN_PAD_FLAG_GHOSTED);
  }

  if (linked_element) {
    // FIXME: dynamyc name for "sink_%u"
    GstPadLinkReturn linkRet;
    GstPad * pp = gst_element_get_request_pad (linked_element, "sink_%u");

    GST_DEBUG_OBJECT (srcpad, "Link %s.%s",
	GST_ELEMENT_NAME (GST_PAD_PARENT (pp)), GST_PAD_NAME (pp));

    linkRet = gst_pad_link (GST_PAD (srcpad), GST_PAD (pp));
    if (GST_PAD_LINK_FAILED (linkRet)) {
      GST_ERROR_OBJECT (srcpad, "Cannot link %s.%s",
	GST_ELEMENT_NAME (GST_PAD_PARENT (pp)), GST_PAD_NAME (pp));
    }
  }
}

static GstPad *
gst_conv_bin_request_new_pad (GstElement *element,
    GstPadTemplate * templ, const gchar * unused, const GstCaps * caps)
{
  GstConvBin * conv = GST_CONV_BIN (element);
  GstPadDirection dir = GST_PAD_TEMPLATE_DIRECTION (templ);
  GstPad * pad = NULL, * basepad = NULL;
  GstElement * baseconv = NULL;
  gchar * name = NULL;
  GList * item = NULL;
  gint num = 0;

  if (conv->converter == NULL)
    goto error_no_converter_name;

  switch (dir) {
  case GST_PAD_SINK: name = "sink"; break;
  case GST_PAD_SRC:  name = "src";  break;
  default:
    GST_ERROR_OBJECT (conv, "unknown pad direction %s",
	GST_PAD_TEMPLATE_NAME_TEMPLATE (templ));
    break;
  }
  for (item = GST_BIN_CHILDREN (GST_BIN (conv)); item; item = g_list_next (item)) {
    baseconv = GST_ELEMENT (item->data);
    basepad = gst_element_get_static_pad (baseconv, name);

    if (!gst_pad_is_linked (basepad) &&
	!GST_OBJECT_FLAG_IS_SET (basepad, GST_CONV_BIN_PAD_FLAG_GHOSTED))
      break;

    gst_object_unref (GST_OBJECT (basepad));
    baseconv = NULL;
    basepad = NULL;
  }

  /*
  g_print ("%s:%d: %s\n", __FILE__, __LINE__,
      GST_PAD_TEMPLATE_NAME_TEMPLATE (templ));
  */

  if (!basepad) {
    item = GST_BIN_CHILDREN (GST_BIN (conv));
    for (num = 0; item; item = g_list_next (item)) ++num;
    name = g_strdup_printf ("conv_%u", num);
    baseconv = gst_element_factory_make (conv->converter, name);
    g_free (name);
    name = NULL;

    if (!baseconv)
      goto error_no_converter;
    if (!gst_bin_add (GST_BIN (conv), baseconv))
      goto error_bin_add;

    /*
    GST_DEBUG_OBJECT (conv, "New converter: %s", GST_ELEMENT_NAME (baseconv));
    */

    switch (dir) {
    case GST_PAD_SINK:
      //basepad = gst_element_request_pad (baseconv, templ, "sink_0", caps);
      basepad = gst_element_get_static_pad (baseconv, "sink");
      gst_conv_bin_add_paired_src_pad (conv, basepad);
      break;
    case GST_PAD_SRC:
      //basepad = gst_element_request_pad (baseconv, templ, "src_0", caps);
      basepad = gst_element_get_static_pad (baseconv, "src");
      break;
    default:
      basepad = NULL;
      break;
    }
  }

  /*
  g_print ("%s:%d: %s -> %s\n", __FILE__, __LINE__,
      GST_PAD_TEMPLATE_NAME_TEMPLATE (templ),
      GST_PAD_NAME (basepad));
  */

  if (basepad) {
    item = GST_ELEMENT_PADS (conv);
    for (num = 0; item; item = g_list_next (item))
      if (GST_PAD_DIRECTION (GST_PAD (item->data)) == dir)
	++num;

    switch (dir) {
    case GST_PAD_SINK:
      name = g_strdup_printf ("sink_%u", num);
      break;
    case GST_PAD_SRC:
      name = g_strdup_printf ("src_%u", num);
      break;
    default:
      name = NULL;
      break;
    }

    pad = gst_ghost_pad_new (name, basepad);
    gst_object_unref (basepad);
    g_free (name);

    gst_pad_set_active (pad, TRUE);

    if (gst_element_add_pad (GST_ELEMENT (conv), pad)) {
      GST_OBJECT_FLAG_SET (basepad, GST_CONV_BIN_PAD_FLAG_GHOSTED);
    }
  } else {
    GST_ERROR_OBJECT (conv, "No basepad for %s",
	GST_PAD_TEMPLATE_NAME_TEMPLATE (templ));
  }

  /*
  g_print ("%s:%d: %s -> %s -> %s\n", __FILE__, __LINE__,
      GST_PAD_TEMPLATE_NAME_TEMPLATE (templ),
      GST_PAD_NAME (basepad), GST_PAD_NAME (pad));
  */

  if (pad) {
    GST_DEBUG_OBJECT (pad, "Requested %s.%s",
	GST_ELEMENT_NAME (baseconv), GST_PAD_NAME (basepad));
  }

  return pad;

  /* Handing Errors */
 error_no_converter_name:
  {
    GST_ERROR_OBJECT (conv, "no underlayer converter name");
    return NULL;
  }

 error_no_converter:
  {
    GST_ERROR_OBJECT (conv, "not converter by name %s", conv->converter);
    return NULL;
  }

 error_bin_add:
  {
    gst_object_unref (baseconv);
    GST_ERROR_OBJECT (conv, "failed to add converter");
    return NULL;
  }
}

static void
gst_conv_bin_release_pad (GstElement * element, GstPad * pad)
{
  //GstConvBin * conv = GST_CONV_BIN (element);
  gst_pad_set_active (pad, FALSE);
  gst_element_remove_pad (element, pad);
}

static void
gst_conv_bin_class_init (GstConvBinClass *klass)
{
  GObjectClass * object_class = G_OBJECT_CLASS (klass);
  GstElementClass * element_class = GST_ELEMENT_CLASS (klass);

  object_class->set_property = (GObjectSetPropertyFunc) gst_conv_bin_set_property;
  object_class->get_property = (GObjectGetPropertyFunc) gst_conv_bin_get_property;
  object_class->finalize = (GObjectFinalizeFunc) gst_conv_bin_finalize;

  g_object_class_install_property (object_class, PROP_CONVERTER,
      g_param_spec_string ("converter", "Base Converter",
	  "Convert N inputs using a specified base converter.", "",
	  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_set_static_metadata (element_class,
      "Convert Bin", "Bin",
      "Convert N inputs base on a specified converter",
      "Duzy Chan <code@duzy.info>");

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_conv_bin_sink_factory));

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_conv_bin_src_factory));

  element_class->request_new_pad = gst_conv_bin_request_new_pad;
  element_class->release_pad = gst_conv_bin_release_pad;

  GST_DEBUG_CATEGORY_INIT (gst_conv_bin_debug, "convbin", 0, "ConvertBin");
}
