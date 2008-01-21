#----------------------------------------------------------------------------
#
# Makefile for ECS-OS2 version of FTE using OpenWatcom 1.4
#
# Notes:
#
# 1. clipserv and cliputil are not required for current versions of OS/2. I
#    do not use an older version so I do not include them in this makefile.
#
# 2. OpenWatcom uses & rather than \ as line continuation if objs.mif is not
#    in the src directory wmake -f owconvert.mak will compile watconvert.exe.
#    When run objs.mif will be created from objs.inc.
#
#----------------------------------------------------------------------------
.SILENT

CC = wpp386
LD = wlink
RC = rc

# Machine type -5r Pent -6r Pent Pro
MACHINE= -6r

#Optimization None -od  - Fastest possible -otexan
OPT= -ot

INCLUDE = $(%watcom)\h;$(%watcom)\h\os2;.\

# suppress warnings 555 and 013
MISC    = -wcd555 -wcd013 -wcd726

DEFS    = -dOS2 -dOS2OW -dINCL_32 -dWATCOM -dUSE_LOCALE
CFLAGS  = -i=$(INCLUDE) $(MISC) $(DEFS) -d0 -w4 -e25 -zq $(OPT) $(MACHINE) -bm -bt=OS2 -mf
LDFLAGS = op m op maxe=25 op q op symf op el
OEXT    = obj

.EXTENSIONS:.rc .res

!include objs.mif

.cpp.obj:
  $(CC) $(CFLAGS) $<

all: cfte.exe fte.exe ftepm.exe fte.cnf

cfte.exe: $(CFTE_OBJS) cfte.def
  $(LD) NAME cfte SYS os2v2 $(LDFLAGS) FILE {$(CFTE_OBJS)}

defcfg.cnf: defcfg.fte cfte.exe
  cfte defcfg.fte defcfg.cnf

defcfg.h: defcfg.cnf bin2c.exe
  bin2c defcfg.cnf >defcfg.h

fte.cnf: cfte.exe
  cfte ..\config\main.fte fte.cnf
  copy fte.cnf ftepm.cnf

bin2c.exe: bin2c.obj
  $(LD) NAME bin2c SYS os2v2 $(LDFLAGS) FILE {bin2c.obj}

c_config.obj: c_config.cpp defcfg.h

fte.exe: $(OBJS) $(VIOOBJS) fte.def
  $(LD) NAME fte SYS os2v2 $(LDFLAGS) FILE {$(OBJS) $(VIOOBJS)}

ftepm.res: ftepm.rc pmdlg.rc
  $(RC) -r ftepm.rc

ftepm.exe: $(OBJS) $(PMOBJS) ftepm.res
  $(LD) NAME ftepm SYS os2v2_pm $(LDFLAGS) FILE {$(OBJS) $(PMOBJS)}
  $(RC) ftepm.res ftepm.exe

clean : .SYMBOLIC
  -@rm *.obj
  -@rm *.exe
  -@rm *.err
  -@rm *.lst
  -@rm *.map

cleanall : .SYMBOLIC
  -@rm *.obj
  -@rm *.exe
  -@rm *.err
  -@rm *.lst
  -@rm *.map
  -@rm def*.h
  -@rm *.cnf
  -@rm *.mif
