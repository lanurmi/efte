INCDIR    =
LIBDIR    =

OPTIMIZE  = /O2 /MT
#OPTIMIZE  = /Zi /Od /MTd

#DEBUG     = -DMSVCDEBUG

CC        = cl
LD        = cl

OEXT=obj

#APPOPTIONS = -DDEFAULT_INTERNAL_CONFIG
#/Fm /GF /J
CCFLAGS   = $(OPTIMIZE) -DNT -DNTCONSOLE -DMSVC $(INCDIR) /DWIN32 /D_CONSOLE \
	$(APPOPTIONS) $(DEBUG)\
	/nologo /W3 /J # /YX
LDFLAGS   = $(OPTIMIZE) $(LIBDIR) /nologo

.SUFFIXES: .cpp .$(OEXT)

!include objs.inc

.cpp.$(OEXT):
	$(CC) $(CCFLAGS) -c $<

.c.$(OEXT):
	$(CC) $(CCFLAGS) -c $<

all: cfte.exe fte.cnf fte.exe

clean:
	-del bin2c.exe
	-del bin2c.pdb
	-del cfte.exe
	-del cfte.pdb
	-del cfte.exp
	-del cfte.lib
	-del defcfg.cnf
	-del defcfg.h
	-del fte.cnf
	-del fte.exe
	-del fte.his
	-del fte.pdb
	-del vc60.pdb
	-del *.obj

cfte.exe: $(CFTE_OBJS) cfte.def
	$(LD) $(LDFLAGS) /Fecfte.exe $(CFTE_OBJS) cfte.def

defcfg.cnf: defcfg.fte cfte.exe
	cfte defcfg.fte defcfg.cnf

defcfg.h: defcfg.cnf bin2c.exe
	bin2c defcfg.cnf >defcfg.h

fte.cnf: ..\config\* cfte.exe
	cfte ..\config\main.fte fte.cnf

bin2c.exe: bin2c.cpp
	$(CC) $(CCFLAGS) $(LDFLAGS) /Febin2c.exe bin2c.cpp

c_config.$(OEXT): defcfg.h

fte.exe: $(OBJS) $(NTOBJS)
	$(LD) $(LDFLAGS) /Fefte.exe $(OBJS) $(NTOBJS) user32.lib

distro: fte.exe fte.cnf cfte.exe
	zip ../fte-nt.zip fte.exe fte.cnf cfte.exe

