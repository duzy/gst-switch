dnl required version of autoconf
AC_PREREQ([2.53])

dnl TODO: fill in your package name and package version here
AC_INIT([gst-switch],[0.0.1])

dnl required versions of gstreamer and plugins-base
GLIB_REQUIRED=2.32.0
GST_REQUIRED=1.0.1
GSTPB_REQUIRED=1.0.1

AC_CONFIG_SRCDIR([plugins/gstswitch.c])
AC_CONFIG_HEADERS([config.h])

dnl required version of automake
AM_INIT_AUTOMAKE([1.10])
AC_CONFIG_MACRO_DIR([m4])

dnl enable mainainer mode by default
AM_MAINTAINER_MODE([enable])

dnl check for tools (compiler etc.)
AC_PROG_CC
AM_PROG_CC_C_O

dnl required version of libtool
LT_PREREQ([2.2.6])
LT_INIT

dnl give error and exit if we don't have pkgconfig
AC_CHECK_PROG(HAVE_PKGCONFIG, pkg-config, [ ], [
  AC_MSG_ERROR([You need to have pkg-config installed!])
])

PKG_CHECK_MODULES(GIO, [
  gio-2.0 >= 2.25.0
], [
  AC_SUBST(GIO_CFLAGS)
  AC_SUBST(GIO_LIBS)
])

PKG_CHECK_MODULES(GLIB, [
  glib-2.0 >= $GLIB_REQUIRED
], [
  AC_SUBST(GLIB_CFLAGS)
  AC_SUBST(GLIB_LIBS)
], [
  AC_MSG_ERROR([
      You need to install or upgrade the glib development
      packages on your system. The minimum version required
      is $GST_REQUIRED.
  ])
])

dnl GTK 
HAVE_GTK=NO
GTK2_REQ=2.14.0
GTK3_REQ=2.91.3
AC_MSG_CHECKING([which gtk+ version to compile examples against (optional)])
AC_ARG_WITH([gtk],
  AS_HELP_STRING([--with-gtk=3.0|2.0],[which gtk+ version to compile against (default: 3.0)]),
  [case "$with_gtk" in
      2.0) GTK_REQ=$GTK2_REQ ;;
      3.0) GTK_REQ=$GTK3_REQ ;;
      *) AC_MSG_ERROR([invalid gtk+ version specified]);;
  esac],
  [with_gtk=3.0
   GTK_REQ=$GTK3_REQ])
AC_MSG_RESULT([$with_gtk (>= $GTK_REQ)])
PKG_CHECK_MODULES(GTK, gtk+-x11-$with_gtk >= $GTK_REQ, HAVE_GTK=yes, HAVE_GTK=no)
AC_SUBST(GTK_LIBS)
AC_SUBST(GTK_CFLAGS)
AM_CONDITIONAL(HAVE_GTK, test "x$HAVE_GTK" = "xyes")

CFLAGS="$GTK_CFLAGS"
CPPFLAGS="$GTK_CFLAGS"
AC_CHECK_HEADER([gdk/gdkx.h], [],
  [AC_MSG_ERROR([GDK X11 header (gdk/gdkx.h) is required])])

AC_ARG_ENABLE(gcov,
  AS_HELP_STRING([--enable-gcov],[enable gcov building]),
  [
    case "${enableval}" in
      yes) BUILD_GCOV=yes ;;
      no)  BUILD_GCOV=no ;;
    esac
  ],
  [BUILD_GCOV=yes])
AC_ARG_ENABLE(gprof,
  AS_HELP_STRING([--enable-gprof],[enable gprof building]),
  [
    case "${enableval}" in
      yes) BUILD_GPROF=yes ;;
      no)  BUILD_GPROF=no ;;
    esac
  ],
  [BUILD_GPROF=yes])
AM_CONDITIONAL(GCOV_ENABLED, test "x$BUILD_GCOV" = "xyes")
AM_CONDITIONAL(GPROF_ENABLED, test "x$BUILD_GPROF" = "xyes")

dnl Check for the required version of GStreamer core (and gst-plugins-base)
dnl This will export GST_CFLAGS and GST_LIBS variables for use in Makefile.am
dnl
dnl If you need libraries from gst-plugins-base here, also add:
dnl for libgstaudio-1.0: gstreamer-audio-1.0 >= $GST_REQUIRED
dnl for libgstvideo-1.0: gstreamer-video-1.0 >= $GST_REQUIRED
dnl for libgsttag-1.0: gstreamer-tag-1.0 >= $GST_REQUIRED
dnl for libgstpbutils-1.0: gstreamer-pbutils-1.0 >= $GST_REQUIRED
dnl for libgstfft-1.0: gstreamer-fft-1.0 >= $GST_REQUIRED
dnl for libgstinterfaces-1.0: gstreamer-interfaces-1.0 >= $GST_REQUIRED
dnl for libgstrtp-1.0: gstreamer-rtp-1.0 >= $GST_REQUIRED
dnl for libgstrtsp-1.0: gstreamer-rtsp-1.0 >= $GST_REQUIRED
dnl etc.
PKG_CHECK_MODULES(GST, [
  gstreamer-1.0 >= $GST_REQUIRED
  gstreamer-base-1.0 >= $GST_REQUIRED
  gstreamer-controller-1.0 >= $GST_REQUIRED
  gstreamer-video-1.0 >= $GST_REQUIRED
], [
  AC_SUBST(GST_CFLAGS)
  AC_SUBST(GST_LIBS)
], [
  AC_MSG_ERROR([
      You need to install or upgrade the GStreamer development
      packages on your system. On debian-based systems these are
      libgstreamer1.0-dev and libgstreamer-plugins-base1.0-dev.
      on RPM-based systems gstreamer1.0-devel, libgstreamer1.0-devel
      or similar. The minimum version required is $GST_REQUIRED.
  ])
])

dnl check if compiler understands -Wall (if yes, add -Wall to GST_CFLAGS)
AC_MSG_CHECKING([to see if compiler understands -Wall])
save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Wall"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([ ], [ ])], [
  GST_CFLAGS="$GST_CFLAGS -Wall"
  AC_MSG_RESULT([yes])
], [
  AC_MSG_RESULT([no])
])

dnl set the plugindir where plugins should be installed (for plugins/Makefile.am)
if test "x${prefix}" = "x$HOME"; then
  plugindir="$HOME/.gstreamer-1.0/plugins"
else
  plugindir="\$(libdir)/gstreamer-1.0"
fi
AC_SUBST(plugindir)

dnl set proper LDFLAGS for plugins
GST_PLUGIN_LDFLAGS='-module -avoid-version -export-symbols-regex [_]*\(gst_\|Gst\|GST_\).*'
AC_SUBST(GST_PLUGIN_LDFLAGS)

AC_CONFIG_FILES(
Makefile
plugins/Makefile
tests/Makefile
tools/Makefile
)

AC_OUTPUT

echo "
  Now just type \"make\" to build.
"
