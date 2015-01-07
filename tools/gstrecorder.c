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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "gstswitchserver.h"
#include "gstcomposite.h"
#include "gstrecorder.h"

enum
{
  PROP_0,
  PROP_MODE,
  PROP_PORT,
  PROP_WIDTH,
  PROP_HEIGHT,
};

enum
{
  SIGNAL__LAST,
};

//static guint gst_recorder_signals[SIGNAL__LAST] = { 0 };
extern gboolean verbose;

#define parent_class gst_recorder_parent_class

G_DEFINE_TYPE (GstRecorder, gst_recorder, GST_TYPE_WORKER);

/**
 * @brief Initialize the GstRecorder instance.
 * @param rec The GstRecorder instance.
 * @memberof GstRecorder
 */
static void
gst_recorder_init (GstRecorder * rec)
{
  rec->sink_port = 0;
  rec->mode = 0;
  rec->width = 0;
  rec->height = 0;

  //INFO ("init %p", rec);
}

/**
 * @brief Invoked to unref objects.
 * @param rec The GstRecorder instance.
 * @memberof GstRecorder
 * @see GObject
 */
static void
gst_recorder_dispose (GstRecorder * rec)
{
  INFO ("dispose %p", rec);
  G_OBJECT_CLASS (parent_class)->dispose (G_OBJECT (rec));
}

/**
 * @param rec The GstRecorder instance.
 * @memberof GstRecorder
 *
 * Destroying the GstRecorder instance.
 *
 * @see GObject
 */
static void
gst_recorder_finalize (GstRecorder * rec)
{
  if (G_OBJECT_CLASS (parent_class)->finalize)
    (*G_OBJECT_CLASS (parent_class)->finalize) (G_OBJECT (rec));
}

/**
 * @param rec The GstRecorder instance.
 * @param property_id
 * @param value
 * @param pspec
 * @memberof GstRecorder
 *
 * Fetching the GstRecorder property.
 *
 * @see GObject
 */
static void
gst_recorder_get_property (GstRecorder * rec, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  switch (property_id) {
    case PROP_MODE:
      g_value_set_uint (value, rec->mode);
      break;
    case PROP_PORT:
      g_value_set_uint (value, rec->sink_port);
      break;
    case PROP_WIDTH:
      g_value_set_uint (value, rec->width);
      break;
    case PROP_HEIGHT:
      g_value_set_uint (value, rec->height);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (rec, property_id, pspec);
      break;
  }
}

/**
 * @param rec The GstRecorder instance.
 * @param property_id
 * @param value
 * @param pspec
 * @memberof GstRecorder
 *
 * Changing the GstRecorder properties.
 *
 * @see GObject
 */
