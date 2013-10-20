dnl AG_GST_PKG_CONFIG_PATH
dnl
dnl sets up a GST_PKG_CONFIG_PATH variable for use in Makefile.am
dnl which contains the path of the in-tree pkgconfig directory first
dnl and then any paths specified in PKG_CONFIG_PATH.
dnl
dnl We do this mostly so we don't have to use unportable shell constructs
dnl such as ${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH} in Makefile.am to handle
dnl the case where the environment variable is not set, but also in order
dnl to avoid a trailing ':' in the PKG_CONFIG_PATH which apparently causes
dnl problems with pkg-config on windows with msys/mingw.
AC_DEFUN([AG_GST_PKG_CONFIG_PATH],
[
  GST_PKG_CONFIG_PATH="\$(top_builddir)/pkgconfig"
  if test "x$PKG_CONFIG_PATH" != "x"; then
    GST_PKG_CONFIG_PATH="$GST_PKG_CONFIG_PATH:$PKG_CONFIG_PATH"
  fi
  AC_SUBST([GST_PKG_CONFIG_PATH])
  AC_MSG_NOTICE([Using GST_PKG_CONFIG_PATH = $GST_PKG_CONFIG_PATH])
])

AC_DEFUN([AG_GST_ARG_WITH_PKG_CONFIG_PATH],
[
  dnl possibly modify pkg-config path
  AC_ARG_WITH(pkg-config-path,
     AC_HELP_STRING([--with-pkg-config-path],
                    [colon-separated list of pkg-config(1) dirs]),
     [
       export PKG_CONFIG_PATH=${withval}
       AC_MSG_NOTICE(Set PKG_CONFIG_PATH to $PKG_CONFIG_PATH)
     ])
])
