/*    e_os2.cpp
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

int EView::SysShowHelp(ExState &State, const char *word) {
    if (sstack.size() < 2) {
        Msg(S_ERROR, "String stack underflow in SysShowHelp (%i)", sstack.size());
        SetBranchCondition(0);
        return 0;
    }

    char file[MAXPATH] = "";
    char wordAsk[64] = "";

    strcpy(wordAsk, sstack.back().c_str()); sstack.pop_back();
    strcpy(file, sstack.back().c_str()); sstack.pop_back();

    if (strlen(file) == 0) {
        if (MView->Win->GetStr("Help file", sizeof(file) - 1, file, HIST_DEFAULT) == 0) {
            SetBranchCondition(0);
            return 0;
        }
    }

    if (word == 0) {
        if (strlen(wordAsk) == 0) {
            if (MView->Win->GetStr("Keyword", sizeof(wordAsk) - 1, wordAsk, HIST_DEFAULT) == 0) {
                SetBranchCondition(0);
                return 0;
            }
        }
        word = wordAsk;
    }

    char cmd[1024];
    sprintf(cmd, "%s %s %s", HelpCommand, file, word);

    if (system(cmd) != 0) {
        Msg(S_ERROR, "Failed to start view.exe!");
        return 0;
    }
    return 1;
}
