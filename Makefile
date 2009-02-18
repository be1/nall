# vim:ts=4:sw=4:noexpandtab:autoindent
PRGNAME=nall
SRC=$(PRGNAME).c na.c menu.c about.c cb.c
CFLAGS=`pkg-config --cflags glib-2.0` `pkg-config --cflags gtk+-2.0`
LDFLAGS=`pkg-config --libs glib-2.0` `pkg-config --libs gtk+-2.0`

all:
	@$(CC) -Wall $(SRC) -o $(PRGNAME) $(CFLAGS) $(LDFLAGS) && echo Have Fun!

clean:
	$(RM) $(PRGNAME)
