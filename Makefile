# vim:ts=4:sw=4:noexpandtab:autoindent
PRGNAME=nall
VERSION=0.8
PREFIX=/usr/local
SRC=$(PRGNAME).c na.c menu.c about.c cb.c
LANGS=fr_FR de
MO=$(patsubst %,%.mo,$(LANGS))
CFLAGS=`pkg-config --cflags glib-2.0` `pkg-config --cflags gtk+-2.0`
LDFLAGS=`pkg-config --libs glib-2.0` `pkg-config --libs gtk+-2.0`

all: $(SRC) *.h $(MO)
	sed -e "s#__VERSION__#$(VERSION)#" PKGBUILD.in > PKGBUILD
	sed -e "s#__PREFIX__#$(PREFIX)#" $(PRGNAME).desktop.in > $(PRGNAME).desktop
	$(CC) -Wall -Wextra $(SRC) -o $(PRGNAME) $(CFLAGS) $(LDFLAGS)

%.mo: %.po
	msgfmt -c -o $@ $<

%.po: $(PRGNAME).pot
	msgmerge -s -U $@ $(PRGNAME).pot

$(PRGNAME).pot: $(SRC) version.h
	xgettext -k_ -d $(PRGNAME) -s --package-name=$(PRGNAME) --package-version=$(VERSION) -o $(PRGNAME).pot $(SRC) version.h

version.h: version.h.in
	sed -e "s#__VERSION__#$(VERSION)#" -e "s#__PRGTITLE__#$(PRGTITLE)#" -e 's#__PRGLOCALEPATH__#$(PREFIX)/share/locale#' version.h.in > version.h

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/share/applications
	mkdir -p $(DESTDIR)$(PREFIX)/share/icons
	mkdir -p $(DESTDIR)$(PREFIX)/share/doc/nall/examples

	install $(PRGNAME) $(DESTDIR)$(PREFIX)/bin/
	install $(PRGNAME).desktop $(DESTDIR)$(PREFIX)/share/applications/
	install $(PRGNAME).png $(DESTDIR)$(PREFIX)/share/icons/
	install scripts/* $(DESTDIR)$(PREFIX)/share/doc/nall/examples

	for lang in $(LANGS); do \
	  mkdir -p $(DESTDIR)$(PREFIX)/share/locale/$$lang/LC_MESSAGES; \
	  install $$lang.mo $(DESTDIR)$(PREFIX)/share/locale/$$lang/LC_MESSAGES/$(PRGNAME).mo; \
	done

uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/bin/$(PRGNAME) 
	$(RM) $(DESTDIR)$(PREFIX)/share/icons/$(PRGNAME).png 
	$(RM) $(DESTDIR)$(PREFIX)/share/applications/$(PRGNAME).desktop
	$(RM) -r $(DESTDIR)$(PREFIX)/share/doc/nall
	for lang in $(LANGS); do \
	  $(RM) $(DESTDIR)$(PREFIX)/share/locale/$(LANG)/LC_MESSAGES/$(PRGNAME).mo; \
	done

dist:
	cd ..; cp -a $(PRGNAME) $(PRGNAME)-$(VERSION); $(RM) -r $(PRGNAME)-$(VERSION)/.git; tar cvf $(PRGNAME)-$(VERSION).tar $(PRGNAME)-$(VERSION); $(RM) -r $(PRGNAME)-$(VERSION); cd -
clean:
	$(RM) $(PRGNAME) $(MO) $(PRGNAME).desktop $(PRGNAME).pot version.h PKGBUILD *~
