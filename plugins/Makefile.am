plugin_LTLIBRARIES = libgstswitch.la

libgstswitch_la_SOURCES = gsttcpmixsrc.c gstswitchplugin.c
libgstswitch_la_CFLAGS = $(GST_CFLAGS) $(GIO_CFLAGS)
libgstswitch_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstswitch_la_LIBADD = $(GST_BASE_LIBS) $(GST_LIBS) $(GIO_LIBS)
libgstswitch_la_LIBTOOLFLAGS = --tag=disable-static
