#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([Nall!], [1.0], [brouits@free.fr], [nall])
AC_CONFIG_SRCDIR([src/cb.h])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

IT_PROG_INTLTOOL([0.40.0])
GETTEXT_PACKAGE=nall
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"],
		                      [The domain to use with gettext])
AM_GLIB_GNU_GETTEXT

# Optional features
AC_ARG_ENABLE(notifications,
	[AC_HELP_STRING([--disable-notifications], [compile without libnotify support])],
	enable_notifications="$enableval", enable_notifications="yes")

# Debug feature
AC_ARG_ENABLE(debug,
	[AC_HELP_STRING([--enable-debug], [compile with debug flags])],
	enable_debug="$enableval", enable_debug="no")
AM_CONDITIONAL(DEBUG, test "$enable_debug" = yes)

# Checks for libraries.
AM_PATH_GLIB_2_0([2.0.0],,[AC_MSG_ERROR([glib 2.0 or newer is required])])
AM_PATH_GTK_2_0([2.10.0],,[AC_MSG_ERROR([gtk+ 2.0 or newer is required])])

# Check for libnotify
if test "x$enable_notifications" = "xyes"; then
	PKG_CHECK_MODULES(LIBNOTIFY, libnotify >= 0.3.2,
		[ AC_DEFINE(HAVE_LIBNOTIFY, 1, [Define to 1 if libnotify has been found.]) ],
		[ AC_MSG_ERROR([libnotify headers not found.
Use --disable-notifications to disable desktop notifications.]) ]
	)
	AC_SUBST(LIBNOTIFY_CFLAGS)
	AC_SUBST(LIBNOTIFY_LIBS)
	PKG_CHECK_MODULES(LIBNOTIFY_OLD, libnotify < 0.7.0,
		[ AC_MSG_NOTICE([Using old libnotify API.])
		  AC_DEFINE([HAVE_LIBNOTIFY_OLD], [1],
			    [Define to 1 if libnotify version is below 0.7.0]) ],
		[ AC_MSG_NOTICE([Using new libnotify API.]) ]
	)
fi

# Checks for header files.
AC_CHECK_HEADERS([libintl.h locale.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SSIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([setlocale])

AC_CONFIG_FILES([
        Makefile
	src/Makefile
	po/Makefile.in
	data/Makefile
	data/nall.desktop.in
	examples/Makefile
])
AC_OUTPUT
