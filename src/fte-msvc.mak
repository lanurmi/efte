INCDIR    =
LIBDIR    =

OPTIMIZE  = /O2
#OPTIMIZE  = /Zi /Od

CC        = cl
LD        = cl

OEXT=obj

#/Fm /GF /J
CCFLAGS   = $(OPTIMIZE) -DNT -DNTCONSOLE -DMSVC $(INCDIR) /DWIN32 /D_CONSOLE \
	/nologo /MTd /W3 /J # /YX
LDFLAGS   = $(OPTIMIZE) $(LIBDIR) /nologo /MTd

.SUFFIXES: .cpp .$(OEXT)

include objs.inc

.cpp.$(OEXT):
	$(CC) $(CCFLAGS) -c $<

.c.$(OEXT):
	$(CC) $(CCFLAGS) -c $<

all: cfte.exe fte.exe

cfte.exe: $(CFTE_OBJS) cfte.def
	$(LD) $(LDFLAGS) /Fecfte.exe $(CFTE_OBJS) cfte.def

defcfg.cnf: defcfg.fte cfte.exe
	cfte defcfg.fte defcfg.cnf

defcfg.h: defcfg.cnf bin2c.exe
	bin2c defcfg.cnf >defcfg.h

bin2c.exe: bin2c.cpp
	$(CC) $(CCFLAGS) $(LDFLAGS) /Febin2c.exe bin2c.cpp

c_config.$(OEXT): defcfg.h

fte.exe: $(OBJS) $(NTOBJS)
	$(LD) $(LDFLAGS) /Fefte.exe $(OBJS) $(NTOBJS) user32.lib

