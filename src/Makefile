# Makefile
#
# Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
#
# You may distribute under the terms of either the GNU General Public
# License or the Artistic License, as specified in the README file.
#

UNAME=$(shell uname -s)
ifndef $(TARGETS)
	TARGETS=efte nefte
	ifeq ($(UNAME),Linux)
		TARGETS := $(TARGETS) vefte
	endif
endif

# =============================================================================
#
# System Options
#
# =============================================================================

# NOTE: Unix does not have stricmp, however, in sysdeps.h it is mapped to
# strcasecmp which unix does have., thus stricmp is a valid function name
ifndef $(X11_PATH)
	X11_PATH=/usr/X11R6
endif

ifndef $(PREFIX)
	PREFIX=/usr/local
endif

DEFINES=-DUNIX -DHAVE_STRICMP
CXXFLAGS=$(DEBUG) -Wall -Wno-long-long -pedantic -I$(X11_PATH)/include
SYSTEM_LIBS=
BASE_LIBS=-lstdc++
EFTE_LIBS=-L$(X11_PATH)/lib -lX11 -lXpm
NEFTE_LIBS=
VEFTE_LIBS=

# System specific flags
ifeq ($(UNAME),Linux)
	DEFINES := $(DEFINES) -DLINUX
	ifndef $(USE_GPM)
		USE_GPM=1
	endif
endif
ifeq ($(UNAME),Darwin)
	DEFINES += -DDARWIN
endif
ifeq ($(UNAME),"HP/UX")
	DEFINES += -DHPUX -D_HPUX_SOURCE -DCASE_FD_SET_INT
endif
ifeq ($(UNAME),AIX)
	DEFINES += -DAIX -D_BSD_INCLUDES -DNO_NEW_CPP_FEATURES
endif
ifeq ($(UNAME),Irix)
	DEFINES += -DIRIX -DNO_NEW_CPP_FEATURES
	CXXFLAGS += -OPT:Olimit=3000   # -xc++
endif
ifeq ($(UNAME),SunOS)
	DEFINES += -DSUNOS -DNO_NEW_CPP_FEATURES
endif
ifeq ($(UNAME),SCO)
	DEFINES += -DSCO
	CXXFLAGS += -b elf
	SYSTEM_LIBS += -lsocket
endif
ifeq ($(UNAME),NCR)
	DEFINES += -DNCR
	CXXFLAGS += -w3
	SYSTEM_LIBS += -lsocket -lnsl -lc -lucb
endif

# Defaults
ifneq ($(USE_LOCALE),0)
	DEFINES += -DUSE_LOCALE
endif
ifneq ($(USE_LOGGING),0)
	DEFINES += -DUSE_LOGGING
endif
ifneq ($(USE_XMB),0)
	DEFINES += -DUSE_XMB
endif
ifneq ($(USE_XICON),0)
	DEFINES += -DUSE_XICON
endif

# Optionals
ifeq ($(USE_XLOCALE),1)
	DEFINES += -DUSE_XLOCALE
endif
ifeq ($(USE_XTINIT),1)
	DEFINES += -DUSE_XTINIT
endif
ifeq ($(USE_HARD_REMAP),1)
	DEFINES += -DUSE_HARD_REMAP
endif
ifeq ($(USE_GPM),1)
	DEFINES += -DUSE_GPM
	NEFTE_LIBS += -lgpm
	VEFTE_LIBS += -lgpm
endif
ifdef $(CURSES)
	NEFTE_LIBS += -l$(CURSES)
else
	NEFTE_LIBS += -lncurses
endif

#
# Compiler Setup
#

OEXT=o
include objs.inc

CXXFLAGS += $(DEFINES)
CXXFLAGS += -g
CXXFLAGS += -fomit-frame-pointer
#CXXFLAGS += -fsched2-use-traces


# =============================================================================
#
# Targets
#
# =============================================================================

all: bin2c defcfg.h $(TARGETS)

include .depend.mak

nefte-setup:
	@echo "nefte       :" $(NEFTE_LIBS)

vefte-setup:
	@echo "vefte       :" $(VEFTE_LIBS)

efte-setup:
	@echo "efte        :" $(EFTE_LIBS)

basesetup:
	@echo "System name :" $(UNAME)
	@echo "Targets     :" $(TARGETS)
	@echo "Compiler    :" $(CXX)
	@echo "CXX Flag    :" $(CXXFLAGS)
	@echo "System Libs :" $(SYSTEM_LIBS)

setup: basesetup $(TARGETS:=-setup)

