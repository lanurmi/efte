
#----------------------------------------------------------------------------
#
# Makefile for NT version of FTE using OpenWatcom 1.4
#
# Notes:
#
#    OpenWatcom uses & rather than \ as line continuation if objs.mif is not
#    in the src directory wmake -f owconvert.mak will compile watconvert.exe.
#    When run objs.mif will be created from objs.inc.
#
#----------------------------------------------------------------------------


CC = wpp386
LD = wlink
RC = wrc

# Machine type -5r Pent -6r Pent Pro
MACHINE= -6r

#Optimization None: -od Time opt: -ot Fastest possible: -otexan
#OPT= -otexan
OPT=-ot

INCLUDE = $(%watcom)\h;$(%watcom)\h\nt;.\

# suppress warnings 555 and 013
MISC    = -wcd555 -wcd013 -wcd726

DEFS    = -DNT -DNTCONSOLE -DMSVC -DWIN32 -D_CONSOLE -DWATCOM -DUSE_LOCALE -DNTOW
CFLAGS  = -i=$(INCLUDE) $(MISC) $(DEFS) -d0 -w4 -e25 -zq $(OPT) $(MACHINE) -bm -bt=NT -mf -xs -xr
LDFLAGS = op m op maxe=25 op q op symf op el
OEXT    = obj

.EXTENSIONS:.rc .res

!include objs.mif

.cpp.obj:
  $(CC) $(CFLAGS) $<

all: cfte.exe fte.exe fte.cnf

cfte.exe: $(CFTE_OBJS) cfte.def
  $(LD) NAME cfte SYS nt $(LDFLAGS) FILE {$(CFTE_OBJS)}

defcfg.cnf: defcfg.fte cfte.exe
  cfte defcfg.fte defcfg.cnf

defcfg.h: defcfg.cnf bin2c.exe
  bin2c defcfg.cnf >defcfg.h

fte.cnf: cfte.exe
  cfte ..\config\main.fte fte.cnf

bin2c.exe: bin2c.obj
  $(LD) NAME bin2c SYS nt $(LDFLAGS) FILE {bin2c.obj}

c_config.obj: c_config.cpp defcfg.h

ftewin32.res:
  $(RC) -r ftewin32.rc

fte.exe: $(OBJS) $(NTOBJS) ftewin32.res
  $(LD) NAME fte SYS nt $(LDFLAGS) FILE {$(OBJS) $(NTOBJS)}
  $(RC) ftewin32.res fte.exe

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


