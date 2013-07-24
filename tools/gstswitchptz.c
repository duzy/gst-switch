/* GstSwitch							    -*- c -*-
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <gst/video/videooverlay.h>
#include <gst/speakertrack/gstcamcontrol.h>
#include <stdlib.h>
#include "gstswitchptz.h"
#include <string.h>
#include <sys/time.h>
#include <math.h>

gboolean verbose;
const char *ptz_device_name = "/dev/ttyUSB0";
const char *ptz_video_name = "/dev/video0";
const char *ptz_control_protocol = "visca";

G_DEFINE_TYPE (GstSwitchPTZ, gst_switch_ptz, GST_TYPE_WORKER);

static void
gst_switch_ptz_quit (GstSwitchPTZ * ptz)
{
  gtk_main_quit ();
}

static void
gst_switch_ptz_window_closed (GtkWidget * widget, GdkEvent * event,
    gpointer data)
{
  GstSwitchPTZ *ptz = GST_SWITCH_PTZ (data);
  gst_switch_ptz_quit (ptz);
}

static void
gst_switch_ptz_fix_coords (GstSwitchPTZ * ptz)
{
#if 0
  if (ptz->x < 0.0)
    ptz->x = 0.0;
  if (1.0 < ptz->x)
    ptz->x = 1.0;

  if (ptz->y < 0.0)
    ptz->y = 0.0;
  if (1.0 < ptz->y)
    ptz->y = 1.0;

  if (ptz->z < 0.0)
    ptz->z = 0.0;
  if (1.0 < ptz->z)
    ptz->z = 1.0;
#else
  if (ptz->x < ptz->controller->pan_min)
    ptz->x = ptz->controller->pan_min;
  if (ptz->controller->pan_max < ptz->x)
    ptz->x = ptz->controller->pan_max;

  if (ptz->y < ptz->controller->tilt_min)
    ptz->y = ptz->controller->tilt_min;
  if (ptz->controller->tilt_max < ptz->y)
    ptz->y = ptz->controller->tilt_max;

  if (ptz->z < 0.0)
    ptz->z = 0.0;
  if (1.0 < ptz->z)
    ptz->z = 1.0;
#endif
}

//#define STEP 0.05
//#define STEP 1

static gboolean
gst_switch_ptz_update_camera (GstSwitchPTZ * ptz)
{
  static double pan, tilt, zoom;        //, pan_speed, tilt_speed, zoom_speed;
  static struct timeval timestamp = { 0, 0 };
  static struct timeval timestamp_z = { 0, 0 };
  struct timeval now = { 0 };
  suseconds_t millis, millis_z;
  gettimeofday (&now, NULL);
  if (timestamp.tv_usec == 0) {
    timestamp = timestamp_z = now;
    pan = ptz->x, tilt = ptz->y, zoom = ptz->z;
    return TRUE;
  }

  millis =
      (now.tv_sec - timestamp.tv_sec) * 1000 + (now.tv_usec -
      timestamp.tv_usec) / 1000;
  millis_z =
      (now.tv_sec - timestamp_z.tv_sec) * 1000 + (now.tv_usec -
      timestamp_z.tv_usec) / 1000;

  //g_print ("%lld, %lld; %f, %f, %f; %f, %f, %f\n", millis, millis_z, pan, tilt, zoom, ptz->x, ptz->y, ptz->z);

  if (ptz->controller) {
    const double d = 0.0001;
    gst_switch_ptz_fix_coords (ptz);
    if (d < fabs (ptz->x - pan) || d < fabs (ptz->y - tilt)) {
      if (700 < millis) {
        gst_cam_controller_move (ptz->controller,
            gtk_adjustment_get_value (ptz->adjust_pan_speed), (pan = ptz->x),
            gtk_adjustment_get_value (ptz->adjust_tilt_speed), (tilt = ptz->y));
        timestamp = now;
      }
    }
    if (d < fabs (ptz->z - zoom)) {
      if (700 < millis_z) {
        gst_cam_controller_zoom (ptz->controller,
            gtk_adjustment_get_value (ptz->adjust_zoom_speed), (zoom = ptz->z));
        timestamp_z = now;
      }
    }
  }

  return TRUE;
}

static void
gst_switch_ptz_button_clicked_left (GtkButton * button, GstSwitchPTZ * ptz)
{
  ptz->x -= ptz->step;          //STEP;
  /*
     if (ptz->controller) {
     gst_switch_ptz_fix_coords (ptz);
     gst_cam_controller_move (ptz->controller,
     gtk_adjustment_get_value (ptz->adjust_pan_speed), ptz->x,
     gtk_adjustment_get_value (ptz->adjust_tilt_speed), ptz->y);
     }
   */
  g_print ("left: (%f, %f, %f)\n", ptz->x, ptz->y, ptz->z);
  gtk_adjustment_set_value (ptz->adjust_pan, ptz->x);
  gtk_adjustment_set_value (ptz->adjust_tilt, ptz->y);
}

