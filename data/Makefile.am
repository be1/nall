desktopdir       = $(datadir)/applications
desktop_in_files = nall.desktop.in
desktop_DATA     = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

pixmapsdir       = $(datadir)/pixmaps
pixmaps_DATA     = nall.png

EXTRA_DIST = \
	$(desktop_in_files) \
	$(pixmaps_DATA)

CLEANFILES = $(desktop_DATA)
