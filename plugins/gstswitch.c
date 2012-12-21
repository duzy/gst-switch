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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstswitch.h"

GST_DEBUG_CATEGORY_STATIC (gst_switch_debug);
#define GST_CAT_DEFAULT gst_switch_debug

#define CASE_ELEMENT_NAME "funnel"

static GstStaticPadTemplate gst_switch_sink_factory =
  GST_STATIC_PAD_TEMPLATE ("sink_%u",
      GST_PAD_SINK,
      GST_PAD_REQUEST,
      GST_STATIC_CAPS_ANY);

static GstStaticPadTemplate gst_switch_src_factory =
  GST_STATIC_PAD_TEMPLATE ("src_%u",
      GST_PAD_SRC,
      GST_PAD_REQUEST,
      GST_STATIC_CAPS_ANY);

enum
{
  PROP_0,
  PROP_CASES,
};

enum
{
  GST_SWITCH_PAD_FLAG_REQUESTED = (GST_PAD_FLAG_LAST << 1),
  GST_SWITCH_PAD_FLAG_LAST	= (GST_PAD_FLAG_LAST << 2)
};

G_DEFINE_TYPE (GstSwitch, gst_switch, GST_TYPE_BIN);

static void 
gst_switch_init (GstSwitch *swit)
{
  swit->default_case = gst_element_factory_make (CASE_ELEMENT_NAME,
      "default");

  //GST_OBJECT_FLAG_SET (swit->default_case, GST_ELEMENT_FLAG_SOURCE);
  //GST_OBJECT_FLAG_SET (swit->default_case, GST_ELEMENT_FLAG_SINK);

  if (!gst_bin_add (GST_BIN (swit), swit->default_case)) {
    GST_ERROR_OBJECT (swit, "Failed to add default case");
  }
}

