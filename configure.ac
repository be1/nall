#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([Nall!], [0.8], [brouits@free.fr], [nall])
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

# Checks for libraries.
AM_PATH_GLIB_2_0([2.0.0],,[AC_MSG_ERROR([glib 2.0 or newer is required])])
AM_PATH_GTK_2_0([2.0.0],,[AC_MSG_ERROR([gtk+ 2.0 or newer is required])])

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
