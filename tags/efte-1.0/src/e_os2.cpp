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
    char file[MAXPATH] = "";
    char cmd[1024];

    if (State.GetStrParam(this, file, sizeof(file) - 1) == 0)
        if (MView->Win->GetStr("Help file",
                               sizeof(file) - 1, file, HIST_DEFAULT) == 0)
            return 0;

    char wordAsk[64] = "";
    if (word == 0) {
        if (State.GetStrParam(this, wordAsk, sizeof(wordAsk) - 1) == 0)
            if (MView->Win->GetStr("Keyword",
                                   sizeof(wordAsk) - 1, wordAsk, HIST_DEFAULT) == 0)
                return 0;
        word = wordAsk;
    }

    sprintf(cmd, "%s %s %s", HelpCommand, file, word);

    if (system(cmd) != 0) {
        Msg(S_ERROR, "Failed to start view.exe!");
        return 0;
    }
    return 1;
}
