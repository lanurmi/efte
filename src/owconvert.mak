#
# owconvert.mak
#
# Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
#
# You may distribute under the terms of either the GNU General Public
# License or the Artistic License, as specified in the README file.
#
#
#----------------------------------------------------------------------------
#
# Makefile for NT/ECS-OS2 convert objs.inc using OpenWatcom 1.4 wmake
#
# watconvert reads in objs.inc, converts '\' character to '&', and saves
# as objs.mif.  objs.mif will be included by the OW14 NT and ECS-OS2 makefiles
#
#----------------------------------------------------------------------------


CC = wpp386
LD = wlink

# Machine type -5r Pent -6r Pent Pro
MACHINE= -6r

!ifdef __OS2__
SYSTEM=os2
LDSYSTEM=os2v2
!endif

!ifdef __NT__
SYSTEM=nt
LDSYSTEM=nt
!endif

#Optimization None: -od Time opt: -ot Fastest possible: -otexan
#OPT= -otexan
OPT=-od

INCLUDE = $(%watcom)\h;$(%watcom)\h\nt;.\

CFLAGS  = -i=$(INCLUDE) -d0 -w4 -e25 -zq $(OPT) $(MACHINE) -bm -bt=$(SYSTEM) -mf -xs -xr
LDFLAGS = op m op maxe=25 op q op symf op el
OEXT    = obj

.cpp.obj:
  $(CC) $(CFLAGS) $<

all: watconvert.exe

watconvert.exe: watconvert.obj
  $(LD) NAME watconvert SYS $(LDSYSTEM) $(LDFLAGS) FILE {watconvert.obj}
  watconvert

