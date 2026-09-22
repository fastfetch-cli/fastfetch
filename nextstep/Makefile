# Makefile for the NeXTSTEP 3.3 / OPENSTEP 4.2 mini port of fastfetch.
#
# Run this with the compiler/make that ship with NeXTSTEP or OPENSTEP
# themselves (GCC 2.5.8 / 2.7.2.1, GNU Make) - there is no cross-compile
# path here, build natively on the target machine or under emulation
# (Previous for m68k, QEMU for x86).

# -posix is required, not optional: this system keeps uname() and friends
# in /usr/lib/libposix.a, but linking that library by hand doesn't work -
# you have to compile *and* link via `cc -posix`, which selects the right
# startup files together with the library.
CC = cc
CFLAGS = -O -Wall -posix
LDFLAGS = -posix
LIBS =
PREFIX = /usr/local

fastfetch: fastfetch.c
	$(CC) $(CFLAGS) -o fastfetch fastfetch.c $(LDFLAGS) $(LIBS)

clean:
	rm -f fastfetch

install: fastfetch
	cp fastfetch $(PREFIX)/bin/fastfetch

.PHONY: clean install
