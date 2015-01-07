include $(top_srcdir)/build/glib-tap.mk

LDADD = $(GTK_LIBS) $(GLIB_LIBS) $(GST_LIBS)

test_gstswitchopts_SOURCES = test_gstswitchopts.c
test_gstswitchopts_CFLAGS = $(GST_CFLAGS) $(GST_BASE_CFLAGS) \
  $(GCOV_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS) -DLOG_PREFIX="\"./tests\""
test_gstrecorder_filename_SOURCES = test_gstrecorder_filename.c ../../tools/gstworker.c
test_gstrecorder_filename_CFLAGS = -fprofile-arcs $(GST_CFLAGS) $(GST_BASE_CFLAGS) \
  $(GCOV_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS) -DLOG_PREFIX="\"./tests\""
test_gstrecorder_filename_LDFLAGS = $(GCOV_LFLAGS)

dist_test_data = \
  $(NULL)

test_programs = \
  test_gstswitchopts \
  test_gstrecorder_filename \
  $(NULL)

include $(top_srcdir)/build/Makefile.am.gitignore
