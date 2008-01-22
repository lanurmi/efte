PREFIX=/usr/local

BINDIR=$(PREFIX)/bin
LIBDIR=$(PREFIX)/lib/fte
CONFIGDIR=$(LIBDIR)/config

.PHONY: all install

all:	fte
	$(MAKE) -C src unix

install: all
	sh ./install

fte: fte.in Makefile
	sed < fte.in >$@ \
	    -e "s|@@CONFIGDIR@@|$(CONFIGDIR)|g" \
	    -e "s|@@BINDIR@@|$(BINDIR)|g"
	chmod a+x $@

dist: fte
	scripts/mkbuildlvl.pl

clean:
	-rm -f core `find . -name '#*' -o -name 'fte-new.cnf'\
	       -o -name '.\#*' -o -name '.*~' -o -name '*~' -o -name 'core*'`
	$(MAKE) -C src -f fte-unix.mak clean
