bin_PROGRAMS = nall

nall_SOURCES = \
        about.c \
	about.h \
	cb.c \
	cb.h \
	menu.c \
	menu.h \
	na.c \
	na.h \
	notify.c \
	notify.h \
	nall.c \
	version.h

INCLUDES = @GLIB_CFLAGS@ @GTK_CFLAGS@ @LIBNOTIFY_CFLAGS@
LDADD    = @GLIB_LIBS@ @GTK_LIBS@ @LIBNOTIFY_LIBS@
DEFS     = -DLOCALEDIR=\"$(localedir)\"
