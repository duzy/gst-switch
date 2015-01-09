dnl required version of autoconf
AC_PREREQ([2.53])

dnl TODO: fill in your package name and package version here
AC_INIT([gst-switch],[0.0.1])

dnl required versions of gstreamer and plugins-base
GLIB_REQUIRED=2.32.0
GST_REQUIRED=1.0.1
GSTPB_REQUIRED=1.0.1
GTK_REQUIRED=3.10.8

AC_CONFIG_SRCDIR([plugins/gstswitch.c])
AC_CONFIG_HEADERS([config.h])

dnl required version of automake
AM_INIT_AUTOMAKE([-Wno-portability 1.11 no-dist-gzip dist-xz tar-ustar])
dnl use pretty build output with automake >= 1.11
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])],
  [AM_DEFAULT_VERBOSITY=1
   AC_SUBST(AM_DEFAULT_VERBOSITY)])
AC_CONFIG_MACRO_DIR([build])

dnl enable mainainer mode by default
AM_MAINTAINER_MODE([enable])

dnl check for tools (compiler etc.)
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX

dnl required version of libtool
LT_PREREQ([2.2.6])
LT_INIT

dnl give error and exit if we don't have pkgconfig
AC_CHECK_PROG(HAVE_PKGCONFIG, pkg-config, [ ], [
  AC_MSG_ERROR([You need to have pkg-config installed!])
])

dnl === Glib + GIO ============================================================
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
      is $GLIB_REQUIRED.
  ])
])

PKG_CHECK_MODULES(GTK, [
  gtk+-3.0 >= $GTK_REQUIRED
], [
  AC_SUBST(GTK_CFLAGS)
  AC_SUBST(GTK_LIBS)
], [
  AC_MSG_ERROR([
      You need to install or upgrade the gtk development
      packages on your system. The minimum version required
      is $GTK_REQUIRED.
  ])
])

PKG_CHECK_MODULES(GDK, [
  gdk-3.0 >= $GTK_REQUIRED
], [
  AC_SUBST(GDK_CFLAGS)
  AC_SUBST(GDK_LIBS)
], [
  AC_MSG_ERROR([
      You need to install or upgrade the gdk development
      packages on your system. The minimum version required
      is $GTK_REQUIRED.
  ])
])

dnl === gstreamer related options =============================================

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

dnl === Testing ===============================================================

GLIB_TESTS

dnl FIXME: Add checking for the testing requirements here.

dnl === Coverage and profiling options ========================================
AC_ARG_ENABLE(gcov,
  AS_HELP_STRING([--enable-gcov],[enable gcov building]),
  [
    case "${enableval}" in
      yes) BUILD_GCOV=yes ;;
      no)  BUILD_GCOV=no ;;
    esac
  ],
  [BUILD_GCOV=yes])
AM_CONDITIONAL(GCOV_ENABLED, test "x$BUILD_GCOV" = "xyes")

AC_ARG_ENABLE(gprof,
  AS_HELP_STRING([--enable-gprof],[enable gprof building]),
  [
    case "${enableval}" in
      yes) BUILD_GPROF=yes ;;
      no)  BUILD_GPROF=no ;;
    esac
  ],
  [BUILD_GPROF=yes])
AM_CONDITIONAL(GPROF_ENABLED, test "x$BUILD_GPROF" = "xyes")

dnl === Local options =========================================================
AC_ARG_ENABLE(speakertrack,
  AS_HELP_STRING([--enable-speakertrack],[enable speakertrack]),
  [
    case "${enableval}" in
      yes) BUILD_SPEAKERTRACK=yes ;;
      no)  BUILD_SPEAKERTRACK=no ;;
    esac
  ],
  [BUILD_SPEAKERTRACK=no])
  AM_CONDITIONAL(SPEAKERTRACK_ENABLED, test "x$BUILD_SPEAKERTRACK" = "xyes")

dnl FIXME: Add the Python-API as an option here

dnl === Output ================================================================

AC_ARG_ENABLE(debug,
AS_HELP_STRING([--enable-debug],
               [enable debugging, default: no]),
[case "${enableval}" in
             yes) debug=true ;;
             no)  debug=false ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],
[debug=false])

AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")

AC_CONFIG_FILES(
Makefile
plugins/Makefile
tests/Makefile
tests/unit/Makefile
tools/Makefile
)

AC_OUTPUT

cd .git/hooks && ln -s ../../scripts/hooks/pre-commit

echo "
  Now just type \"make\" to build.
"
