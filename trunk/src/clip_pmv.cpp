/*    clip_pm.cpp
 *
 *    Copyright (c) 1994-1998, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"
#include "clip.h"

#define INCL_WIN
#define INCL_DOS
#define INCL_ORDINALS
#include <os2.h>
#include <string.h>
#include <stdlib.h>

/*
 *     Pointers to PM functions
 *     Instead of calling PM directly, we are obtaining these pointers
 * on first access to clipboard.
 *     All these prototypes are copied from OS2.H
 *     Note: all prototypes are 32-bit version only
 */

#ifndef INCL_32
#error Prototypes are for 32-bit compiler only
#endif

HAB    APIENTRY (*p_WinInitialize)(ULONG flOptions);
BOOL   APIENTRY (*p_WinTerminate)(HAB hab);
HMQ    APIENTRY (*p_WinCreateMsgQueue)(HAB hab, LONG cmsg);
BOOL   APIENTRY (*p_WinDestroyMsgQueue)(HMQ hmq);
BOOL   APIENTRY (*p_WinEmptyClipbrd)(HAB hab);
BOOL   APIENTRY (*p_WinOpenClipbrd)(HAB hab);
BOOL   APIENTRY (*p_WinCloseClipbrd)(HAB hab);
BOOL   APIENTRY (*p_WinSetClipbrdData)(HAB hab, ULONG ulData, ULONG fmt, ULONG rgfFmtInfo);
ULONG  APIENTRY (*p_WinQueryClipbrdData)(HAB hab, ULONG fmt);

static struct impentry {
    ULONG ordinal;
    PFN *pointer;
} imported_functions[] = {
    { ORD_WIN32INITIALIZE,     (PFN *) &p_WinInitialize       },
    { ORD_WIN32TERMINATE,      (PFN *) &p_WinTerminate        },
    { ORD_WIN32CREATEMSGQUEUE, (PFN *) &p_WinCreateMsgQueue   },
    { ORD_WIN32DESTROYMSGQUEUE,(PFN *) &p_WinDestroyMsgQueue  },
    { ORD_WIN32EMPTYCLIPBRD,   (PFN *) &p_WinEmptyClipbrd     },
    { ORD_WIN32OPENCLIPBRD,    (PFN *) &p_WinOpenClipbrd      },
    { ORD_WIN32CLOSECLIPBRD,   (PFN *) &p_WinCloseClipbrd     },
    { ORD_WIN32SETCLIPBRDDATA, (PFN *) &p_WinSetClipbrdData   },
    { ORD_WIN32QUERYCLIPBRDDATA,(PFN *)&p_WinQueryClipbrdData },
    { 0, 0 }
};

/*
 *    Load PMWIN.DLL and get pointers to all reqd PM functions
 */

static BOOL loadDLL(void) {
    static BOOL loaded;
    static BOOL loaded_ok;
    static HMODULE pmwin;

    char error[200];

    if (loaded)
        return loaded_ok;
    loaded = TRUE;

    if (DosLoadModule((PSZ)error, sizeof(error), (PSZ)"PMWIN.DLL", &pmwin) == 0) {
        struct impentry *imp;
        for (imp = imported_functions; imp->ordinal; imp++)
            if (DosQueryProcAddr(pmwin, imp->ordinal, NULL, imp->pointer) != 0)
                goto byebye;
        loaded_ok = TRUE;
    }
    byebye:
    return loaded_ok;
}

/*
 AccessPmClipboard:
 Purpose: perform all necessary PM stuff and open clipboard for access;
 Return : TRUE on success, FALSE on error;
 Note   : on error, clipboard is released automatically.

 LeavePmClipboard:
 Purpose: releases previously opened clipboard and clear all PM stuff
 Return : none
 */

static struct {
    PPIB ppib;
    HAB hab;
    HMQ hmq;
    ULONG savedtype;
    BOOL opened;
} PmInfo;

static void LeavePmClipboard(void) {

    if (PmInfo.opened)
        p_WinCloseClipbrd(PmInfo.hab);
    if (PmInfo.hmq)
        p_WinDestroyMsgQueue(PmInfo.hmq);
    if (PmInfo.hab)
        p_WinTerminate(PmInfo.hab);
    PmInfo.ppib->pib_ultype = PmInfo.savedtype;
}

static BOOL AccessPmClipboard(void) {
    PTIB ptib;

    if (loadDLL() == FALSE) {
        DosBeep(100, 1500);
        return FALSE;
    }

    memset(&PmInfo, 0, sizeof(PmInfo));

    // mutate into PM application for clipboard (Win**) functions access
    DosGetInfoBlocks(&ptib, &PmInfo.ppib);
    PmInfo.savedtype = PmInfo.ppib->pib_ultype;
    PmInfo.ppib->pib_ultype = PROG_PM;

    if ((PmInfo.hab = p_WinInitialize(0)) != NULLHANDLE) {
        if ((PmInfo.hmq = p_WinCreateMsgQueue(PmInfo.hab, 0)) != NULLHANDLE) {
            if (p_WinOpenClipbrd(PmInfo.hab) == TRUE) {
                PmInfo.opened = TRUE;
            }
        }
    }
    if (PmInfo.opened != TRUE) {
        LeavePmClipboard();
        DosBeep(100, 1500);
    }
    return PmInfo.opened;
}

int GetClipText(ClipData *cd) {
    int rc = -1;
    char *text;

    cd->fLen = 0;
    cd->fChar = 0;

    if (AccessPmClipboard() != TRUE)
        return rc;

    if ((text = (char *) p_WinQueryClipbrdData(PmInfo.hab, CF_TEXT)) != 0) {
        cd->fLen = strlen(text);
        cd->fChar = strdup(text);
        rc = 0;
    }

    LeavePmClipboard();
    return rc;
}

int PutClipText(ClipData *cd) {
    ULONG len;
    void *text;
    int rc = -1;

    if (AccessPmClipboard() != TRUE)
        return rc;

    p_WinEmptyClipbrd(PmInfo.hab);
    len = cd->fLen;

    if (len) {
        DosAllocSharedMem((void **)&text,
                          0,
                          len + 1,
                          PAG_READ | PAG_WRITE | PAG_COMMIT | OBJ_GIVEABLE);
        strncpy((char *)text, cd->fChar, len + 1);
        if (!p_WinSetClipbrdData(PmInfo.hab, (ULONG) text, CF_TEXT, CFI_POINTER))
            DosBeep(100, 1500);
        else
            rc = 0;
    }

    LeavePmClipboard();
    return rc;
}