static void
gst_switch_ptz_button_clicked_left_top (GtkButton * button, GstSwitchPTZ * ptz)
{
  ptz->x -= ptz->step;          //STEP;
  ptz->y += ptz->step;          //STEP;
  /*
     if (ptz->controller) {
     gst_switch_ptz_fix_coords (ptz);
     gst_cam_controller_move (ptz->controller,
     gtk_adjustment_get_value (ptz->adjust_pan_speed), ptz->x,
     gtk_adjustment_get_value (ptz->adjust_tilt_speed), ptz->y);
     }
   */
  g_print ("left-top: (%f, %f, %f)\n", ptz->x, ptz->y, ptz->z);
  gtk_adjustment_set_value (ptz->adjust_pan, ptz->x);
  gtk_adjustment_set_value (ptz->adjust_tilt, ptz->y);
}

static void
gst_switch_ptz_button_clicked_top (GtkButton * button, GstSwitchPTZ * ptz)
{
  ptz->y += ptz->step;          //STEP;
  /*
     if (ptz->controller) {
     gst_switch_ptz_fix_coords (ptz);
     gst_cam_controller_move (ptz->controller,
     gtk_adjustment_get_value (ptz->adjust_pan_speed), ptz->x,
     gtk_adjustment_get_value (ptz->adjust_tilt_speed), ptz->y);
     }
   */
  g_print ("top: (%f, %f, %f)\n", ptz->x, ptz->y, ptz->z);
  gtk_adjustment_set_value (ptz->adjust_pan, ptz->x);
  gtk_adjustment_set_value (ptz->adjust_tilt, ptz->y);
}

static void
gst_switch_ptz_button_clicked_top_right (GtkButton * button, GstSwitchPTZ * ptz)
{
  ptz->x += ptz->step;          //STEP;
  ptz->y += ptz->step;          //STEP;
  /*
     if (ptz->controller) {
     gst_switch_ptz_fix_coords (ptz);
     gst_cam_controller_move (ptz->controller,
     gtk_adjustment_get_value (ptz->adjust_pan_speed), ptz->x,
     gtk_adjustment_get_value (ptz->adjust_tilt_speed), ptz->y);
     }
   */
  g_print ("top-right: (%f, %f, %f)\n", ptz->x, ptz->y, ptz->z);
  gtk_adjustment_set_value (ptz->adjust_pan, ptz->x);
  gtk_adjustment_set_value (ptz->adjust_tilt, ptz->y);
}

static void
gst_switch_ptz_button_clicked_right (GtkButton * button, GstSwitchPTZ * ptz)
{
  ptz->x += ptz->step;          //STEP;
  /*
     if (ptz->controller) {
     gst_switch_ptz_fix_coords (ptz);
     gst_cam_controller_move (ptz->controller,
     gtk_adjustment_get_value (ptz->adjust_pan_speed), ptz->x,
     gtk_adjustment_get_value (ptz->adjust_tilt_speed), ptz->y);
     }
   */
  g_print ("right: (%f, %f, %f)\n", ptz->x, ptz->y, ptz->z);
  gtk_adjustment_set_value (ptz->adjust_pan, ptz->x);
  gtk_adjustment_set_value (ptz->adjust_tilt, ptz->y);
}

static void
gst_switch_ptz_button_clicked_right_bottom (GtkButton * button,
    GstSwitchPTZ * ptz)
{
  ptz->x += ptz->step;          //STEP;
  ptz->y -= ptz->step;          //STEP;
  /*
     if (ptz->controller) {
     gst_switch_ptz_fix_coords (ptz);
     gst_cam_controller_move (ptz->controller,
     gtk_adjustment_get_value (ptz->adjust_pan_speed), ptz->x,
     gtk_adjustment_get_value (ptz->adjust_tilt_speed), ptz->y);
     }
   */
  g_print ("right-bottom: (%f, %f, %f)\n", ptz->x, ptz->y, ptz->z);
  gtk_adjustment_set_value (ptz->adjust_pan, ptz->x);
  gtk_adjustment_set_value (ptz->adjust_tilt, ptz->y);
}

