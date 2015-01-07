include $(top_srcdir)/build/glib-tap.mk

LDADD = \
  $(GLIB_LIBS) $(GST_LIBS)

CFLAGS += $(GST_CFLAGS)

dist_test_data = \
  $(NULL)

test_programs = \
  test_gstswitchopts \
  test_gstcomposite \
  $(NULL)

if GCOV_ENABLED
coverage:
	gcov *.o
endif

include $(top_srcdir)/build/Makefile.am.gitignore
