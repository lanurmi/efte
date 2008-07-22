# 
# fte-bcc2.mak
# 
# Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
#
# You may distribute under the terms of either the GNU General Public
# License or the Artistic License, as specified in the README file.
#

LIBDIR = \BCOS2\LIB
INCDIR = \BCOS2\INCLUDE

.AUTODEPEND

DEFINES=-DOS2 -DBCPP -DHEAPWALK -DINCL_32

DEBUG =
#DEBUG=-v

INIT = $(LIBDIR)\c02.obj

CC = bcc
LD = tlink
CCFLAGS = $(DEFINES) -L$(LIBDIR) -I$(INCDIR) -H=fte.CSM \
          -k- -sm -K -w -w-par -Ot -RT- -xd- -x- -vi- -d -a -y $(DEBUG)

LDFLAGS = -L$(LIBDIR) $(DEBUG) -s -Toe -Oc -B:0x10000
OEXT=obj

!include objs.inc

.cpp.$(OEXT):
	$(CC) $(CCFLAGS) -c {$< }

.c.$(OEXT):
	$(CC) $(CCFLAGS) -c {$< }

all: cefte.exe efte.exe eftepm.exe clipserv.exe cliputil.exe

defcfg.cnf: defcfg.fte cefte.exe
	cfte defcfg.fte defcfg.cnf

defcfg.h: defcfg.cnf bin2c.exe
	bin2c defcfg.cnf >defcfg.h

c_config.obj: defcfg.h

bin2c.exe: bin2c.cpp
	$(CC) $(CCFLAGS) bin2c.cpp

cefte.exe: $(CFTE_OBJS)
	$(LD) @&&|
        $(LDFLAGS) $(INIT) $**,cefte.exe,cfte.map,os2 c2mt,cfte.def
|

efte.exe: $(OBJS) $(VIOOBJS)
	$(LD) @&&|
        $(LDFLAGS) $(INIT) $**,efte.exe,fte.map,os2 c2mt,fte.def
|

eftepm.exe:: $(OBJS) $(PMOBJS)
	$(LD) @&&|
        $(LDFLAGS) $(INIT) $**,eftepm.exe,ftepm.map,c2mt os2,ftepm.def
|
        rc -i $(INCDIR) ftepm.rc eftepm.exe

clipserv.exe: clipserv.obj
	$(LD) @&&|
        $(LDFLAGS) $(INIT) $**,clipserv.exe,clipserv.map,c2mt os2,clipserv.def
|
        
cliputil.exe: cliputil.obj clip_vio.obj
	$(LD) @&&|
        $(LDFLAGS) $(INIT) $**,cliputil.exe,cliputil.map,c2mt os2,cliputil.def
|

eftepm.exe:: ftepm.res
        rc ftepm.res eftepm.exe

ftepm.res: ftepm.rc pmdlg.rc
        rc -i $(INCDIR) -r ftepm.rc