static void
gst_switch_set_property (GstSwitch *swit, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  switch (prop_id) {
  case PROP_CASES:
    g_print ("%s:%d: set_property: cases=%s\n", __FILE__, __LINE__,
	g_value_get_string (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (swit), prop_id, pspec);
    break;
  }
}

static void
gst_switch_get_property (GstSwitch *swit, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  switch (prop_id) {
  case PROP_CASES:
    g_print ("%s:%d: get_property: cases=%s\n", __FILE__, __LINE__,
	g_value_get_string (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (swit), prop_id, pspec);
    break;
  }
}

static void
gst_switch_finalize (GstSwitch *swit)
{
  G_OBJECT_CLASS (swit)->finalize (G_OBJECT (swit));
}

static GstPad *
gst_switch_request_new_pad (GstElement *element,
    GstPadTemplate * templ, const gchar * unused, const GstCaps * caps)
{
  GstSwitch * swit = GST_SWITCH (element);
  GstElement * swcase = NULL;
  GstPad * realpad = NULL;
  GstPad * ghostpad = NULL;
  gchar *name = NULL;
  GList *item;
  gint num;

  switch (GST_PAD_TEMPLATE_DIRECTION (templ)) {
  case GST_PAD_SINK:
  {
    /*
    name = g_strdup_printf ("case_%u", num);
    swcase = gst_element_factory_make (CASE_ELEMENT_NAME, name);
    g_free (name);

    GST_OBJECT_FLAG_SET (swcase, GST_ELEMENT_FLAG_SOURCE);
    GST_OBJECT_FLAG_SET (swcase, GST_ELEMENT_FLAG_SINK);

    if (!gst_bin_add (GST_BIN (swit), swcase))
      goto error_bin_add;
    */

    swcase = swit->default_case;

    item = GST_ELEMENT_PADS (swcase);
    for (num = 0; item; item = g_list_next (item))
      if (GST_PAD_IS_SINK (GST_PAD (item->data))) ++num;
    name = g_strdup_printf ("sink_%u", num);
    realpad = gst_element_request_pad (swcase, templ, name, caps);
    g_free (name);

    item = GST_ELEMENT_PADS (swit);
    for (num = 0; item; item = g_list_next (item))
      if (GST_PAD_IS_SINK (GST_PAD (item->data))) ++num;
    name = g_strdup_printf ("sink_%u", num);
    ghostpad = gst_ghost_pad_new (name, realpad);
    gst_object_unref (realpad);
    g_free (name);
  } break;
  case GST_PAD_SRC:
  {
    item = GST_BIN_CHILDREN (GST_BIN (swit));
    for (num = 0; item; item = g_list_next (item)) {
      swcase = GST_ELEMENT (item->data);
      realpad = gst_element_get_static_pad (swcase, "src");

      if (!gst_pad_is_linked (realpad) &&
	  !GST_OBJECT_FLAG_IS_SET (realpad, GST_SWITCH_PAD_FLAG_REQUESTED))
	break;

      gst_object_unref (GST_OBJECT (realpad));
      realpad = NULL;
      ++num;
    }

    name = g_strdup_printf ("src_%u", num);
    ghostpad = gst_ghost_pad_new (name, realpad);
    gst_object_unref (GST_OBJECT (realpad));
    g_free (name);
  } break;
  default:
    goto error_direction;
  }

  if (!realpad)
    goto error_no_pad;

  if (!gst_element_add_pad (swit, ghostpad))
    goto error_add_pad;

  {
    gchar * dir = "?";
    switch (GST_PAD_DIRECTION (ghostpad)) {
    case GST_PAD_SRC:  dir = "SRC";  break;
    case GST_PAD_SINK: dir = "SINK"; break;
    default: break;
    }
    g_print ("%s:%d: request_new_pad: %s: %s.%s (%s.%s)\n",
	__FILE__, __LINE__, dir,
	GST_ELEMENT_NAME (swit), GST_PAD_NAME (ghostpad),
	GST_ELEMENT_NAME (swcase), GST_PAD_NAME (realpad));
  }


  GST_OBJECT_FLAG_SET (realpad, GST_SWITCH_PAD_FLAG_REQUESTED);
  return ghostpad;

  /* Handling Errors */
 error_bin_add:
  {
    GST_ERROR_OBJECT (swit, "failed adding child %s",
	GST_ELEMENT_NAME (swcase));
    g_object_unref (swcase);
    return NULL;
  }

 error_direction:
  {
    GST_ERROR_OBJECT (swit, "unknown pad direction %s",
	GST_PAD_TEMPLATE_NAME_TEMPLATE (templ));
    return NULL;
  }

 error_no_pad:
  {
    GST_ERROR_OBJECT (swit, "no pad %s on %s",
	GST_PAD_TEMPLATE_NAME_TEMPLATE (templ), GST_ELEMENT_NAME (swcase));
    return NULL;
  }

 error_add_pad:
  {
    GST_ERROR_OBJECT (swit, "failed to add pad");
    gst_object_unref (ghostpad);
    return NULL;
  }
}

static void
gst_switch_release_pad (GstElement * element, GstPad * pad)
{
  GstSwitch * swit = GST_SWITCH (element);
  gst_pad_set_active (pad, FALSE);
  gst_element_remove_pad (element, pad);
}

static void
gst_switch_class_init (GstSwitchClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  object_class->set_property = (GObjectSetPropertyFunc) gst_switch_set_property;
  object_class->get_property = (GObjectGetPropertyFunc) gst_switch_get_property;
  object_class->finalize = (GObjectFinalizeFunc) gst_switch_finalize;

  g_object_class_install_property (object_class, PROP_CASES,
      g_param_spec_string ("cases", "Switch Cases",
          "The cases for switching", "",
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_set_static_metadata (element_class,
      "Stream Switch", "Element",
      "Switch within streams",
      "Duzy Chan <code@duzy.info>");

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_switch_sink_factory));

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_switch_src_factory));

  element_class->request_new_pad = gst_switch_request_new_pad;
  element_class->release_pad = gst_switch_release_pad;

  GST_DEBUG_CATEGORY_INIT (gst_switch_debug, "switch", 0, "Switch");
}
