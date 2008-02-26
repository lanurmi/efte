/*    e_win32.cpp
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1997, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

// Win32 (NT) specific routines

#include "fte.h"

// Silence warnings about redefinition, we don't need this here anyway.
#undef SEARCH_ALL

#include <windows.h>

int EView::SysShowHelp(ExState &State, const char *word) {
    char wordAsk[64] = "";
    if (word == 0) {
        if (sstack.size() == 0) {
            Msg(S_ERROR, "String stack underflow error in SysShowHelp");
            SetBranchCondition(0);
            return 0;
        }

        strcpy(wordAsk, sstack.back().c_str()); sstack.pop_back();

        if (strlen(wordAsk) == 0) {
            if (MView->Win->GetStr("Keyword", sizeof(wordAsk) - 1, wordAsk, HIST_DEFAULT) == 0) {
                SetBranchCondition(0);
                return 0;
            }
        }

        word = wordAsk;
    }


    if (sstack.size() == 0) {
        Msg(S_ERROR, "String stack underflow error in SysShowHelp");
        SetBranchCondition(0);
        return 0;
    }

    char file[MAXPATH] = "";
    strcpy(file, sstack.back().c_str()); sstack.pop_back();

    if (strlen(file) == 0) {
        if (MView->Win->GetStr("Help file", sizeof(file) - 1, file, HIST_DEFAULT) == 0) {
            SetBranchCondition(0);
            return 0;
        }
    }

    //** Start WinHelp,
    if (!WinHelp(0, file, HELP_KEY, (DWORD)word)) {
        Msg(S_ERROR, "Failed to start WinHelp!");
        return 0;
    }
    return 1;
}