static void
gst_switch_ptz_button_clicked_bottom (GtkButton * button, GstSwitchPTZ * ptz)
{
  ptz->y -= ptz->step;          //STEP;
  /*
     if (ptz->controller) {
     gst_switch_ptz_fix_coords (ptz);
     gst_cam_controller_move (ptz->controller,
     gtk_adjustment_get_value (ptz->adjust_pan_speed), ptz->x,
     gtk_adjustment_get_value (ptz->adjust_tilt_speed), ptz->y);
     }
   */
  g_print ("bottom: (%f, %f, %f)\n", ptz->x, ptz->y, ptz->z);
  gtk_adjustment_set_value (ptz->adjust_pan, ptz->x);
  gtk_adjustment_set_value (ptz->adjust_tilt, ptz->y);
}

static void
gst_switch_ptz_button_clicked_bottom_left (GtkButton * button,
    GstSwitchPTZ * ptz)
{
  ptz->x -= ptz->step;          //STEP;
  ptz->y -= ptz->step;          //STEP;
  /*
     if (ptz->controller) {
     gst_switch_ptz_fix_coords (ptz);
     gst_cam_controller_move (ptz->controller,
     gtk_adjustment_get_value (ptz->adjust_pan_speed), ptz->x,
     gtk_adjustment_get_value (ptz->adjust_tilt_speed), ptz->y);
     }
   */
  g_print ("bottom-left: (%f, %f, %f)\n", ptz->x, ptz->y, ptz->z);
  gtk_adjustment_set_value (ptz->adjust_pan, ptz->x);
  gtk_adjustment_set_value (ptz->adjust_tilt, ptz->y);
}

static void
gst_switch_ptz_button_clicked_center (GtkButton * button, GstSwitchPTZ * ptz)
{
  ptz->x = 0;                   //0.50;
  ptz->y = 0;                   //0.50;
  /*
     if (ptz->controller) {
     gst_switch_ptz_fix_coords (ptz);
     gst_cam_controller_move (ptz->controller,
     gtk_adjustment_get_value (ptz->adjust_pan_speed), ptz->x,
     gtk_adjustment_get_value (ptz->adjust_tilt_speed), ptz->y);
     }
   */
  g_print ("center: (%f, %f, %f)\n", ptz->x, ptz->y, ptz->z);
  gtk_adjustment_set_value (ptz->adjust_pan, ptz->x);
  gtk_adjustment_set_value (ptz->adjust_tilt, ptz->y);
}

static void
gst_switch_ptz_button_clicked_zoom_minus (GtkButton * button,
    GstSwitchPTZ * ptz)
{
  ptz->z -= 0.05;               //STEP;
  /*
     if (ptz->controller) {
     gst_switch_ptz_fix_coords (ptz);
     gst_cam_controller_zoom (ptz->controller,
     gtk_adjustment_get_value (ptz->adjust_zoom_speed), ptz->z);
     //gtk_adjustment_set_value (ptz->adjust_zoom_speed, ptz->z);
     }
   */
  g_print ("zoom: (%f, %f, %f)\n", ptz->x, ptz->y, ptz->z);
  gtk_adjustment_set_value (ptz->adjust_zoom, ptz->z);
}

static void
gst_switch_ptz_button_clicked_zoom_reset (GtkButton * button,
    GstSwitchPTZ * ptz)
{
  ptz->z = 0.50;
  /*
     if (ptz->controller) {
     gst_switch_ptz_fix_coords (ptz);
     gst_cam_controller_zoom (ptz->controller,
     gtk_adjustment_get_value (ptz->adjust_zoom_speed), ptz->z);
     //gtk_adjustment_set_value (ptz->adjust_zoom_speed, ptz->z);
     }
   */
  g_print ("zoom: (%f, %f, %f)\n", ptz->x, ptz->y, ptz->z);
  gtk_adjustment_set_value (ptz->adjust_zoom, ptz->z);
}

