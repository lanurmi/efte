# versions of FTE to build

# Versions:
#  xfte - using XLib (the most stable)

#  vfte - for Linux console directly (with limitations, see con_linux.cpp)

TGT_NFTE = nfte
#TGT_QFTE = qfte
#TGT_SFTE = sfte
TGT_VFTE = vfte
TGT_XFTE = xfte

TARGETS = $(TGT_XFTE) $(TGT_NFTE) $(TGT_QFTE) $(TGT_SFTE) $(TGT_VFTE)

# Comment or uncoment this two flags below if
# you want to use:

# Keyboard remaping by XFTE
#REMAPFLAG = -DUSE_HARD_REMAP

# Drawing fonts with locale support
XMBFLAG = -DUSE_XMB

# System X11R6 is compiled with X_LOCALE
#SYSTEM_X_LOCALE = -DX_LOCALE

# Enable normal locale support
USE_LOCALE = -DUSE_LOCALE

I18NOPTIONS = $(XMBFLAG) $(REMAPFLAG) $(SYSTEM_X_LOCALE) $(USE_LOCALE)

# Optionally, you can define:
# -DDEFAULT_INTERNAL_CONFIG to use internal config by default
# -DUSE_XTINIT to use XtInitialize on init
# -DFTE_NO_LOGGING to completely disable trace logging
APPOPTIONS = -DDEFAULT_INTERNAL_CONFIG

# Supply icons in X, requires Xpm library
USE_XICON = -DUSE_XICON
XPMLIB = -lXpm

#gcc/g++
#CC = g++
#LD = g++
CC = $(CXX)
LD = $(CXX)
CPPOPTIONS = -Wall -Wpointer-arith -Wconversion -Wwrite-strings -Winline

# try this for smaller/faster code and less dependencies
NOEXCEPTION = -fno-rtti -fno-exceptions


# choose your os here

#######################################################################
# Linux
UOS      = -DLINUX
XINCDIR  = -I/usr/X11R6/include
XLIBDIR  = -L/usr/X11R6/lib

#######################################################################
# HP/UX
#UOS      = -DHPUX -D_HPUX_SOURCE -DCAST_FD_SET_INT
#UOS      = -DHPUX -D_HPUX_SOURCE

#CC  = CC +a1
#LD  = CC
#CC = aCC
#LD = aCC

#XLIBDIR  = -L/usr/lib/X11R6

#XINCDIR  = -I/usr/include/X11R5
#XLIBDIR  = -L/usr/lib/X11R5

#MINCDIR  = -I/usr/include/Motif1.2
#MLIBDIR  = -L/usr/lib/Motif1.2

SINCDIR   = -I/usr/include/slang

#######################################################################
# AIX
#UOS      = -DAIX -D_BSD_INCLUDES

#CC   = xlC
#LD   = xlC
#CPPOPTIONS = -DNO_NEW_CPP_FEATURES
#TARGETS = xfte

#######################################################################
# Irix
# missing fnmatch, but otherwise ok (tested only on 64bit)
# 6.x has fnmatch now ;-)
# uncomment below to use SGI CC compiler
#UOS  = -DIRIX
#CC   = CC
#LD   = CC
#CPPOPTIONS = -DNO_NEW_CPP_FEATURES -OPT:Olimit=3000 # -xc++

#######################################################################
# SunOS (Solaris)
#UOS      = -DSUNOS
#CC = CC -noex
#LD = CC -noex
#CPPOPTIONS = -DNO_NEW_CPP_FEATURES
#XINCDIR  = -I/usr/openwin/include
#XLIBDIR  = -L/usr/openwin/lib

#######################################################################
# for SCO CC
#
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# If you have problems with the program "cfte"
# try to compile this program without optimalization -O2
# use just -O
#
#UOS = -DSCO
#CC  = CC  -b elf
#LD  = $(CC)
#XLIBDIR  = -L/usr/X11R6/lib
#SOCKETLIB = -lsocket
#CPPOPTIONS = +.cpp

#######################################################################
# NCR
#CC = cc -Hnocopyr
#LD = cc -Hnocopyr
#CPPOPTIONS = -w3
#UOS = -DNCR
#XINCDIR  = -I../../include
#SOCKETLIB = -lsocket -lnsl -lc -lucb

