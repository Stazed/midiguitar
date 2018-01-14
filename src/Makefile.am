AUTOMAKE_OPTIONS = foreign

CFLAGS = -Wall -Wextra

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
bin_PROGRAMS = midiguitar

#FIXME: don't compile RtMidi if not defined
midiguitar_SOURCES = main.cpp guitar.cpp RtMidi.cpp guitar.h RtMidi.h globals.h