static void
gst_recorder_set_property (GstRecorder * rec, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  switch (property_id) {
    case PROP_MODE:
      rec->mode = (GstCompositeMode) (g_value_get_uint (value));
      break;
    case PROP_PORT:
      rec->sink_port = g_value_get_uint (value);
      break;
    case PROP_WIDTH:
      rec->width = g_value_get_uint (value);
      break;
    case PROP_HEIGHT:
      rec->height = g_value_get_uint (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (G_OBJECT (rec), property_id, pspec);
      break;
  }
}

/*
 * @param dir - directory to create
 * @return nothing
 * Create a directory and all intermediary directories
 * if necessary. Note that errors are ignored here, if
 * the resulting path is in fact unusable having early
 * warning here is not necessary
 */
static void
gst_recorder_mkdirs (const char *dir)
{
  char tmp[256];
  strncpy (tmp, dir, sizeof (tmp));
  size_t len = strlen (tmp);
  if (len > 0) {
    if (tmp[len - 1] == '/')
      tmp[len--] = 0;
    if (len > 0) {
      size_t at = 1;            // skip leading slash
      while (at < len) {
        char *p = strchr (tmp + at, '/');
        if (p != NULL && *p == '/') {
          *p = '\0';
          mkdir (tmp, S_IRWXU);
          *p = '/';
          at = p - tmp + 1;
        } else
          at = len;
      }
      mkdir (tmp, S_IRWXU);
    }
  }
}

/**
 * @param filepath - file file/path of a unix file
 * @return length of the path potion of the file, excluding the separator
 */
static size_t
gst_recorder_pathlen (const char *filepath)
{
  if (filepath != NULL && strlen (filepath) > 0) {
    char const *sep = strrchr (filepath + 1, '/');
    if (sep != NULL)
      return sep - filepath;
  }
  return 0;
}


/**
 * @param filename Template name of the file to save
 * @return the file name string, need to be freed after used
 *
 * This is used to generate a new recording file name for the recorder.
 */
static const gchar *
gst_recorder_new_filename (const gchar * filename)
{
  if (!filename)
    return NULL;

  gchar fnbuf[256];
  time_t t = time (NULL);
  struct tm *tm = localtime (&t);
  // Note: reserve some space for collision suffix
  strftime (fnbuf, sizeof (fnbuf) - 5, filename, tm);
  // We now have a fully built name in our buffer
  // If there is at least one directory present, make sure they exist
  size_t pathlen = gst_recorder_pathlen (fnbuf);
  if (pathlen > 0) {
    fnbuf[pathlen] = '\0';
    gst_recorder_mkdirs (fnbuf);
    fnbuf[pathlen] = '/';
  }
  pathlen = strlen (fnbuf);     // reuse for length of file/path

  // handle name collisions by adding a suffix/extension
  size_t suffix = 0;
  while (1) {
    struct stat s;
    if (-1 == stat (fnbuf, &s)) {
      if (ENOENT == errno)
        break;
      else {
        perror (fnbuf);
        return NULL;            // can't record
      }
    }
    snprintf (fnbuf + pathlen, 256 - pathlen, ".%03d", (int) suffix++);
    // can't record if we've used up our additions
    if (suffix > 999)
      return NULL;
  }

  return g_strdup (fnbuf);
}

/**
 * @param rec The GstRecorder instance.
 * @memberof GstRecorder
 * @return The recorder pipeline string, needs freeing when used
 *
 * Fetching the recorder pipeline invoked by the GstWorker.
 */
static GString *
gst_recorder_get_pipeline_string (GstRecorder * rec)
{
  const gchar *filename = gst_recorder_new_filename (opts.record_filename);
  GString *desc;

  //INFO ("Recording to %s and port %d", filename, rec->sink_port);

  desc = g_string_new ("");

  g_string_append_printf (desc, "intervideosrc name=source_video "
      "channel=composite_video ");
  g_string_append_printf (desc, "interaudiosrc name=source_audio "
      "channel=composite_audio ");

  g_string_append_printf (desc,
      "source_video. ! video/x-raw,width=%d,height=%d ",
      rec->width, rec->height);
  /*
     ASSESS ("assess-record-video-source");
   */
  g_string_append_printf (desc, "! queue2 ");
  /*
     ASSESS ("assess-record-video-encode-queued");
   */
  g_string_append_printf (desc, "! vp8enc ");
  /*
     ASSESS ("assess-record-video-encoded");
   */
  g_string_append_printf (desc, "! mux. ");

  g_string_append_printf (desc, "source_audio. ");
  /*
     ASSESS ("assess-record-audio-source");
   */
  g_string_append_printf (desc, "! queue2 ");
  /*
     ASSESS ("assess-record-audio-queued");
   */
  g_string_append_printf (desc, "! voaacenc ");
  /*
     ASSESS ("assess-record-audio-encoded");
   */
  g_string_append_printf (desc, "! mux. ");

  g_string_append_printf (desc, "avimux name=mux ");
  /*
     ASSESS ("assess-record-mux-result");
   */
  g_string_append_printf (desc, "! tee name=result ");

  if (filename) {
    g_string_append_printf (desc, "filesink name=disk_sink sync=false "
        "location=\"%s\" ", filename);
    g_free ((gpointer) filename);
    g_string_append_printf (desc, "result. ");
    /*
       ASSESS ("assess-record-file-to-queue");
     */
    g_string_append_printf (desc, "! queue2 ");
    /*
       ASSESS ("assess-record-file-to-sink");
     */
    g_string_append_printf (desc, "! disk_sink. ");
  }

  g_string_append_printf (desc, "tcpserversink name=tcp_sink sync=false "
      "port=%d ", rec->sink_port);
  g_string_append_printf (desc, "result. ");
  /*
     ASSESS ("assess-record-tcp-to-queue");
   */
  g_string_append_printf (desc, "! queue2 ");
  /*
     ASSESS ("assess-record-tcp-to-sink");
   */
  g_string_append_printf (desc, "! gdppay ! tcp_sink. ");
  return desc;
}

/**
 * @param rec The GstRecorder instance.
 * @param element
 * @param socket
 * @memberof GstRecorder
 *
 * Invoked when client socket added on the encoding out port.
 */
static void
gst_recorder_client_socket_added (GstElement * element,
    GSocket * socket, GstRecorder * rec)
{
  g_return_if_fail (G_IS_SOCKET (socket));

  INFO ("client-socket-added: %d", g_socket_get_fd (socket));
}

/**
 * @param rec The GstRecorder instance.
 * @param element
 * @param socket
 * @memberof GstRecorder
 *
 * Invoked when the client socket on the encoding out port is closed. We need
 * to manually close the socket to avoid FD leaks.
 */
static void
gst_recorder_client_socket_removed (GstElement * element,
    GSocket * socket, GstRecorder * rec)
{
  g_return_if_fail (G_IS_SOCKET (socket));

  INFO ("client-socket-removed: %d", g_socket_get_fd (socket));

  g_socket_close (socket, NULL);
}

/**
 * @param rec The GstRecorder instance.
 * @memberof GstRecorder
 * @return TRUE indicating the recorder is prepared, FALSE otherwise.
 *
 * Invoked when the GstWorker is preparing the pipeline.
 */
static gboolean
gst_recorder_prepare (GstRecorder * rec)
{
  GstElement *tcp_sink = NULL;

  g_return_val_if_fail (GST_IS_RECORDER (rec), FALSE);

  tcp_sink = gst_worker_get_element_unlocked (GST_WORKER (rec), "tcp_sink");

  g_return_val_if_fail (GST_IS_ELEMENT (tcp_sink), FALSE);

  g_signal_connect (tcp_sink, "client-added",
      G_CALLBACK (gst_recorder_client_socket_added), rec);

  g_signal_connect (tcp_sink, "client-socket-removed",
      G_CALLBACK (gst_recorder_client_socket_removed), rec);

  gst_object_unref (tcp_sink);
  return TRUE;
}

/**
 * @brief Initialize the GstRecorderClass.
 * @param klass The GstRecorderClass instance.
 * @memberof GstRecorderClass
 */
static void
gst_recorder_class_init (GstRecorderClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstWorkerClass *worker_class = GST_WORKER_CLASS (klass);

  object_class->dispose = (GObjectFinalizeFunc) gst_recorder_dispose;
  object_class->finalize = (GObjectFinalizeFunc) gst_recorder_finalize;
  object_class->set_property =
      (GObjectSetPropertyFunc) gst_recorder_set_property;
  object_class->get_property =
      (GObjectGetPropertyFunc) gst_recorder_get_property;

  g_object_class_install_property (object_class, PROP_MODE,
      g_param_spec_uint ("mode", "Mode",
          "Composite Mode",
          COMPOSE_MODE_NONE,
          COMPOSE_MODE__LAST,
          COMPOSE_MODE_NONE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_PORT,
      g_param_spec_uint ("port", "Port",
          "Sink port",
          GST_SWITCH_MIN_SINK_PORT,
          GST_SWITCH_MAX_SINK_PORT,
          GST_SWITCH_MIN_SINK_PORT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_WIDTH,
      g_param_spec_uint ("width", "Input Width",
          "Input video frame width",
          1, G_MAXINT,
          GST_SWITCH_COMPOSITE_DEFAULT_WIDTH,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (object_class, PROP_HEIGHT,
      g_param_spec_uint ("height",
          "Input Height",
          "Input video frame height",
          1, G_MAXINT,
          GST_SWITCH_COMPOSITE_DEFAULT_HEIGHT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  worker_class->prepare = (GstWorkerPrepareFunc) gst_recorder_prepare;
  worker_class->get_pipeline_string = (GstWorkerGetPipelineStringFunc)
      gst_recorder_get_pipeline_string;
}
