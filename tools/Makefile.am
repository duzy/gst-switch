bin_PROGRAMS = gst-switch-srv gst-switch-ui

gst_switch_srv_SOURCES = gstworker.c gstswitchsrv.c \
  gstswitcher.c gstcompositor.c
gst_switch_srv_CFLAGS = $(GST_CFLAGS) $(GST_BASE_CFLAGS) \
  $(GST_PLUGINS_BASE_CFLAGS) -DLOG_PREFIX="\"./tools\""
gst_switch_srv_LDFLAGS = $(GST_LIBS) $(GST_BASE_LIBS) $(GST_PLUGINS_BASE_LIBS) $(GSTPB_BASE_LIBS)
gst_switch_srv_LDADD = $(GST_LIBS) $(LIBM)

gst_switch_ui_SOURCES = gstswitchui.c
gst_switch_ui_CFLAGS = $(GST_CFLAGS) $(GST_BASE_CFLAGS) \
  $(GST_PLUGINS_BASE_CFLAGS) $(X_CFLAGS) $(GTK_CFLAGS) \
  -DLOG_PREFIX="\"./tools\""
gst_switch_ui_LDFLAGS = $(GST_LIBS) $(GST_BASE_LIBS) $(GST_PLUGINS_BASE_LIBS) $(GSTPB_BASE_LIBS)
gst_switch_ui_LDADD = $(GST_LIBS) $(X_LIBS) $(LIBM) $(GTK_LIBS) $(GLIB_LIBS)

