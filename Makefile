# vim:ts=4:sw=4:noexpandtab:autoindent
PRGNAME=nall
PREFIX=/usr/local
SRC=$(PRGNAME).c na.c menu.c about.c cb.c
CFLAGS=`pkg-config --cflags glib-2.0` `pkg-config --cflags gtk+-2.0`
LDFLAGS=`pkg-config --libs glib-2.0` `pkg-config --libs gtk+-2.0`

all:
	$(CC) -Wall -Wextra $(SRC) -o $(PRGNAME) $(CFLAGS) $(LDFLAGS) && echo Have Fun!

install:
	install nall $(DESTDIR)/$(PREFIX)/bin/
	install nall.desktop $(DESTDIR)/$(PREFIX)/share/applications/
	install nall.png $(DESTDIR)/$(PREFIX)/share/icons/

uninstall:
	$(RM) $(DESTDIR)/$(PREFIX)/bin/nall 
	$(RM) $(DESTDIR)/$(PREFIX)/share/applications/nall.desktop

clean:
	$(RM) $(PRGNAME)
