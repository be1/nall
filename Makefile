PRGNAME=nall
SRC=$(PRGNAME).c na.c
CFLAGS=`pkg-config --cflags glib-2.0` `pkg-config --cflags gtk+-2.0` -g
LDFLAGS=`pkg-config --libs glib-2.0` `pkg-config --libs gtk+-2.0`

all:
	@$(CC) $(SRC) -o $(PRGNAME) $(CFLAGS) $(LDFLAGS) && echo Have Fun!

clean:
	rm -f $(PRGNAME) *.o