static void
gst_switch_ptz_button_clicked_zoom_plus (GtkButton * button, GstSwitchPTZ * ptz)
{
  ptz->z += 0.05;               //STEP;
  /*
     if (ptz->controller) {
     gst_switch_ptz_fix_coords (ptz);
     gst_cam_controller_zoom (ptz->controller,
     gtk_adjustment_get_value (ptz->adjust_zoom_speed), ptz->z);
     //gtk_adjustment_set_value (ptz->adjust_zoom_speed, ptz->z);
     }
   */
  g_print ("zoom: (%f, %f, %f)\n", ptz->x, ptz->y, ptz->z);
  gtk_adjustment_set_value (ptz->adjust_zoom, ptz->z);
}

static void
gst_switch_ptz_zoom_speed_changed (GtkAdjustment * adjustment,
    GstSwitchPTZ * ptz)
{
}

static void
gst_switch_ptz_pan_speed_changed (GtkAdjustment * adjustment,
    GstSwitchPTZ * ptz)
{
}

static void
gst_switch_ptz_tilt_speed_changed (GtkAdjustment * adjustment,
    GstSwitchPTZ * ptz)
{
}

static void
gst_switch_ptz_pan_changed (GtkAdjustment * adjustment, GstSwitchPTZ * ptz)
{
  ptz->x = gtk_adjustment_get_value (adjustment);
  /*
     if (ptz->controller) {
     gst_switch_ptz_fix_coords (ptz);
     gst_cam_controller_move (ptz->controller,
     gtk_adjustment_get_value (ptz->adjust_pan_speed), ptz->x,
     gtk_adjustment_get_value (ptz->adjust_tilt_speed), ptz->y);
     }
   */
  g_print ("pan: (%f, %f, %f)\n", ptz->x, ptz->y, ptz->z);
}

static void
gst_switch_ptz_tilt_changed (GtkAdjustment * adjustment, GstSwitchPTZ * ptz)
{
  ptz->y = gtk_adjustment_get_value (adjustment);
  /*
     if (ptz->controller) {
     gst_switch_ptz_fix_coords (ptz);
     gst_cam_controller_move (ptz->controller,
     gtk_adjustment_get_value (ptz->adjust_pan_speed), ptz->x,
     gtk_adjustment_get_value (ptz->adjust_tilt_speed), ptz->y);
     }
   */
  g_print ("tilt: (%f, %f, %f)\n", ptz->x, ptz->y, ptz->z);
}

static void
gst_switch_ptz_zoom_changed (GtkAdjustment * adjustment, GstSwitchPTZ * ptz)
{
  ptz->z = gtk_adjustment_get_value (adjustment);
  /*
     if (ptz->controller) {
     gst_switch_ptz_fix_coords (ptz);
     gst_cam_controller_zoom (ptz->controller,
     gtk_adjustment_get_value (ptz->adjust_zoom_speed), ptz->z);
     }
   */
  g_print ("zoom: (%f, %f, %f)\n", ptz->x, ptz->y, ptz->z);
}

