PREFIX=/usr/local

BINDIR=$(PREFIX)/bin
LIBDIR=$(PREFIX)/lib/fte
CONFIGDIR=$(LIBDIR)/config

.PHONY: all install

all:	fte
	(cd src ; make unix)

install: all
	sh install

fte: fte.in Makefile fte.spec
	sed < fte.in >$@ \
	    -e "s|@@CONFIGDIR@@|$(CONFIGDIR)|g" \
	    -e "s|@@BINDIR@@|$(BINDIR)|g"
	chmod a+x $@

dist: fte

clean:
	rm -f fte
	(cd src ; make -f fte-unix.mak clean)
