/*    clip_pm.cpp
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"
#include "clip.h"

#define INCL_WIN
#include <os2.h>
#include <string.h>
#include <stdlib.h>

extern HAB hab;

int GetClipText(ClipData *cd) {
    char *text;

    cd->fLen = 0;
    cd->fChar = 0;
    if ((WinOpenClipbrd(hab) == TRUE) &&
            ((text = (char *) WinQueryClipbrdData(hab, CF_TEXT)) != 0)) {
        cd->fLen = strlen(text);
        cd->fChar = strdup(text);
    }
    WinCloseClipbrd(hab);
    return 0;
}

int PutClipText(ClipData *cd) {
    ULONG len;
    void *text;

    if (WinOpenClipbrd(hab) == TRUE) {
        WinEmptyClipbrd(hab);
        len = cd->fLen;

        if (len) {
            DosAllocSharedMem((void **)&text,
                              0,
                              len + 1,
                              PAG_READ | PAG_WRITE | PAG_COMMIT | OBJ_GIVEABLE);
            strncpy((char *)text, cd->fChar, len + 1);
            if (!WinSetClipbrdData(hab, (ULONG) text, CF_TEXT, CFI_POINTER))
                DosBeep(100, 1500);

        }
        WinCloseClipbrd(hab);
    }
    return 0;
}
