/* GstSwitch
 * Copyright (C) 2013 Duzy Chan <code@duzy.info>
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

#include "gstassess.h"
#include "../logutils.h"

GST_DEBUG_CATEGORY_STATIC (gst_assess_debug);
#define GST_CAT_DEFAULT gst_assess_debug

#define GST_ASSESS_LOCK(obj) (g_mutex_lock (&(obj)->lock))
#define GST_ASSESS_UNLOCK(obj) (g_mutex_unlock (&(obj)->lock))
#define ASSESS_DB_LOCK() (g_mutex_lock (&assess_db_lock))
#define ASSESS_DB_UNLOCK() (g_mutex_unlock (&assess_db_lock))
#define ASSESS_POINT_LOCK(ap) (g_mutex_lock (&(ap)->lock))
#define ASSESS_POINT_UNLOCK(ap) (g_mutex_unlock (&(ap)->lock))

typedef struct _GstAssessPoint {
  GMutex lock;
  const gchar *name;
  guint64 dropped_time; /* measured in milliseconds */
} GstAssessPoint;

typedef struct _GstAssessDB {
  guint timer;
  GHashTable *hash;
} GstAssessDB;

static GMutex assess_db_lock = { 0 };
static GstAssessDB assess_db = { 0 };

static GstStaticPadTemplate gst_assess_sink_factory =
  GST_STATIC_PAD_TEMPLATE ("sink",
      GST_PAD_SINK,
      GST_PAD_REQUEST,
      GST_STATIC_CAPS_ANY);

static GstStaticPadTemplate gst_assess_src_factory =
  GST_STATIC_PAD_TEMPLATE ("src",
      GST_PAD_SRC,
      GST_PAD_REQUEST,
      GST_STATIC_CAPS_ANY);

enum
{
  PROP_0,
};

#define gst_assess_parent_class parent_class
G_DEFINE_TYPE (GstAssess, gst_assess, GST_TYPE_BASE_TRANSFORM);

static gboolean
assess_db_timeout (gpointer data)
{
  gboolean ret = TRUE;
  GHashTable *assess_point_hash = NULL;
  GList *keys, *key, *names, *name;
  ASSESS_DB_LOCK ();
  keys = g_hash_table_get_keys (assess_db.hash);
  for (key = keys; key; key = g_list_next (key)) {
    const gchar *s = (gchar *) key->data;
    assess_point_hash  = g_hash_table_lookup (assess_db.hash, s);
    names = g_hash_table_get_keys (assess_point_hash);
    INFO ("%s", s);
    for (name = names; name; name = g_list_next (name)) {
      GstAssessPoint *assess_point = g_hash_table_lookup (
	  assess_point_hash, name->data);
      INFO ("  %s", assess_point->name);
      INFO ("  %lld", (long long int) assess_point->dropped_time);
    }
    g_list_free (names);
  }
  g_list_free (keys);
  ASSESS_DB_UNLOCK ();
  return ret;
}

static void 
gst_assess_init (GstAssess *assess)
{
  g_mutex_init (&assess->lock);
}

static void
gst_assess_dispose (GstAssess *assess)
{
  GstElement *pipeline = NULL;
  GHashTable *assess_point_hash = NULL;

  pipeline = GST_ELEMENT (gst_element_get_parent (GST_ELEMENT (assess)));

  if (!pipeline) {
    ERROR ("fix me");
    goto end;
  }

  ASSESS_DB_LOCK ();

  if (assess_db.hash) {
    assess_point_hash = g_hash_table_lookup (assess_db.hash,
	GST_ELEMENT_NAME (pipeline));
    if (assess_point_hash) {
      g_hash_table_remove (assess_point_hash, GST_ELEMENT_NAME (assess));
    }
  }

  ASSESS_DB_UNLOCK ();

 end:
  G_OBJECT_CLASS (parent_class)->dispose (G_OBJECT (assess));
}

static void
gst_assess_finalize (GstAssess *assess)
{
  g_mutex_clear (&assess->lock);

  G_OBJECT_CLASS (parent_class)->finalize (G_OBJECT (assess));
}

static void
gst_assess_set_property (GstAssess *assess, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  switch (prop_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (assess), prop_id, pspec);
    break;
  }
}

static void
gst_assess_get_property (GstAssess *assess, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  switch (prop_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (assess), prop_id, pspec);
    break;
  }
}

static gboolean
gst_assess_start (GstBaseTransform * trans)
{
  GstAssess *this = GST_ASSESS (trans);
  (void) this;
  return TRUE;
}

static gboolean
gst_assess_stop (GstBaseTransform * trans)
{
  GstAssess *this = GST_ASSESS (trans);
  (void) this;
  return TRUE;
}

