# versions of FTE to build

# Versions:
#  xfte - using XLib (the most stable)

#  vfte - for Linux console directly (with limitations, see con_linux.cpp)

TARGETS = xfte vfte sfte
#TARGETS = xfte

PRIMARY = xfte

# Comment or uncoment this two flags below if
# you want to use:

# Keyboard remaping by XFTE
#REMAPFLAG = -DUSE_HARD_REMAP

# Drawing fonts with locale support
XMBFLAG = -DUSE_XMB

# System X11R6 is compiled with X_LOCALE
#SYSTEM_X_LOCALE = -DX_LOCALE

I18NOPTIONS = $(XMBFLAG) $(REMAPFLAG) $(SYSTEM_X_LOCALE)

COPTIONS = -Wall -Wpointer-arith -Wconversion -Wwrite-strings \
           -Wmissing-prototypes -Wmissing-declarations -Winline

#CC       = g++
#LD       = g++
# try this for smaller/faster code and less dependencies
CC       = g++ -fno-rtti -fno-exceptions
LD       = gcc -fno-rtti -fno-exceptions


# choose your os here

#######################################################################
# Linux
UOS      = -DLINUX
XLIBDIR  = -L/usr/X11R6/lib

#######################################################################
# HP/UX
#UOS      = -DHPUX -D_HPUX_SOURCE

#CC   = CC +a1
#LD   = CC

#XLIBDIR  = -L/usr/lib/X11R6

#XINCDIR  = -I/usr/include/X11R5
#XLIBDIR  = -L/usr/lib/X11R5

#MINCDIR  = -I/usr/include/Motif1.2
#MLIBDIR  = -L/usr/lib/Motif1.2

SINCDIR   = -I/usr/include/slang

#######################################################################
# AIX
#UOS      = -DAIX -D_BSD_INCLUDES # not recently tested (it did work)

#CC   = xlC
#LD   = xlC

#######################################################################
# Irix
# missing fnmatch, but otherwise ok (tested only on 64bit)
# 6.x has fnmatch now ;-)
# uncomment below to use SGI CC compiler
#UOS  = -DIRIX
#CC   = CC
#LD   = CC
#COPTIONS = -xc++

#######################################################################
# SunOS (Solaris)
#UOS      = -DSUNOS
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
#COPTIONS = +.cpp

#######################################################################
# NCR
#CC = cc -Hnocopyr
#LD = cc -Hnocopyr
#COPTIONS = -w3
#UOS = -DNCR
#XINCDIR  = -I../../include
#SOCKETLIB = -lsocket -lnsl -lc -lucb

#######################################################################

#QTDIR   = /users/markom/qt
#QLIBDIR  = -L$(QTDIR)/lib
#QINCDIR  = -I$(QTDIR)/include
#QINCDIR  = -I/usr/include/qt

MOC      = moc

LIBDIR   = 
INCDIR   =

#OPTIMIZE = -g # -O -g
OPTIMIZE = -O2 
#OPTIMIZE = -O2 -s

CCFLAGS  = $(OPTIMIZE) $(I18NOPTIONS) $(COPTIONS) -DUNIX $(UOS) $(INCDIR) $(XINCDIR) $(QINCDIR) $(MINCDIR) $(SINCDIR)
LDFLAGS  = $(OPTIMIZE) $(LIBDIR) $(XLIBDIR) $(QLIBDIR) $(MLIBDIR)

OEXT     = o

.SUFFIXES: .cpp .o .moc

include objs.inc

XLIBS    = -lX11 $(SOCKETLIB)
VLIBS    = -lgpm -lncurses
# -ltermcap outdated by ncurses
SLIBS    = -lslang
QLIBS    = -lqt
#MLIBS    = -lXm -lXp -lXt -lXpm -lXext

.cpp.o:
	$(CC) $(CCFLAGS) -c $<

.c.o:
	$(CC) $(CCFLAGS) -c $<

.cpp.moc: 
	$(MOC) $< -o $@

all:    cfte $(TARGETS)
#rm -f fte ; ln -s $(PRIMARY) fte

cfte: cfte.o s_files.o
	$(LD) $(LDFLAGS) cfte.o s_files.o -o cfte 

c_config.o: defcfg.h

defcfg.h: defcfg.cnf
	perl mkdefcfg.pl <defcfg.cnf >defcfg.h

defcfg.cnf: defcfg.fte cfte
	./cfte defcfg.fte defcfg.cnf

xfte: $(OBJS) $(XOBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(XOBJS) $(XLIBS) -o xfte

#qfte: g_qt.moc g_qt_dlg.moc $(OBJS) $(QOBJS)
#	$(LD) $(LDFLAGS) $(OBJS) $(QOBJS) $(QLIBS) $(XLIBS) -o qfte

vfte: $(OBJS) $(VOBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(VOBJS) $(VLIBS) -o vfte

sfte: $(OBJS) $(SOBJS) compkeys
	$(LD) $(LDFLAGS) $(OBJS) $(SOBJS) $(SLIBS) -o sfte

compkeys: compkeys.o 
	$(LD) $(LDFLAGS) compkeys.o -o compkeys

#mfte: $(OBJS) $(MOBJS)
#	$(LD) $(LDFLAGS) $(OBJS) $(MOBJS) $(MLIBS) $(XLIBS) -o mfte

g_qt.obj: g_qt.moc

g_qt_dlg.obj: g_qt_dlg.moc

clean:
	rm -f core *.o $(TARGETS) defcfg.h defcfg.cnf cfte fte vfte compkeys