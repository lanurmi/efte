# fte-unix.make modified to be generic enough to work with WINNT

# versions of FTE to build
# Versions:
#  xfte - using XLib (the most stable)
#  vfte - for Linux console directly (with limitations, see con_linux.cpp)

#WINNT
XEXT=.exe
OEXT=obj
OUTFLAG = -out:

#UNIX
#XEXT=
#OEXT=o
#OUTFLAG = -o 
# -o must have a space after it above

.SUFFIXES: .cpp .$(OEXT)

TARGETS = xfte$(XEXT)
#TARGETS = vfte$(XEXT) xfte$(XEXT)

PRIMARY = xfte$(XEXT)

# choose your os here

#######################################################################
#MSVC/EXCEED XDK
X_BASE =	C:\win32app\Exceed\xdk

X_LIBS = -LIBPATH:$(X_BASE)\lib Xm.lib \
	Mrm.lib Xmu.lib Xt.lib \
	Xlib.lib hclshm.lib xlibcon.lib
XINCDIR = -I:$(X_BASE)\include

#optimize
OPTIMIZE  = /Ox -DNDEBUG
#debug
OPTIMIZE  = /Od /Zi -D_DEBUG
LDOPTIMIZE = /DEBUG 

CC        = cl
LD        = link

CCFLAGS   = $(OPTIMIZE) -DNT -DMSVC $(INCDIR) -DWIN32 -D_CONSOLE -DWINHCLX \
	-nologo -W3 -J -DNONXP -DWINNT -D_X86_ -MD -Op -W3 -QIfdiv -GX \
	$(XINCDIR)
LDFLAGS   = $(LDOPTIMIZE) $(LIBDIR) -nologo Advapi32.lib User32.lib Wsock32.lib \
	-map /NODEFAULTLIB:libc.lib $(X_LIBS)


#######################################################################
# Linux
#UOS      = -DLINUX
#XLIBDIR  = -L/usr/X11/lib

#######################################################################
# HP/UX
#UOS      = -DHPUX -D_HPUX_SOURCE
#XINCDIR  = -I/usr/include/X11R5
#XLIBDIR  = -L/usr/lib/X11R5
#MINCDIR  = -I/usr/include/Motif1.2
#MLIBDIR  = -L/usr/lib/Motif1.2

#######################################################################
# AIX
#UOS      = -DAIX -D_BSD_INCLUDES # not recently tested (it did work)

#######################################################################
# Irix
# missing fnmatch, but otherwise ok (tested only on 64bit)
# 6.x has fnmatch now ;-)
# uncomment below to use SGI CC compiler
#UOS      = -DIRIX

#######################################################################
# SunOS (Solaris)
#UOS      = -DSUNOS
#XINCDIR  = -I/usr/openwin/include
#XLIBDIR  = -L/usr/openwin/lib

#######################################################################

#QTDIR   = /users/markom/qt
#QLIBDIR  = -L$(QTDIR)/lib
#QINCDIR  = -I$(QTDIR)/include

MOC      = moc

# for GCC
#CC       = g++
#LD       = gcc
#COPTIONS = -xc++ -Wall

# for IRIX CC
#CC       = CC
#LD       = CC
#COPTIONS = -xc++

#LIBDIR   = 
#INCDIR   =

#OPTIMIZE = -g
#OPTIMIZE = -O -g
#OPTIMIZE = -O -s

#UNIX
#CCFLAGS  = $(OPTIMIZE) $(COPTIONS) -DUNIX $(UOS) $(INCDIR) $(XINCDIR) $(QINCDIR) $(MINCDIR)
#LDFLAGS  = $(OPTIMIZE) $(LIBDIR) $(XLIBDIR) $(QLIBDIR) $(MLIBDIR)

include objs.inc

.cpp.$(OEXT):
	$(CC) $(CCFLAGS) -c $<

.c.$(OEXT):
	$(CC) $(CCFLAGS) -c $<

all: cfte$(XEXT) $(TARGETS)


#UNIX
#XLIBS    = -lX11
#QLIBS    = -lqt
#VLIBS    = -lgpm -ltermcap
#MLIBS    = -lXm -lXt

.cpp.$(OEXT):
	$(CC) $(CCFLAGS) -c $<

.c.$(OEXT):
	$(CC) $(CCFLAGS) -c $<

.cpp.moc: 
	$(MOC) $< -o $@

all:    cfte$(XEXT) $(TARGETS)
#	rm -f fteln -s $(PRIMARY) fte

cfte$(XEXT): cfte.$(OEXT) s_files.$(OEXT)
	$(LD) $(LDFLAGS) cfte.$(OEXT) s_files.$(OEXT) $(OUTFLAG)cfte$(XEXT) 

c_config.$(OEXT): defcfg.h

defcfg.h: defcfg.cnf
	perl mkdefcfg.pl <defcfg.cnf >defcfg.h

defcfg.cnf: defcfg.fte cfte$(XEXT)
	cfte$(XEXT) defcfg.fte defcfg.cnf

xfte$(XEXT): $(OBJS) $(XOBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(XOBJS) $(XLIBS) $(OUTFLAG)xfte$(XEXT)

qfte$(XEXT): g_qt.moc g_qt_dlg.moc $(OBJS) $(QOBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(QOBJS) $(QLIBS) $(XLIBS) $(OUTFLAG)qfte$(XEXT)

vfte$(XEXT): $(OBJS) $(VOBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(VOBJS) $(VLIBS) $(OUTFLAG)vfte$(XEXT)

mfte$(XEXT): $(OBJS) $(MOBJS)
	$(LD) $(LDFLAGS) $(OBJS) $(MOBJS) $(MLIBS) $(XLIBS) $(OUTFLAG)mfte$(XEXT)

g_qt.$(OEXT): g_qt.moc

g_qt_dlg.$(OEXT): g_qt_dlg.moc

clean:
	rm -f *.$(OEXT) $(TARGETS) defcfg.h defcfg.cnf cfte$(XEXT) fte$(XEXT)
