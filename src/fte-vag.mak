#
#
#  Makefile for VisualAge C++ version 3.00
#
#

EXE =

OEXT = obj

DEBUG = 0
#DEBUG = 1

CC = icc
LINK = ilink
RC = rc
VOID = echo > NUL

C_OPTIONS = /Q /Tl /G4 /Gm+ /DOS2 /DINCL_32
C_OPT_R = /O /Gs- 
C_OPT_D = /Ti /Tx
CPP_OPTIONS = /Q /G4 /Gm+ /DOS2 /DINCL_32
CPP_OPT_R = /O /Gs-
CPP_OPT_D = /Ti /Tm /Tx 
L_OPTIONS = /BASE:0x010000 /EXEC /NOE /NOLOGO
L_OPT_R = /EXEPACK:2 /PACKC /PACKD /OPTF 
L_OPT_D = /DEBUG /DBGPACK
RC_OPT = -n

C_SRC =
C_H =
CPP_SRC	= 
CPP_HPP =

!include objs.inc

RES = 

LIBS = 
HLIB =

DEF = 


.SUFFIXES:
.SUFFIXES: .cpp .rc

all: cfte.exe fte.exe ftepm.exe clipserv.exe cliputil.exe

clipserv.exe: clipserv.$(OEXT) clipserv.def
!IF $(DEBUG)
 	$(VOID) <<clipserv.lnk
clipserv.$(OEXT) $(L_OPTIONS) $(L_OPT_D) 
/OUT:$@ /MAP:clipserv.MAP
$(LIBS)
clipserv.def
<< 
!ELSE
	$(VOID) <<clipserv.lnk
clipserv.$(OEXT)  $(L_OPTIONS) $(L_OPT_R) 
/OUT:$@ /MAP:clipserv.MAP
$(LIBS)
clipserv.def
<< 
!ENDIF
	$(LINK) @clipserv.lnk

cliputil.exe: cliputil.$(OEXT) clip_vio.$(OEXT) cliputil.def
!IF $(DEBUG)
 	$(VOID) <<cliputil.lnk
cliputil.$(OEXT) clip_vio.$(OEXT) $(L_OPTIONS) $(L_OPT_D) 
/OUT:$@ /MAP:cliputil.MAP
$(LIBS)
cliputil.def
<< 
!ELSE
	$(VOID) <<cliputil.lnk
cliputil.$(OEXT) clip_vio.$(OEXT) $(L_OPTIONS) $(L_OPT_R) 
/OUT:$@ /MAP:cliputil.MAP
$(LIBS)
cliputil.def
<< 
!ENDIF
	$(LINK) @cliputil.lnk

cfte.exe: $(CFTE_OBJS) cfte.def
!IF $(DEBUG)
 	$(VOID) <<cfte.lnk
$(CFTE_OBJS) $(L_OPTIONS) $(L_OPT_D) 
/OUT:$@ /MAP:cfte.MAP
$(LIBS)
cfte.def
<< 
!ELSE
 	$(VOID) <<cfte.lnk
$(CFTE_OBJS) $(L_OPTIONS) $(L_OPT_R) 
/OUT:$@ /MAP:cfte.MAP
$(LIBS)
cfte.def
<< 
!ENDIF
	$(LINK) @cfte.lnk

defcfg.cnf: defcfg.fte cfte.exe
	cfte defcfg.fte defcfg.cnf

defcfg.h: defcfg.cnf bin2c.exe
	bin2c defcfg.cnf >defcfg.h

bin2c.obj: bin2c.cpp

bin2c.exe: bin2c.obj
!IF $(DEBUG)
 	$(VOID) <<bin2c.lnk
bin2c.obj $(L_OPTIONS) $(L_OPT_D) /PM:VIO
/OUT:$@ /MAP:bin2c.MAP
$(LIBS)
$(DEF)
<< 
!ELSE
 	$(VOID) <<bin2c.lnk
bin2c.obj $(L_OPTIONS) $(L_OPT_R) /PM:VIO
/OUT:$@ /MAP:bin2c.MAP
$(LIBS)
$(DEF)
<< 
!ENDIF
	$(LINK) @bin2c.lnk

c_config.$(OEXT): defcfg.h

fte.exe: $(OBJS) $(VIOOBJS) fte.def
!IF $(DEBUG)
 	$(VOID) <<fte.lnk
$(OBJS) $(VIOOBJS) $(L_OPTIONS) $(L_OPT_D) 
/OUT:$@ /MAP:fte.MAP
$(LIBS)
fte.def
<< 
!ELSE
	$(VOID) <<fte.lnk
$(OBJS) $(VIOOBJS) $(L_OPTIONS) $(L_OPT_R) 
/OUT:$@ /MAP:fte.MAP
$(LIBS)
fte.def
<< 
!ENDIF
	$(LINK) @fte.lnk

ftepm.res: ftepm.rc pmdlg.rc

ftepm.exe: $(OBJS) $(PMOBJS) ftepm.def ftepm.res
!IF $(DEBUG)
 	$(VOID) <<ftepm.lnk
$(OBJS) $(PMOBJS) $(L_OPTIONS) $(L_OPT_D) 
/OUT:$@ /MAP:ftepm.MAP
$(LIBS)
ftepm.def
<< 
!ELSE
	$(VOID) <<ftepm.lnk
$(OBJS) $(PMOBJS) $(L_OPTIONS) $(L_OPT_R) 
/OUT:$@ /MAP:ftepm.MAP
$(LIBS)
ftepm.def
<< 
!ENDIF
	$(LINK) @ftepm.lnk
	$(RC) $(RC_OPT) ftepm.res ftepm.EXE

$(EXE).EXE: $(OBJS) $(C_SRC:.c=.obj) $(CPP_SRC:.cpp=.obj) $(RES) $(DEF) $(LIBS)
!IF $(DEBUG)
 	$(VOID) <<$(EXE).lnk
$(OBJS) $(C_SRC:.c=.obj) $(CPP_SRC:.cpp=.obj) $(L_OPTIONS) $(L_OPT_D) 
/OUT:$@ /MAP:$(EXE).MAP
$(LIBS)
$(DEF)
<< 
!ELSE
	$(VOID) <<$(EXE).lnk
$(OBJS) $(C_SRC:.c=.obj) $(CPP_SRC:.cpp=.obj) $(L_OPTIONS) $(L_OPT_R) 
/OUT:$@ /MAP:$(EXE).MAP
$(LIBS)
$(DEF)
<< 
!ENDIF
	$(LINK) @$(EXE).lnk
	$(RC) $(RC_OPT) $(RES) $(EXE).EXE


# $(C_SRC:.c=.obj): $(C_SRC) $(C_H)

# $(CPP_SRC:.cpp=.obj): $(CPP_SRC) $(CPP_HPP)

# $(RES): $(RES:.res=.rc)

.C.$(OEXT):
!IF $(DEBUG)
	$(CC) /C $(C_OPTIONS) $(C_OPT_D) $<
!ELSE        
	$(CC) /C $(C_OPTIONS) $(C_OPT_R) $<
!ENDIF    
    
.CPP.$(OEXT):
!IF $(DEBUG)
	$(CC) /C $(CPP_OPTIONS) $(CPP_OPT_D) $<
!ELSE        
	$(CC) /C $(CPP_OPTIONS) $(CPP_OPT_R) $<
!ENDIF        

.RC.RES:
	$(RC) -r $(RC_OPT) $<