clean:
	rm -f *.o .depend.mak

cleanall:
	rm -f efte nefte vefte bin2c defcfg.h config.h .depend.mak

help:
	@echo ""
	@echo "A simple ''make'' in most situations will work. This makefile, However,"
	@echo "has been setup to enable you to turn many options on and off by command"
	@echo "line switches."
	@echo
	@echo "To use a switch: make SWITCH=VALUE"
	@echo
	@echo "Switches:"
	@echo "  USE_GPM=0/1        Enable console mouse support via GPM"
	@echo "  USE_LOCALE=0/1     Use normal locale support"
	@echo "  USE_XLOCALE=0/1    Use X11 compiled in locale support"
	@echo "  USE_XICON=0/1      Use libxicon to supply icons for the X version"
	@echo "  USE_XTINIT=0/1     Use XtInitialize on init"
	@echo "  USE_XMB=0/1        Use libxmb to draw fonts with locale support"
	@echo "  USE_LOGGING=0/1    Enable trace logging within eFTE"
	@echo "  USE_HARD_REMAP=0/1 Keyboard remapping by eFTE"
	@echo "  CURSES=libname     Specify an alternate curses library"
	@echo "  X11_PATH=basepath  Specify location of X11 installation"
	@echo
	@echo "Defaults:"
	@echo "  Enabled   : USE_LOCALE, USE_LOGGING, USE_XMB, USE_XICON"
	@echo "  Disabled  : USE_XLOCALE, USE_XTINIT, USE_HARD_REMAP"
	@echo "  Curses    : ncurses"
	@echo "  X11 Path  : /usr/X11R6"
	@echo
	@echo "Smart defaults:"
	@echo "  Linux     : USE_GPM=1"
	@echo "  Non-Linux : USE_GPM=0"
	@echo
	@echo "Notes:"
	@echo "  If for your system, you find that you have to change the defaults"
	@echo "  in order to compile (not just for personal preference), please"
	@echo "  help us make eFTE that much better for all and submit a bug"
	@echo "  report to: http://sourceforge.net/projects/efte"
	@echo
	@echo "Individual target names:"
	@echo "  help      - Display this help message"
	@echo "  setup     - Display the setup that will be used to build"
	@echo "  all       - Build all binaries known to work on $(UNAME)"
	@echo "  efte      - Build the X11 version"
	@echo "  nefte     - Curses/NCurses version"
	@echo "  vefte     - Linux virtual console version"
	@echo "  clean     - Clean temporary build files"
	@echo "  cleanall  - Clean everything including built binaries"
	@echo "  install   - Install eFTE"
	@echo "  uninstall - Uninstall eFTE"
	@echo

bin2c: bin2c.cpp
	$(CXX) $(DEFINES) -o $@ $<

defcfg.h: bin2c simple.fte
	./bin2c simple.fte > defcfg.h

config.h:
	sed s%\$$\{CMAKE_INSTALL_PREFIX\}%$(PREFIX)%g < config.h.in > config.h

install: $(TARGETS)
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(TARGETS) $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(PREFIX)/share/doc
	install -d $(DESTDIR)$(PREFIX)/share/doc/efte
	install -m 644 ../README ../COPYING ../Artistic ../HISTORY ../AUTHORS $(DESTDIR)$(PREFIX)/share/doc/efte
	install -d $(DESTDIR)$(PREFIX)/share/efte
	install -d $(DESTDIR)$(PREFIX)/share/efte/config
	cp -a ../config/* $(DESTDIR)$(PREFIX)/share/efte/config

efte: config.h defcfg.h $(OBJS) $(XOBJS)
	$(CXX) -o efte $(OBJS) $(XOBJS) $(EFTE_LIBS)

nefte: config.h defcfg.h $(OBJS) $(NOBJS)
	$(CXX) -o nefte $(OBJS) $(NOBJS) $(NEFTE_LIBS)

vefte: config.h defcfg.h $(OBJS) $(VOBJS)
	$(CXX) -o vefte $(OBJS) $(VOBJS) $(VEFTE_LIBS)

.cpp.$(OEXT):
	$(CXX) $(CXXFLAGS) -o $@ -c $<

.depend.mak: Makefile
	which g++ 2>/dev/null >/dev/null && for f in $(addsuffix .cpp,$(basename $(OBJS) $(XOBJS) $(NOBJS) $(VOBJS) )) ; do \
		g++ $(CXXFLAGS) -MM "$$f" >> "$@" 2>/dev/null ; done || touch "$@"
