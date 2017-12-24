AUTOMAKE_OPTIONS = foreign

CFLAGS = -O2 -s -Wall -Wextra

LDFLAGS = -lasound -lfltk

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
bin_PROGRAMS = midiguitar
midiguitar_SOURCES = main.cpp guitar.cpp guitar.h

desktopdir = $(datadir)/applications
desktop_DATA = midiguitar.desktop
EXTRA_DIST = $(desktop_DATA)

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = midi_guitar_48.png
EXTRA_DIST = $(pixmaps_DATA)