static void
gst_switch_ptz_init (GstSwitchPTZ * ptz)
{
  GtkWidget *box_main, *box_video, *box_control;
  GtkWidget *box_control_pan, *box_control_tilt, *box_control_zoom;
  GtkWidget *pan_scale, *tilt_scale, *control_grid;
  GtkWidget *control_buttons[3][3] = { {NULL} };
  GtkWidget *box_zoom, *zoom_minus, *zoom_reset, *zoom_plus;
  GtkWidget *scrollwin;
  GtkWidget *scale_zoom;
  GtkWidget *scale_pan_speed, *scale_tilt_speed, *scale_zoom_speed;
  GtkWidget *label_pan_speed, *label_tilt_speed, *label_zoom_speed;
  const gchar *control_labels[3][3] = {
    {"  \\  ", "  ^  ", "  /  "},
    {"  <  ", "  *  ", "  >  "},
    {"  /  ", "  v  ", "  \\  "},
  };
  int n, m;

  ptz->step = 1;
  ptz->x = ptz->y = ptz->z = 0; //0.5;
  ptz->controller = gst_cam_controller_new (ptz_control_protocol);
  if (ptz->controller) {
    gst_cam_controller_open (ptz->controller, ptz_device_name);
  }

  ptz->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (ptz->window), 640, 480);
  gtk_window_set_title (GTK_WINDOW (ptz->window), "PTZ Control");
  g_signal_connect (G_OBJECT (ptz->window), "delete-event",
      G_CALLBACK (gst_switch_ptz_window_closed), ptz);

  box_main = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
  box_video = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
  box_control = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);

  scrollwin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollwin),
      GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_size_request (scrollwin, 100, -1);
  gtk_widget_set_vexpand (scrollwin, TRUE);
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrollwin),
      box_control);

  gtk_box_pack_start (GTK_BOX (box_main), box_video, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (box_main), scrollwin, FALSE, TRUE, 0);
  gtk_container_add (GTK_CONTAINER (ptz->window), box_main);
  gtk_container_set_border_width (GTK_CONTAINER (ptz->window), 5);

  ptz->video_view = gtk_drawing_area_new ();
  gtk_widget_set_name (ptz->video_view, "video");
  gtk_widget_set_double_buffered (ptz->video_view, FALSE);
  gtk_widget_set_hexpand (ptz->video_view, TRUE);
  gtk_widget_set_vexpand (ptz->video_view, TRUE);
  gtk_widget_set_events (ptz->video_view, GDK_EXPOSURE_MASK
      | GDK_LEAVE_NOTIFY_MASK
      | GDK_BUTTON_PRESS_MASK
      | GDK_BUTTON_RELEASE_MASK
      | GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);
  gtk_box_pack_start (GTK_BOX (box_video), ptz->video_view, TRUE, TRUE, 0);

  ptz->adjust_pan = gtk_adjustment_new (0,
      ptz->controller->pan_min, ptz->controller->pan_max + 5, 1, 5, 5);
  ptz->adjust_tilt = gtk_adjustment_new (0,
      ptz->controller->tilt_min, ptz->controller->tilt_max + 5, 1, 5, 5);
  pan_scale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, ptz->adjust_pan);
  tilt_scale = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, ptz->adjust_tilt);
  gtk_box_pack_start (GTK_BOX (box_control), pan_scale, FALSE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (box_control), tilt_scale, FALSE, TRUE, 0);
  g_signal_connect (gtk_range_get_adjustment (pan_scale), "value-changed",
      G_CALLBACK (gst_switch_ptz_pan_changed), ptz);
  g_signal_connect (gtk_range_get_adjustment (tilt_scale), "value-changed",
      G_CALLBACK (gst_switch_ptz_tilt_changed), ptz);

  control_grid = gtk_grid_new ();
  gtk_grid_insert_row (GTK_GRID (control_grid), 0);
  gtk_grid_insert_row (GTK_GRID (control_grid), 1);
  gtk_grid_insert_row (GTK_GRID (control_grid), 2);
  gtk_grid_insert_column (GTK_GRID (control_grid), 0);
  gtk_grid_insert_column (GTK_GRID (control_grid), 1);
  gtk_grid_insert_column (GTK_GRID (control_grid), 2);
  for (n = 0; n < 3; ++n) {
    for (m = 0; m < 3; ++m) {
      GtkWidget *btn = control_buttons[m][n] = gtk_button_new ();
      gtk_grid_attach (GTK_GRID (control_grid), btn, n, m, 1, 1);
      gtk_widget_set_size_request (btn, 110, 110);
      gtk_button_set_label (GTK_BUTTON (btn), control_labels[m][n]);
    }
  }
  gtk_box_pack_start (GTK_BOX (box_control), control_grid, FALSE, TRUE, 0);

  box_zoom = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  zoom_minus = gtk_button_new_with_label ("-");
  zoom_reset = gtk_button_new_with_label ("*");
  zoom_plus = gtk_button_new_with_label ("+");
  gtk_widget_set_size_request (zoom_minus, 110, 50);
  gtk_widget_set_size_request (zoom_reset, 110, 50);
  gtk_widget_set_size_request (zoom_plus, 110, 50);
  gtk_box_pack_start (GTK_BOX (box_control), gtk_label_new ("zoom:"), TRUE,
      TRUE, 0);
  gtk_box_pack_start (GTK_BOX (box_zoom), zoom_minus, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (box_zoom), zoom_reset, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (box_zoom), zoom_plus, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (box_control), box_zoom, FALSE, TRUE, 0);
  g_signal_connect (zoom_minus, "clicked",
      G_CALLBACK (gst_switch_ptz_button_clicked_zoom_minus), ptz);
  g_signal_connect (zoom_reset, "clicked",
      G_CALLBACK (gst_switch_ptz_button_clicked_zoom_reset), ptz);
  g_signal_connect (zoom_plus, "clicked",
      G_CALLBACK (gst_switch_ptz_button_clicked_zoom_plus), ptz);

  ptz->adjust_zoom = gtk_adjustment_new (0.50, 0.0, 1.00, 0.01, 0.01, 0.05);
  scale_zoom = gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, ptz->adjust_zoom);
  gtk_box_pack_start (GTK_BOX (box_control), scale_zoom, FALSE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (box_control), gtk_label_new (""), TRUE, TRUE, 0);

  ptz->adjust_pan_speed =
      gtk_adjustment_new (50, 0, 120, 5, 5,
      5 /*0.50, 0.0, 1.00, 0.01, 0.01, 0.05 */ );
  ptz->adjust_tilt_speed =
      gtk_adjustment_new (50, 0, 120, 5, 5,
      5 /*0.50, 0.0, 1.00, 0.01, 0.01, 0.05 */ );
  ptz->adjust_zoom_speed =
      gtk_adjustment_new (0.50, 0.0, 1.00, 0.01, 0.01, 0.05);
  scale_pan_speed =
      gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, ptz->adjust_pan_speed);
  scale_tilt_speed =
      gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, ptz->adjust_tilt_speed);
  scale_zoom_speed =
      gtk_scale_new (GTK_ORIENTATION_HORIZONTAL, ptz->adjust_zoom_speed);
  label_pan_speed = gtk_label_new ("P:");
  label_tilt_speed = gtk_label_new ("T:");
  label_zoom_speed = gtk_label_new ("Z:");
  box_control_pan = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  box_control_tilt = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  box_control_zoom = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start (GTK_BOX (box_control), gtk_label_new ("Speeds:"), FALSE,
      TRUE, 0);
  gtk_box_pack_start (GTK_BOX (box_control_pan), label_pan_speed, FALSE, FALSE,
      0);
  gtk_box_pack_start (GTK_BOX (box_control_pan), scale_pan_speed, TRUE, TRUE,
      0);
  gtk_box_pack_start (GTK_BOX (box_control_tilt), label_tilt_speed, FALSE,
      FALSE, 0);
  gtk_box_pack_start (GTK_BOX (box_control_tilt), scale_tilt_speed, TRUE, TRUE,
      0);
  gtk_box_pack_start (GTK_BOX (box_control_zoom), label_zoom_speed, FALSE,
      FALSE, 0);
  gtk_box_pack_start (GTK_BOX (box_control_zoom), scale_zoom_speed, TRUE, TRUE,
      0);
  gtk_box_pack_start (GTK_BOX (box_control), box_control_pan, FALSE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (box_control), box_control_tilt, FALSE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (box_control), box_control_zoom, FALSE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (box_control), gtk_label_new (""), TRUE, TRUE, 0);

  if (strcmp (ptz_control_protocol, "visca-sony") == 0) {
    gtk_widget_set_sensitive (box_control_pan, FALSE);
    gtk_widget_set_sensitive (box_control_tilt, FALSE);
    gtk_widget_set_sensitive (box_control_zoom, FALSE);
  }

  /*
     g_signal_connect (G_OBJECT (scale_pan_speed), "value-changed",
     G_CALLBACK (gst_switch_ptz_window_closed), ptz);
   */
  g_signal_connect (ptz->adjust_pan_speed, "value-changed",
      G_CALLBACK (gst_switch_ptz_pan_speed_changed), ptz);
  g_signal_connect (ptz->adjust_tilt_speed, "value-changed",
      G_CALLBACK (gst_switch_ptz_tilt_speed_changed), ptz);
  g_signal_connect (ptz->adjust_zoom_speed, "value-changed",
      G_CALLBACK (gst_switch_ptz_zoom_speed_changed), ptz);
  g_signal_connect (ptz->adjust_zoom, "value-changed",
      G_CALLBACK (gst_switch_ptz_zoom_changed), ptz);

  g_signal_connect (control_buttons[0][0], "pressed",
      G_CALLBACK (gst_switch_ptz_button_clicked_left_top), ptz);
  g_signal_connect (control_buttons[0][1], "pressed",
      G_CALLBACK (gst_switch_ptz_button_clicked_top), ptz);
  g_signal_connect (control_buttons[0][2], "pressed",
      G_CALLBACK (gst_switch_ptz_button_clicked_top_right), ptz);
  g_signal_connect (control_buttons[1][0], "pressed",
      G_CALLBACK (gst_switch_ptz_button_clicked_left), ptz);
  g_signal_connect (control_buttons[1][1], "pressed",
      G_CALLBACK (gst_switch_ptz_button_clicked_center), ptz);
  g_signal_connect (control_buttons[1][2], "pressed",
      G_CALLBACK (gst_switch_ptz_button_clicked_right), ptz);
  g_signal_connect (control_buttons[2][0], "pressed",
      G_CALLBACK (gst_switch_ptz_button_clicked_bottom_left), ptz);
  g_signal_connect (control_buttons[2][1], "pressed",
      G_CALLBACK (gst_switch_ptz_button_clicked_bottom), ptz);
  g_signal_connect (control_buttons[2][2], "pressed",
      G_CALLBACK (gst_switch_ptz_button_clicked_right_bottom), ptz);

  g_timeout_add (250, (GSourceFunc) gst_switch_ptz_update_camera, ptz);
}