static GstFlowReturn
gst_assess_transform (GstBaseTransform *trans, GstBuffer *buffer)
{
  GstAssess *this = NULL;
  GstElement *pipeline = NULL;
  GstClock *pipeline_clock = NULL;
  GstClockTime t;
  GstFlowReturn ret;
  GHashTable *assess_point_hash = NULL;
  GstAssessPoint *assess_point = NULL;

  this = GST_ASSESS (trans);
  pipeline = GST_ELEMENT (gst_element_get_parent (GST_ELEMENT (trans)));
  pipeline_clock = gst_pipeline_get_clock (GST_PIPELINE (pipeline));

  ASSESS_DB_LOCK ();

  assess_point_hash = g_hash_table_lookup (assess_db.hash,
      GST_ELEMENT_NAME (pipeline));
  if (assess_point_hash == NULL) {
    assess_point_hash = g_hash_table_new_full (g_str_hash, g_str_equal,
	g_free, g_free);
    g_hash_table_insert (assess_db.hash,
	g_strdup (GST_ELEMENT_NAME (pipeline)),	assess_point_hash);
  }

  assess_point = g_hash_table_lookup (assess_point_hash,
      GST_ELEMENT_NAME (this));
  if (assess_point == NULL) {
    assess_point = g_new0 (GstAssessPoint, 1);
    assess_point->name = g_strdup (GST_ELEMENT_NAME (this));
    g_hash_table_insert (assess_point_hash, assess_point->name, assess_point);
    g_mutex_init (&assess_point->lock);
  }

  ASSESS_DB_UNLOCK ();

  if (!assess_point) {
    goto end;
  }

  ASSESS_POINT_LOCK (assess_point);

  /*
  INFO ("%s.%s, %p, %lld, %lld, %lld, %lld, %lld",
      GST_ELEMENT_NAME (pipeline),
      GST_ELEMENT_NAME (trans), pipeline,
      GST_BUFFER_DURATION (buffer) / GST_MSECOND,
      GST_BUFFER_TIMESTAMP (buffer),
      GST_BUFFER_DTS (buffer),
      GST_BUFFER_OFFSET (buffer),
      GST_BUFFER_OFFSET_END (buffer));
  */

  if (GST_CLOCK_TIME_NONE != GST_BUFFER_DURATION (buffer)) {
    assess_point->dropped_time += GST_BUFFER_DURATION (buffer) / GST_MSECOND;
  }
  
  ASSESS_POINT_UNLOCK (assess_point);

 end:
  ret = GST_FLOW_OK;
  return ret;
}

static gboolean
gst_assess_sink_event (GstBaseTransform *trans, GstEvent *event)
{
  GstAssess *this = GST_ASSESS (trans);
  gboolean ret = TRUE;
  (void) this;
  ret = GST_BASE_TRANSFORM_CLASS (parent_class)->sink_event (trans, event);
  return ret;
}

static GstStateChangeReturn
gst_assess_change_state (GstElement * element, GstStateChange transition)
{
  GstStateChangeReturn ret;
  GstAssess *this = GST_ASSESS (element);

  (void) this;

  switch (transition) {
  case GST_STATE_CHANGE_READY_TO_PAUSED:
    break;
  default:
    break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);

  switch (transition) {
  case GST_STATE_CHANGE_PAUSED_TO_READY:
    //gst_assess_reset (this);
    break;
  default:
    break;
  }

  return ret;
}

static void
gst_assess_class_init (GstAssessClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
  GstBaseTransformClass *basetrans_class = GST_BASE_TRANSFORM_CLASS (klass);

  object_class->set_property = (GObjectSetPropertyFunc) gst_assess_set_property;
  object_class->get_property = (GObjectGetPropertyFunc) gst_assess_get_property;
  object_class->finalize = (GObjectFinalizeFunc) gst_assess_finalize;
  object_class->dispose = (GObjectFinalizeFunc) gst_assess_dispose;

  gst_element_class_set_static_metadata (element_class,
      "Stream Assessment", "Element",
      "Assess streams for frame drop and latency",
      "Duzy Chan <code@duzy.info>");

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_assess_sink_factory));

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_assess_src_factory));

  //element_class->request_new_pad = gst_assess_request_new_pad;
  //element_class->release_pad = gst_assess_release_pad;
  element_class->change_state = GST_DEBUG_FUNCPTR (gst_assess_change_state);

  basetrans_class->sink_event = GST_DEBUG_FUNCPTR (gst_assess_sink_event);
  basetrans_class->transform_ip = GST_DEBUG_FUNCPTR (gst_assess_transform);
  basetrans_class->start = GST_DEBUG_FUNCPTR (gst_assess_start);
  basetrans_class->stop = GST_DEBUG_FUNCPTR (gst_assess_stop);

  if (assess_db.hash == NULL) {
    ASSESS_DB_LOCK ();
    if (assess_db.hash == NULL) {
      assess_db.hash = g_hash_table_new_full (g_str_hash, g_str_equal,
	  g_free, g_hash_table_destroy);
      assess_db.timer = g_timeout_add (1000,
	  (GSourceFunc) assess_db_timeout, NULL);
    }
    ASSESS_DB_UNLOCK ();
  }

  GST_DEBUG_CATEGORY_INIT (gst_assess_debug, "assess", 0, "Assess");
}
