/* w_execute.cpp
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"

int nestlevel = 0;
#define NEST   nestlevel++
#define UNNEST --nestlevel

int Tick() {
    SSCHECK(1, "tick");
    std::string cname = sstack.back(); sstack.pop_back();
    ParamStack.push(CmdNum(cname.c_str()));
    return 1;
}

int EGUI::Execute(ExState &State, GxView *view) {
    PSCHECK(1, "execute");
    NEST;
    int ret = ExecCommand(view, ParamStack.pop(), State);
    UNNEST;
    return ret;
}

int EGUI::ExecuteCommand(ExState &State, GxView *view)
{
    ExModelView *V = (ExModelView *)view->Top;
    EView *View = V->View;
    char name[64] = "";

    if (View->MView->Win->GetStr("Command name", sizeof(name), name,
                                 HIST_DEFAULT) == 0)
        return 0;

    int command = CmdNum(name);
    if (command == 0) {
        View->Msg(S_INFO, "%s is an unknown command", name);
        SetBranchCondition(0);
        return 0;
    }

    return ExecCommand(view, command, State);

}