static void
gst_switch_ptz_fini (GstSwitchPTZ * ptz)
{
  if (G_OBJECT_CLASS (gst_switch_ptz_parent_class)->finalize)
    (*G_OBJECT_CLASS (gst_switch_ptz_parent_class)->finalize) (G_OBJECT (ptz));
}

static gboolean
gst_switch_ptz_prepare (GstSwitchPTZ * ptz)
{
  GstWorker *worker = GST_WORKER (ptz);
  GstElement *sink = gst_worker_get_element_unlocked (worker, "sink");
  gulong handle = GDK_WINDOW_XID (gtk_widget_get_window (ptz->video_view));
  g_return_val_if_fail (GST_IS_ELEMENT (sink), FALSE);
  gst_video_overlay_set_window_handle (GST_VIDEO_OVERLAY (sink), handle);
  gst_object_unref (sink);
  return TRUE;
}

static GString *
gst_switch_ptz_get_pipeline (GstSwitchPTZ * ptz)
{
  GString *desc;

  desc = g_string_new ("");

  g_string_append_printf (desc, "v4l2src device=%s ", ptz_video_name);
  g_string_append_printf (desc, "! videoconvert ");
  g_string_append_printf (desc, "! xvimagesink name=sink ");
  return desc;
}

static void
gst_switch_ptz_run (GstSwitchPTZ * ptz)
{
  gtk_widget_show_all (ptz->window);
  gst_worker_start (GST_WORKER (ptz));
  gtk_main ();
}

