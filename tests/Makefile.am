noinst_PROGRAMS = \
  test-switch-server

test_switch_server_SOURCES = test_switch_server.c \
  ../tools/gstworker.c ../tools/gstswitchclient.c
test_switch_server_CFLAGS = $(GST_CFLAGS) $(GST_BASE_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS) \
  -DLOG_PREFIX="\"./tests\""
test_switch_server_LDFLAGS = $(GST_LIBS) $(GST_BASE_LIBS) $(GST_PLUGINS_BASE_LIBS) $(GSTPB_BASE_LIBS)
test_switch_server_LDADD = $(GST_LIBS) $(GIO_LIBS) $(LIBM)

include names.mk
$(TESTS) $(UI_TESTS): clean-test-instances
	@$(TESTWRAP) ./test-switch-server $(TESTARGS) --enable-$@ || true

clean-test-instances:
	pkill gst-switch-srv || true
	pkill gst-switch-ui || true
