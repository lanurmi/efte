PREFIX=/usr/local

BINDIR=$(PREFIX)/bin
LIBDIR=$(PREFIX)/lib/fte
CONFIGDIR=$(LIBDIR)/config

.PHONY: all install

all:	fte
	(cd src ; make unix)

install: all
	-rm -rf $(LIBDIR)
	mkdir -p $(BINDIR)
	mkdir -p $(LIBDIR)
	cp fte $(BINDIR)
	cp src/xfte $(BINDIR)
	cp src/cfte $(BINDIR)
	src/cfte config/main.fte $(LIBDIR)/system.fterc
	cp -r config $(LIBDIR)
	chmod a+r $(CONFIG)/*
	chmod a+r $(CONFIG)/*/*
	find $(CONFIG) -type d | while read dir; do chmod a+x $dir; done

fte: fte.in Makefile fte.spec
	sed <$< >$@ \
	    -e "s|@@CONFIGDIR@@|$(CONFIGDIR)|g" \
	    -e "s|@@BINDIR@@|$(BINDIR)|g"
	chmod a+x $@

dist: fte

clean:
	rm -f fte
	(cd src ; make -f fte-unix.mak clean)
