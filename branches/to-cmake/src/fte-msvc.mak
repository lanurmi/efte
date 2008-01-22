INCDIR    =
LIBDIR    =

OPTIMIZE  = /O2 /MT
#OPTIMIZE  = /Zi /Od /MTd

#DEBUG     = -DMSVCDEBUG

CC        = cl
LD        = cl
RC	  = rc

OEXT=obj

#APPOPTIONS = -DDEFAULT_INTERNAL_CONFIG
#/Fm /GF /J

# Use these settings for MSVC 6
#CCFLAGS   = $(OPTIMIZE) -DNT -DNTCONSOLE -DMSVC -DWIN32 -D_CONSOLE -DNO_NEW_CPP_FEATURES $(INCDIR) /GX \
#	$(APPOPTIONS) $(DEBUG) \
#	/nologo /W3 /J # /YX

# Use these settings for MSVC 2003
#CCFLAGS   = $(OPTIMIZE) -DNT -DNTCONSOLE -DMSVC -DWIN32 -D_CONSOLE $(INCDIR) /GX \
#	$(APPOPTIONS) $(DEBUG) \
#	/nologo /W3 /J # /YX

# Use these settings for MSVC Express 2008
CCFLAGS   = $(OPTIMIZE) -DNT -D_CRT_NONSTDC_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS \
	-DNTCONSOLE -DMSVC -DWIN32 -D_CONSOLE $(INCDIR) /EHsc \
	$(APPOPTIONS) $(DEBUG) \
	/nologo /W3 /J

LDFLAGS   = $(OPTIMIZE) $(LIBDIR) /nologo
RCFLAGS   =

.SUFFIXES: .cpp .$(OEXT)

!include objs.inc

NTRES     = ftewin32.res

.cpp.$(OEXT):
	$(CC) $(CCFLAGS) -c $<

.c.$(OEXT):
	$(CC) $(CCFLAGS) -c $<

all: cefte.exe efte.cnf efte.exe

clean:
	-del bin2c.exe
	-del bin2c.pdb
	-del cefte.exe
	-del cefte.pdb
	-del cefte.exp
	-del cefte.lib
	-del defcfg.cnf
	-del defcfg.h
	-del efte.cnf
	-del efte.exe
	-del efte.his
	-del efte.pdb
	-del vc60.pdb
	-del eftewin32.res
	-del *.obj

cefte.exe: $(CFTE_OBJS) cfte.def
	$(LD) $(LDFLAGS) /Fecefte.exe $(CFTE_OBJS) cfte.def

defcfg.cnf: defcfg.fte cefte.exe
	cefte defcfg.fte defcfg.cnf

defcfg.h: defcfg.cnf bin2c.exe
	bin2c defcfg.cnf >defcfg.h

efte.cnf: ..\config\* cefte.exe
	cefte ..\config\main.fte efte.cnf

bin2c.exe: bin2c.cpp
	$(CC) $(CCFLAGS) $(LDFLAGS) /Febin2c.exe bin2c.cpp

c_config.$(OEXT): defcfg.h

ftewin32.res: ftewin32.rc
	$(RC) $(RCFLAGS) ftewin32.rc

efte.exe: $(OBJS) $(NTOBJS) $(NTRES)
	$(LD) $(LDFLAGS) /Feefte.exe $(OBJS) $(NTOBJS) user32.lib $(NTRES)

distro: efte.exe efte.cnf cefte.exe
	zip ../efte-nt.zip efte.exe efte.cnf cefte.exe