static int
gst_switch_ptz_main (int argc, char **argv)
{
  GstSwitchPTZ *ptz = GST_SWITCH_PTZ (g_object_new (GST_TYPE_SWITCH_PTZ, NULL));

  gst_switch_ptz_run (ptz);

  g_object_unref (G_OBJECT (ptz));
  return 0;
}

static void
gst_switch_ptz_class_init (GstSwitchPTZClass * klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstWorkerClass *worker_class = GST_WORKER_CLASS (klass);
  object_class->finalize = (GObjectFinalizeFunc) gst_switch_ptz_fini;
  worker_class->prepare = (GstWorkerPrepareFunc) gst_switch_ptz_prepare;
  worker_class->get_pipeline_string = (GstWorkerGetPipelineStringFunc)
      gst_switch_ptz_get_pipeline;
}

static GOptionEntry entries[] = {
  {"verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Be verbose", NULL},
  {"device", 'd', 0, G_OPTION_ARG_STRING, &ptz_device_name,
      "PTZ camera control device", "DEVICE"},
  {"protocol", 'p', 0, G_OPTION_ARG_STRING, &ptz_control_protocol,
      "PTZ camera control protocol", "NAME"},
  {"video", 'v', 0, G_OPTION_ARG_STRING, &ptz_video_name,
      "Camera video name (default /dev/video0)", "NAME"},
  {NULL}
};

static void
gst_switch_ptz_args (int *argc, char **argv[])
{
  GOptionContext *context;
  GError *error = NULL;
  context = g_option_context_new ("");
  g_option_context_add_main_entries (context, entries, "gst-switch-ptz");
  g_option_context_add_group (context, gst_init_get_option_group ());
  if (!g_option_context_parse (context, argc, argv, &error)) {
    g_print ("option parsing failed: %s\n", error->message);
    exit (1);
  }
  g_option_context_free (context);
}

int
main (int argc, char **argv)
{
  gst_switch_ptz_args (&argc, &argv);
  gtk_init (&argc, &argv);
  return gst_switch_ptz_main (argc, argv);
}
