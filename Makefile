# vim:ts=4:sw=4:noexpandtab:autoindent
PRGNAME=nall
PREFIX=/usr/local
SRC=$(PRGNAME).c na.c menu.c about.c cb.c
CFLAGS=`pkg-config --cflags glib-2.0` `pkg-config --cflags gtk+-2.0`
LDFLAGS=`pkg-config --libs glib-2.0` `pkg-config --libs gtk+-2.0`

all: $(SRC) *.h
	sed -e "s#__PREFIX__#$(DESTDIR)$(PREFIX)#" $(PRGNAME).desktop.in > $(PRGNAME).desktop
	$(CC) -Wall -Wextra $(SRC) -o $(PRGNAME) $(CFLAGS) $(LDFLAGS)

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/share/applications
	mkdir -p $(DESTDIR)$(PREFIX)/share/icons

	install $(PRGNAME) $(DESTDIR)$(PREFIX)/bin/
	install $(PRGNAME).desktop $(DESTDIR)$(PREFIX)/share/applications/
	install $(PRGNAME).png $(DESTDIR)$(PREFIX)/share/icons/

uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/bin/$(PRGNAME) 
	$(RM) $(DESTDIR)$(PREFIX)/share/icons/$(PRGNAME).png 
	$(RM) $(DESTDIR)$(PREFIX)/share/applications/$(PRGNAME).desktop

clean:
	$(RM) $(PRGNAME) $(PRGNAME).desktop *~
