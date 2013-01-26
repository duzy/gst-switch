bin_PROGRAMS = gst-switch-srv gst-switch-ui

if GCOV_ENABLED
# --coverage
#GCOV_CFLAGS = -fprofile-arcs -ftest-coverage
#GCOV_LFLAGS = -fprofile-arcs -ftest-coverage
GCOV_CFLAGS = --coverage
GCOV_LFLAGS = --coverage
endif

gst_switch_srv_SOURCES = gstworker.c gstswitchserver.c gstcase.c \
  gstcomposite.c gstswitchcontroller.c gstrecorder.c \
  gio/gsocketinputstream.c
gst_switch_srv_CFLAGS = $(GST_CFLAGS) $(GST_BASE_CFLAGS) $(GCOV_CFLAGS) \
  $(GST_PLUGINS_BASE_CFLAGS) -DLOG_PREFIX="\"./tools\""
gst_switch_srv_LDFLAGS = $(GCOV_LFLAGS) $(GST_LIBS) $(GST_BASE_LIBS) \
  $(GST_PLUGINS_BASE_LIBS) $(GSTPB_BASE_LIBS)
gst_switch_srv_LDADD = $(GIO_LIBS) $(LIBM)

gst_switch_ui_SOURCES = gstworker.c gstswitchui.c gstvideodisp.c \
  gstaudiovisual.c gstswitchclient.c
gst_switch_ui_CFLAGS = $(GST_CFLAGS) $(GST_BASE_CFLAGS) $(GCOV_CFLAGS) \
  $(GST_PLUGINS_BASE_CFLAGS) $(X_CFLAGS) $(GTK_CFLAGS) \
  -DLOG_PREFIX="\"./tools\""
gst_switch_ui_LDFLAGS = $(GCOV_LFLAGS) $(GST_LIBS) $(GST_BASE_LIBS) \
  $(GST_PLUGINS_BASE_LIBS) $(GSTPB_BASE_LIBS)
gst_switch_ui_LDADD = $(GST_LIBS) $(X_LIBS) $(LIBM) $(GTK_LIBS) $(GLIB_LIBS)

if GCOV_ENABLED
coverage:
	gcov gst_switch_srv-*.o
endif
