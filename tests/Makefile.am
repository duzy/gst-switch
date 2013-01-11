noinst_PROGRAMS = \
  test-mix-source \
  test-switch-server

test_mix_source_SOURCES = test_mix_source.c
test_mix_source_CFLAGS = $(GST_CFLAGS) $(GST_BASE_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS) \
  -DLOG_PREFIX="\"./tests\""
test_mix_source_LDFLAGS = $(GST_LIBS) $(GST_BASE_LIBS) $(GST_PLUGINS_BASE_LIBS) $(GSTPB_BASE_LIBS)
test_mix_source_LDADD = $(GST_LIBS) $(GIO_LIBS) $(LIBM)

test_switch_server_SOURCES = test_switch_server.c
test_switch_server_CFLAGS = $(GST_CFLAGS) $(GST_BASE_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS) \
  -DLOG_PREFIX="\"./tests\""
test_switch_server_LDFLAGS = $(GST_LIBS) $(GST_BASE_LIBS) $(GST_PLUGINS_BASE_LIBS) $(GSTPB_BASE_LIBS)
test_switch_server_LDADD = $(GST_LIBS) $(GIO_LIBS) $(LIBM)