#######################################################################

#QTDIR   = /users/markom/qt
#QLIBDIR  = -L$(QTDIR)/lib
#QINCDIR  = -I$(QTDIR)/include
#QINCDIR  = -I/usr/include/qt
QINCDIR =  -I/usr/include/qt3
MOC      = moc

LIBDIRS   =
INCDIRS   = $(XINCDIR) $(QINCDIR) $(MINCDIR) $(SINCDIR)

OPTIMIZE = -g # -O -g
#OPTIMIZE = -O2
#OPTIMIZE = -O2 -s

CCFLAGS  = $(CPPOPTIONS) $(OPTIMIZE) $(NOEXCEPTION) $(INCDIRS) -DUNIX $(UOS) \
	$(I18NOPTIONS) $(APPOPTIONS) $(USE_XICON)
LDFLAGS  = $(OPTIMIZE) $(LIBDIRS)


.SUFFIXES: .cpp .o .moc

OEXT     = o
include objs.inc

SRCS = $(OBJS:.o=.cpp)

# Need -lXt below if USE_XTINIT is defined
XLIBS    = $(XLIBDIR) -lX11 $(SOCKETLIB) $(XPMLIB)
VLIBS    = -lgpm -lncurses
NLIBS    = -lncurses
SLIBS    = -lslang
#QLIBS    = $(QLIBDIR) -lqt
QLIBS    = $(QLIBDIR) -lqt-mt
#MLIBS    = $(MLIBDIR) -lXm -lXp -lXt -lXpm -lXext

.cpp.o:
	$(CC) -c $< $(CXXFLAGS) $(CPPFLAGS) $(CCFLAGS)

.c.o:
	$(CC) -c $< $(CFLAGS) $(CPPFLAGS) $(CCFLAGS)

.cpp.moc:
	$(MOC) -o $@ $<

all:    cfte $(TARGETS)

cfte: cfte.o s_files.o s_string.o
	$(LD) -o $@ $(LDFLAGS) cfte.o s_files.o s_string.o

c_config.o: defcfg.h

defcfg.h: defcfg.cnf
	perl mkdefcfg.pl <defcfg.cnf >defcfg.h

#DEFAULT_FTE_CONFIG = simple.fte
DEFAULT_FTE_CONFIG = defcfg.fte
#DEFAULT_FTE_CONFIG = defcfg2.fte
#DEFAULT_FTE_CONFIG = ../config/main.fte

defcfg.cnf: $(DEFAULT_FTE_CONFIG) cfte
	./cfte $(DEFAULT_FTE_CONFIG) defcfg.cnf

xfte: .depend $(OBJS) $(XOBJS)
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(XOBJS) $(XLIBS)

qfte: g_qt.moc g_qt_dlg.moc $(OBJS) $(QOBJS)
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(QOBJS) $(QLIBS) $(XLIBS)

vfte: $(OBJS) $(VOBJS)
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(VOBJS) $(VLIBS)

sfte: $(OBJS) $(SOBJS) compkeys
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(SOBJS) $(SLIBS)

nfte: $(OBJS) $(NOBJS) compkeys
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(NOBJS) $(NLIBS)

compkeys: compkeys.o
	$(LD) -o $@ $(LDFLAGS) compkeys.o

mfte: $(OBJS) $(MOBJS)
	$(LD) -o $@ $(LDFLAGS) $(OBJS) $(MOBJS) $(MLIBS) $(XLIBS)

g_qt.obj: g_qt.moc

g_qt_dlg.obj: g_qt_dlg.moc

.depend: defcfg.h
	$(CC) -MM $(CCFLAGS) $(SRCS) 1>.depend

# purposefully not part of "all".
tags: $(SRCS) $(wildcard *.h)
	ctags *.h $(SRCS)

.PHONY: clean

clean:
	rm -f core *.o *.moc .depend $(TARGETS) defcfg.h defcfg.cnf \
	cfte fte sfte vfte nfte qfte xfte compkeys tags

#
# include dependency files if they exist
#
ifneq ($(wildcard .depend),)
include .depend
endif
