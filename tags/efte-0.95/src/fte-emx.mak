#
# fte-emx.mak
#
# Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
#
#
# You may distribute under the terms of either the GNU General Public
# License or the Artistic License, as specified in the README file.
#
#

INCDIR    =
LIBDIR    =

#OPTIMIZE  = -g
OPTIMIZE  = -O -s
#OPTIMIZE   = -O2 -s

MT        = -Zmt

CC        = gcc
LD        = gcc

#XTYPE      =  -Zomf
#XLTYPE     = -Zsys -Zlinker /map -Zlinker /runfromvdm # -Zomf
#OEXT=obj
OEXT=o

#DEFS       = -DDEBUG_EDITOR -DCHECKHEAP
#LIBS      = -lmalloc1
#DEFS      = -DDEBUG_EDITOR -DDBMALLOC -I/src/dbmalloc
#LIBS      = -L/src/dbmalloc -ldbmalloc
LIBS      = -lstdcpp

DEFS=-DINCL_32  #-DUSE_OS2_TOOLKIT_HEADERS

CCFLAGS   = $(OPTIMIZE) $(MT) $(XTYPE) -x c++ -Wall -DOS2 $(DEFS) $(INCDIR) -pipe
LDFLAGS   = $(OPTIMIZE) $(MT) -Zmap $(XLTYPE) $(LIBDIR)

.SUFFIXES: .cpp .$(OEXT)

include objs.inc

.cpp.$(OEXT):
	$(CC) $(CCFLAGS) -c $<

.c.$(OEXT):
	$(CC) $(CCFLAGS) -c $<

all: cefte.exe efte.exe eftepm.exe clipserv.exe cliputil.exe

clipserv.exe: clipserv.$(OEXT) clipserv.def
	$(LD) $(LDFLAGS) clipserv.$(OEXT) clipserv.def -o clipserv.exe $(LIBS)

cliputil.exe: cliputil.$(OEXT) clip_vio.$(OEXT) cliputil.def
	$(LD) $(LDFLAGS) cliputil.$(OEXT) clip_vio.$(OEXT) cliputil.def -o cliputil.exe $(LIBS)

cefte.exe: $(CFTE_OBJS) cfte.def
	$(LD) $(LDFLAGS) $(CFTE_OBJS) cfte.def -o cefte.exe $(LIBS)

defcfg.cnf: defcfg.fte cefte.exe
	cefte defcfg.fte defcfg.cnf

defcfg.h: defcfg.cnf bin2c.exe
	bin2c defcfg.cnf >defcfg.h

bin2c.exe: bin2c.cpp
	$(CC) $(CCFLAGS) bin2c.cpp -o bin2c.exe

c_config.$(OEXT): defcfg.h

efte.exe: $(OBJS) $(VIOOBJS) fte.def
	$(LD) $(LDFLAGS) $(OBJS) $(VIOOBJS) fte.def -o efte.exe $(LIBS)

ftepm.res: ftepm.rc pmdlg.rc bmps/*.bmp
	rc -r -i \emx\include ftepm.rc ftepm.res

eftepm.exe: $(OBJS) $(PMOBJS) ftepm.def ftepm.res
	$(LD) $(LDFLAGS) $(OBJS) $(PMOBJS) ftepm.def ftepm.res -o eftepm.exe $(LIBS)

efte.cnf: cefte.exe
	cefte ..\config\main.fte efte.cnf

#rc -i \emx\include ftepm.rc ftepm.exe

#ftepm.exe:: ftepm.res
#	rc ftepm.res ftepm.exe

distro: eftepm.exe efte.exe efte.cnf cefte.exe clipserv.exe cliputil.exe
	zip ../efte-os2.zip eftepm.exe efte.exe efte.cnf cefte.exe clipserv.exe cliputil.exe
	(cd .. && zip -r efte-config.zip Artistic doc config)
