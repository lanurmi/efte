
#----------------------------------------------------------------------------
#
# Makefile for NT version of eFTE using OpenWatcom 1.4
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

all: cefte.exe efte.exe efte.cnf

cefte.exe: $(CFTE_OBJS) cfte.def
  $(LD) NAME ecfte SYS nt $(LDFLAGS) FILE {$(CFTE_OBJS)}

defcfg.cnf: defcfg.fte cefte.exe
  cefte defcfg.fte defcfg.cnf

defcfg.h: defcfg.cnf bin2c.exe
  bin2c defcfg.cnf >defcfg.h

efte.cnf: cefte.exe
  cefte ..\config\main.fte efte.cnf

bin2c.exe: bin2c.obj
  $(LD) NAME bin2c SYS nt $(LDFLAGS) FILE {bin2c.obj}

c_config.obj: c_config.cpp defcfg.h

eftewin32.res:
  $(RC) -r ftewin32.rc eftewin32.res

efte.exe: $(OBJS) $(NTOBJS) eftewin32.res
  $(LD) NAME efte SYS nt $(LDFLAGS) FILE {$(OBJS) $(NTOBJS)}
  $(RC) eftewin32.res efte.exe

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


