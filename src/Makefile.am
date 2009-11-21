bin_PROGRAMS = nall

nall_SOURCES = \
        about.c \
	about.h \
	cb.c \
	cb.h \
	cfgfile.c \
	cfgfile.h \
	manage.c \
	manage.h \
	menu.c \
	menu.h \
	na.c \
	na.h \
	notify.c \
	notify.h \
	nall.c \
	version.h
nall_LDFLAGS = -export-dynamic

uidir   = $(datadir)/nall
ui_DATA = \
	nall-dialogs.ui

INCLUDES = @GLIB_CFLAGS@ @GTK_CFLAGS@ @LIBNOTIFY_CFLAGS@
LDADD    = @GLIB_LIBS@ @GTK_LIBS@ @LIBNOTIFY_LIBS@
DEFS     = -DLOCALEDIR=\"$(localedir)\" -DNALL_DATADIR=\"$(datadir)/nall\"

AM_CFLAGS = -Wall
if DEBUG
AM_CFLAGS += -g -DEBUG
endif
