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

enum
{
  PROP_0,
  PROP_CASES,
};

G_DEFINE_TYPE (GstSwitch, gst_switch, GST_TYPE_ELEMENT);

static void 
gst_switch_init (GstSwitch *swit)
{
}

static void
gst_switch_set_property (GstSwitch *swit, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  switch (prop_id) {
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
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (swit), prop_id, pspec);
    break;
  }
}

static void
gst_switch_finalize (GstSwitch *swit)
{
}

static void
gst_switch_class_init (GstSwitchClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  object_class->set_property = (GObjectSetPropertyFunc) gst_switch_set_property;
  object_class->get_property = (GObjectGetPropertyFunc) gst_switch_get_property;
  object_class->finalize = (GObjectFinalizeFunc) gst_switch_finalize;

  /**
   *  Suppose we're having 5 sink pads, named as "sink-pad-1" to "sink-pad-5",
   *  and the first two pads should be selected by names "A" and "B", then
   *  we will have "cases" as this:
   *  
   *    structure "cases":
   *      case("A") -> pad("sink-pad-1")
   *      case("B") -> pad("sink-pad-2")
   *
   *  Other three pads could be selected by "default" label.
   */
  g_object_class_install_property (object_class, PROP_CASES,
      g_param_spec_object ("cases", "Switch Cases",
          "The cases for switching", G_TYPE_OBJECT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_set_static_metadata (element_class,
      "Stream Switch", "Element",
      "Switch within streams",
      "Duzy Chan <code@duzy.info>");

  GST_DEBUG_CATEGORY_INIT (gst_switch_debug, "switch", 0,
      "Switch");
}
