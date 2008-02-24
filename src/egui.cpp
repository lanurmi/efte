/*    egui.cpp
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"
#include "log.h"

int LastEventChar = -1;

EFrame::EFrame(int XSize, int YSize): GFrame(XSize, YSize) {
    CMap = 0;
    CModel = 0;
}

EFrame::~EFrame() {
}

void EFrame::Update() {
    GxView *V = (GxView *)Active;

    if (V) {
        if (CModel != ActiveModel && ActiveModel) {
            char Title[256] = ""; //fte: ";
            char STitle[256] = ""; //"fte: ";
            ActiveModel->GetTitle(Title, sizeof(Title) - 1,
                                  STitle, sizeof(STitle) - 1);
            ConSetTitle(Title, STitle);
            CModel = ActiveModel;
        }
    }
    GFrame::Update();
}

void EFrame::UpdateMenu() {
    GxView *V = (GxView *)Active;
    EEventMap *Map = 0;

    if (V)
        Map = V->GetEventMap();

    if (Map != CMap || CMap == 0) {
        const char *NMenu = 0;
        const char *OMenu = 0;
        // set menu

        if (CMap)
            OMenu = CMap->GetMenu(EM_MainMenu);
        if (Map)
            NMenu = Map->GetMenu(EM_MainMenu);
        if (NMenu == 0)
            NMenu = "Main";
        CMap = Map;

        if (OMenu && strcmp(OMenu, NMenu) == 0) {
            // ok
        } else {
            SetMenu(NMenu);
        }
    } /*else if (CMap == 0 && Map == 0) {
        SetMenu("Main");
    }*/

    GFrame::UpdateMenu();
}

EGUI::EGUI(int &argc, char **argv, int XSize, int YSize)
        : GUI(argc, argv, XSize, YSize) {
    ActiveMap = 0;
    OverrideMap = 0;
    CharMap[0] = 0;
}

EGUI::~EGUI() {
}

/**
 * MACRO: Print a string to the console
 */

int Print(GxView *view, ExState &State) {
    FILE *f;
    int whereTo, found, vI;
    char vS[256];
    EView *View = 0;

    if (view != NULL) {
        ExModelView *V = (ExModelView *)view->Top;
        View = V->View;
    }

    if (State.GetIntParam(View, &whereTo) == 0)
        whereTo = 1;

    f = whereTo == 2 ? stderr : stdout;

    do {
        found = 1;
        if (State.GetIntParam(View, &vI) != 0)
            fprintf(f, "%i", vI);
        else if (State.GetStrParam(View, vS, sizeof(vS)) != 0)
            fprintf(f, vS);
        else
            found = 0;
    } while (found == 1);

    SetBranchCondition(1);
    return 1;
}

/**
 * MACRO: Push a number/char from a macro onto the stack
 */
int Push(GxView *view, ExState &State) {
    int found;
    int x = 0;
    char s[64] = "";
    EView *View = 0;

    if (view != NULL) {
        ExModelView *V = (ExModelView *)view->Top;
        View = V->View;
    }

    do {
        found = 0;
        if (State.GetIntParam(View, &x)) {
            ParamStack.push(x);
            found = 1;
        } else if (State.GetStrParam(View, s, sizeof(s))) {
            ParamStack.push((int) s[0]);
            found = 1;
        }
    } while(found == 1);

    SetBranchCondition(1);
    return 1;
}

/**
 * MACRO: Print a stack diagnostic message to stderr
 */
int EGUI::Diag(ExState &State) {
    int count;
    char msg[256];

    if (State.GetIntParam(0, &count) == 0)
        count = 0;
    if (State.GetStrParam(0, msg, sizeof(msg)))
        fprintf(stderr, "%s: ", msg);

    fprintf(stderr, "cond=%08x, tos=%d, nos=%d, 3rd=%d", BranchCondition, ParamStack.peek(0), ParamStack.peek(1), ParamStack.peek(2));
    for (int i=3; i <= count; i++)
        fprintf(stderr, ", %ith=%d", i, ParamStack.peek(i));
    fprintf(stderr, "\n");

    return 1;
}

// --- arithmetic ---

int EGUI::Plus() {
    ParamStack.push(ParamStack.pop()+ParamStack.pop());
    return 1;
}

// don't really need - could provide "Invert" and do
//2's complement add in macro
int EGUI::Minus() {
    int tos=ParamStack.pop();
    ParamStack.push(+ParamStack.pop()-tos);
    return 1;
}


int EGUI::Mul() {
    ParamStack.push(ParamStack.pop()*ParamStack.pop());
    return 1;
}


int EGUI::Div() {
    int tos=ParamStack.pop();
    if (!tos) return 0;           // div by 0
    ParamStack.push(ParamStack.pop()/tos);
    return 1;
}

// --- bits ---

int EGUI::And() {
    ParamStack.push(ParamStack.pop() & ParamStack.pop());
    return 1;
}

int EGUI::Or() {
    ParamStack.push(ParamStack.pop() | ParamStack.pop());
    return 1;
}

int EGUI::Xor() {
    ParamStack.push(ParamStack.pop() ^ ParamStack.pop());
    return 1;
}

int EGUI::Shift() {
    int tos = ParamStack.pop();                         // shift count and direction
    if (tos) {
        unsigned int nos = ParamStack.pop();            // shift value
        if (tos < 0)  {
            ParamStack.push(nos>>(-tos));
        } else {
            ParamStack.push(nos << tos);
        }
    }
    return 1;
}


// --- comparison ---

// replace top two stack items against identity flag
int EGUI::Equals() {
    ParamStack.push(-(ParamStack.pop() == ParamStack.pop()));
    return 1;
}

// true if 2nd item less than top item:
//  3 4 Less   ( true )
int EGUI::Less() {
    ParamStack.push(-(ParamStack.pop() > ParamStack.pop()));
    return 1;
}

// interface condition, provided by old commands, to
// condition reading of new commands (passed on stack)
// old commands buffer their conditions, until read
// and transported to stack by "Flag"
// That way, the number of test results and conditions
// provided by old commands is irrelevant. we can choose
// to use or ignore as we see fit.
// as soon we need one of the last n result flags, each
// execution of Flag delivers the next, back into history.
int EGUI::Flag() {
    // TODO: warning C4146: unary minus operator applied to unsigned type, result still unsigned
    ParamStack.push(-(BranchCondition & 1));
    BranchCondition = (BranchCondition >> 1);
    return 1;
}

// old commands return false on fail, causing termination of macro execution.
// this prevents the macro to deal with the condition, therefore those commands
// should return "true".
// Because we want to be able to mimick the former behaviour, the condition, as
// left by those commands, will be read by "Abort" and returned to macro interpreter
// as return code. i.e. a command, followed by Abort, will terminate macro execution
// if its condition was "false".
// this replaces the previous idea of not terminating macro execution if a command is
// followed by a branch - which doesn't work. simply negating the test condition, instead
// of branching directly, would render this unfunctional already. So we have instead:
// macro  { FailingCommand MacroContinues ... }   or
// macro  { FailingCommand Abort MacroHasTerminated } or
// macro  { SuccessfulCommand Abort MacroContinues ... }
int EGUI::Abort() {
    int failcondition=BranchCondition & 1;
    BranchCondition = (BranchCondition >> 1);
    return failcondition;
}

// --- stack ---

int EGUI::Dup() {
    ParamStack.dup();
    return 1;
}

int EGUI::Drop() {
    ParamStack.pop();
    return 1;
}

int EGUI::Swap() {
    ParamStack.swap();
    return 1;
}

int EGUI::Over() {
    ParamStack.push(ParamStack.peek(1));
    return 1;
}

int EGUI::Rot() {
    int tos = ParamStack.pop();
    ParamStack.swap();
    ParamStack.push(tos);
    ParamStack.swap();
    return 1;
}


// --- stack2 ---
int EGUI::ToR() {
    ControlStack.push(ParamStack.pop());
    return 1;
}

int EGUI::RFrom() {
    ParamStack.push(ControlStack.pop());
    return 1;
}

int EGUI::RFetch() {
    ParamStack.push(ControlStack.peek(0));
    return 1;
}


int EGUI::I() {
    ParamStack.push(ControlStack.peek(0));
    return 1;
}

int EGUI::J() {
    ParamStack.push(ControlStack.peek(2));
    return 1;
}

int PushStr(GxView *view, ExState &State) {
    int found;
    int x = 0;
    char s[1024] = "";
    EView *View = 0;

    if (view != NULL) {
        ExModelView *V = (ExModelView *)view->Top;
        View = V->View;
    }

    do {
        found = 0;
        if (State.GetIntParam(View, &x)) {
            snprintf(s, 1024, "%i", x);
            sstack.push_back(s);
            found = 1;
        } else if (State.GetStrParam(View, s, sizeof(s))) {
            sstack.push_back(s);
            found = 1;
        }
    } while(found == 1);

    SetBranchCondition(1);
    return 1;
}

int DupStr() {
    if (sstack.size() == 0) {
        SetBranchCondition(0);
        return 0;
    }

    sstack.push_back(sstack[sstack.size()-1]);

    SetBranchCondition(1);
    return 1;
}

int DropStr() {
    if (sstack.size() == 0) {
        SetBranchCondition(0);
        return 0;
    }

    sstack.pop_back();

    SetBranchCondition(1);
    return 1;
}

int SwapStr() {
    int tosIdx = sstack.size();
    if (tosIdx < 2)
        return 0;

    tosIdx--;

    std::string tos = sstack.at(tosIdx);
    sstack[tosIdx] = sstack[tosIdx - 1];
    sstack[tosIdx - 1] = tos;

    SetBranchCondition(1);
    return 1;
}

int DiagStr(ExState &State) {
    char msg[128];
    int ssize = sstack.size();

    if (State.GetStrParam(0, msg, sizeof(msg)))
        fprintf(stderr, "%s: ", msg);

    if (ssize == 0)
        fprintf(stderr, "empty");
    else {
        int tos = sstack.size()-1;
        for (int i = ssize - 1; i >= 0; i--) {
            fprintf(stderr, "%i='%s'", i, sstack[tos-i].c_str());
            if (i != 0) fprintf(stderr, ", ");
        }
    }

    fprintf(stderr, "\n");
    return 1;
}

int RotStr(ExState &State) {
    if (sstack.size() < 3) {
        SetBranchCondition(0);
        return 0;
    }

    std::string tos = sstack[sstack.size() - 1];
    sstack.pop_back();

    SwapStr();
    sstack.push_back(tos);
    SwapStr();

    SetBranchCondition(1);
    return 1;
}

int CompareStr(ExState &State) {
    if (sstack.size() < 2) {
        SetBranchCondition(0);
        return 0;
    }

    int tos = sstack.size() - 1;
    int compareTo;

    std::string what = sstack[tos];
    sstack.pop_back();

    if (State.GetIntParam(0, &compareTo)) {
        compareTo = tos - compareTo;
        ParamStack.push(-(what.compare(sstack[compareTo])));
    } else {
        compareTo = tos - 1;
        ParamStack.push(-(what.compare(sstack[compareTo])));
        sstack.pop_back();
    }

    SetBranchCondition(1);
    return 1;
}




int OverStr() {
    if (sstack.size() < 2) {
        SetBranchCondition(0);
        return 0;
    }

    sstack.push_back(sstack[sstack.size()-2]);
    SetBranchCondition(1);
    return 1;
}

int PickStr(ExState &State) {
    int idx;
    if (State.GetIntParam(0, &idx) == 0)
        idx = ParamStack.pop();
    idx = sstack.size() - 1 - idx;

    if ((unsigned int) idx >= sstack.size()) {
        SetBranchCondition(0);
        return 0;
    }

    sstack.push_back(sstack[idx]);

    SetBranchCondition(1);
    return 1;
}

int DepthStr() {
    ParamStack.push(sstack.size());
    SetBranchCondition(1);
    return 1;
}

int SubSearchStr() {
    int tos = sstack.size() - 1;
    int searchInI  = tos - ParamStack.pop();
    int searchForI = tos - ParamStack.pop();

    if (searchInI < 0 || searchForI < 0) {
        SetBranchCondition(0);
        return 0;
    }

    std::string searchIn  = sstack[searchInI];
    std::string searchFor = sstack[searchForI];

    fprintf(stderr, "Search In: %i (%s), Search For: %i (%s)\n",
            searchInI, searchIn.c_str(),
            searchForI, searchFor.c_str());

    std::string::size_type foundAt = searchIn.find(searchFor, 0);
    ParamStack.push(foundAt == std::string::npos ? -1 : (int) foundAt);

    SetBranchCondition(1);
    return 1;
}

int MergeStr() {
    unsigned int tos = sstack.size();
    if (tos < 2) {
        SetBranchCondition(0);
        return 0;
    }

    tos--; // 0 based index

    std::string tosS = sstack[tos];
    std::string nosS = sstack[tos-1];
    sstack.pop_back();
    sstack.pop_back();

    sstack.push_back(nosS + tosS);

    SetBranchCondition(1);
    return 1;
}

int SplitStr() {
    int tos = sstack.size() - 1;
    if (tos < 0) {
        SetBranchCondition(0);
        return 0;
    }

    unsigned int pos = ParamStack.pop();
    std::string tosS = sstack[tos];

    sstack.pop_back();
    sstack.push_back(tosS.substr(pos));
    sstack.push_back(tosS.substr(0, pos));

    SetBranchCondition(1);
    return 1;
}

int LenStr(ExState &State, GxView *view) {
    EView *View = 0;
    if (view != NULL) {
        ExModelView *V = (ExModelView *)view->Top;
        View = V->View;
    }

    int idx;
    if (State.GetIntParam(View, &idx)) {
        idx = sstack.size() - 1 - idx;
    } else {
        idx = sstack.size() - 1;
    }

    if ((unsigned int) idx >= sstack.size()) {
        SetBranchCondition(0);
        return 0;
    }

    ParamStack.push(sstack[idx].length());

    SetBranchCondition(1);
    return 1;
}

int MidStr(ExState &State, GxView *view) {
    int idx = 0, start = -1, end = -1;
    EView *View = 0;

    if (view != NULL) {
        ExModelView *V = (ExModelView *)view->Top;
        View = V->View;
    }

    if (State.GetIntParam(View, &start)) {
        if (State.GetIntParam(View, &end)) {
            if (State.GetIntParam(View, &idx))
                idx = sstack.size() - 1 - idx;
        } else {
            end = -1;
        }
    } else {
        start = -1;
    }

    idx = sstack.size() - 1 - idx;

    if (end == -1)
        end = ParamStack.pop();
    if (start == -1)
        start = ParamStack.pop();

    if ((unsigned int) idx >= sstack.size()) {
        SetBranchCondition(0);
        return 0;
    }

    std::string tosS = sstack[idx];

    if (start < 0) start = tosS.length() + start;
    if (end < 0) end = tosS.length() + end;

    sstack[idx] = tosS.substr(start, end);

    SetBranchCondition(1);
    return 1;
}

int GetString(ExState &State, GxView *view) {
    EView *View = 0;
    if (view != NULL) {
        ExModelView *V = (ExModelView *)view->Top;
        View = V->View;
    } else {
        SetBranchCondition(0);
        return 0;
    }

    char msg[256] = "", str[256] = "";
    if (State.GetStrParam(View, msg, sizeof(msg)) == 0) {
        strcpy(msg, "String");
    }

    if (View->MView->Win->GetStr(msg, sizeof(str), str, HIST_DEFAULT) == 0) {
        SetBranchCondition(0);
        return 0;
    }

    sstack.push_back(str);

    SetBranchCondition(1);
    return 1;
}

int EGUI::ExecuteCommand(ExState &State, GxView *view)
{
    ExModelView *V = (ExModelView *)view->Top;
    EView *View = V->View;
    char name[64] = "";

    if (State.GetStrParam(View, name, sizeof(name)) == 0) {
        if (View->MView->Win->GetStr("Command name", sizeof(name), name,
                                     HIST_DEFAULT) == 0)
            return 0;
    }

    int command = CmdNum(name);
    if (command == 0) {
        View->Msg(S_INFO, "%s is an unknown command", name);
        SetBranchCondition(0);
        return 0;
    }

    return ExecCommand(view, command, State);
}

int EGUI::ExecCommand(GxView *view, int Command, ExState &State) {
    if (Command & CMD_EXT) {
        return ExecMacro(view, Command & ~CMD_EXT);
    }

    if (Command == ExFail) {
        return ErFAIL;
    }

    // Commands that will run regardless of a View or Buffer
    switch (Command) {
    case ExPrint:
        return Print(view, State);
    case ExPush:
        return Push(view, State);
    case ExPlus:
        return Plus();
    case ExMinus:
        return Minus();
    case ExMul:
        return Mul();
    case ExDiv:
        return Div();

    case ExAnd:
        return And();
    case ExOr:
        return Or();
    case ExXor:
        return Xor();
    case ExShift:
        return Shift();

    case ExEquals:
        return Equals();
    case ExLess:
        return Less();
    case ExFlag:
        return Flag();
    case ExAbort:
        return Abort();

    case ExDup:
        return Dup();
    case ExDrop:
        return Drop();
    case ExSwap:
        return Swap();
    case ExOver:
        return Over();
    case ExRot:
        return Rot();

    case ExToR:
        return ToR();
    case ExRFrom:
        return RFrom();
    case ExRFetch:
        return RFetch();
    case ExI:
        return I();
    case ExJ:
        return J();
    case ExDiag:
        return Diag(State);

    case ExDiagStr:
        return DiagStr(State);
    case ExPushStr:
        return PushStr(view, State);
    case ExDupStr:
        return DupStr();
    case ExDropStr:
        return DropStr();
    case ExSwapStr:
        return SwapStr();
    case ExRotStr:
        return RotStr(State);
    case ExCompareStr:
        return CompareStr(State);
    case ExOverStr:
        return OverStr();
    case ExPickStr:
        return PickStr(State);
    case ExDepthStr:
        return DepthStr();
    case ExSubSearchStr:
        return SubSearchStr();
    case ExSplitStr:
        return SplitStr();
    case ExMergeStr:
        return MergeStr();
    case ExLenStr:
        return LenStr(State, view);
    case ExMidStr:
        return MidStr(State, view);
    case ExGetString:
        return GetString(State, view);
    }

    if (view->IsModelView()) {
        ExModelView *V = (ExModelView *)view->Top;
        EView *View = V->View;

        switch (Command) {
        case ExFileClose:
            return FileClose(View, State);
        case ExFileCloseAll:
            return FileCloseAll(View, State);
        case ExExitEditor:
            return ExitEditor(View);
        case ExIncrementalSearch:
            return View->MView->Win->IncrementalSearch(View);
        }
    }

    switch (Command) {
    case ExExecuteCommand:
        return ExecuteCommand(State, view);
    case ExWinRefresh:
        view->Repaint();
        return 1;
    case ExWinNext:
        return WinNext(view);
    case ExWinPrev:
        return WinPrev(view);
    case ExShowEntryScreen:
        return ShowEntryScreen();
    case ExRunProgram:
        return RunProgram(State, view);
    case ExRunProgramAsync:
        return RunProgramAsync(State, view);
    case ExMainMenu:
        return MainMenu(State, view);
    case ExShowMenu:
        return ShowMenu(State, view);
    case ExLocalMenu:
        return LocalMenu(view);
    case ExFrameNew:
        return FrameNew();
    case ExFrameNext:
        return FrameNext(view);
    case ExFramePrev:
        return FramePrev(view);
    case ExWinHSplit:
        return WinHSplit(view);
    case ExWinClose:
        return WinClose(view);
    case ExWinZoom:
        return WinZoom(view);
    case ExWinResize:
        return WinResize(State, view);
    case ExDesktopSaveAs:
        return DesktopSaveAs(State, view);
    case ExDesktopSave:
        if (DesktopFileName[0] != 0)
            return SaveDesktop(DesktopFileName);
        return 0;
    case ExDesktopLoad:
        return DesktopLoad(State, view);
    case ExChangeKeys:
        {
            char kmaps[64] = "";
            EEventMap *m;

            if (State.GetStrParam(0, kmaps, sizeof(kmaps)) == 0) {
                SetOverrideMap(0, 0);
                return 0;
            }
            m = FindEventMap(kmaps);
            if (m == 0)
                return 0;
            SetOverrideMap(m->KeyMap, m->Name);
            return 1;
        }
    }
    return view->ExecCommand(Command, State);
}

int EGUI::BeginMacro(GxView *view) {
    view->BeginMacro();
    return 1;
}


int EGUI::ExecMacro(GxView *view, const char *name) {
    int num = MacroNum(name);
    if (num == 0) return 1;
    int result = ExecMacro(view, num);
    return result;
}

int EGUI::ExecMacro(GxView *view, int Macro) {
    STARTFUNC("EGUI::ExecMacro");

    int i, j, tos, rtos, rnos, ResultOfCommandExecution;
    ExMacro *m;
    ExState State;
    if (Macro == -1)  {
        ENDFUNCRC(ErFAIL);
    }
    if (BeginMacro(view) == -1) {
        ENDFUNCRC(ErFAIL);
    }
    State.Pos = 0;
    State.Macro = Macro;
    m = &Macros[State.Macro];

    for (i=State.Pos; i < m->Count; i++) {
        if (m->cmds[i].type != CT_COMMAND) {
            continue;
        }
        switch (m->cmds[i].u.num) {

        case ExNop:
            break;


        case ExExit:             // works now as exit: early terminate a macro
            i = m->Count;
            break;


        case ExUnconditionalBranch:
            i += m->cmds[i].repeat;
            break;


        case ExConditionalBranch:
            if (ParamStack.pop() == 0)
                i += m->cmds[i].repeat;
            break;


        case ExDoRuntime:                                           // do loop setup code: initialize loop
            if (ParamStack.peek(0) == ParamStack.peek(1)) {         // limit and start identical? skip loop then:
                ParamStack.pop();                                   //    drop start
                ParamStack.pop();                                   //    drop limit
                i += m->cmds[i].repeat;                             //    proceed at behind LOOP
            } else {                                                // will have to loop:
                tos = ParamStack.pop();                             //    pop start to temp
                ControlStack.push(ParamStack.pop());                //    stack limit
                ControlStack.push(tos);                             //    stack start
            }                                                       //    proceed with loop body
            break;


        case ExLoopRuntime:                                         // executed once per loop iteration:
            tos = ControlStack.pop()+1;                             //    increment loop index
            if (ControlStack.peek(0) == tos)  {                     //    reached limit?
                ControlStack.pop();                                 //    yes: clean up
            } else {                                                //    no:
                ControlStack.push(tos);                             //       keep loop index for next round
                i += m->cmds[i].repeat;                             //       branch back to behind DO
            }
            break;


        case ExPlusLoopRuntime:                                     // executed once per loop iteration:
            rtos = ControlStack.pop();
            rnos = ControlStack.peek(0);
            tos = rtos - rnos;
            rtos += ParamStack.pop();
            if ( (( rtos - rnos ) ^ tos ) > 0 )  {
                ControlStack.push(rtos);                            //       keep loop index for next round
                i += m->cmds[i].repeat;                             //       branch back to behind DO
            } else {                                                //    no:
                ControlStack.pop();                                 //    yes: clean up
            }
            break;


        case ExLeaveRuntime:
            ControlStack.pop();
            ControlStack.pop();
            i += m->cmds[i].repeat;
            i += m->cmds[i].repeat;
            break;


        case ExTimes:
            tos = ParamStack.pop();
            if (tos) {
                m->cmds[i+1].repeat = tos;                         // maybe (test whether needed): rpt=0? skip next.
            } else {
                i += 1;
            }
            break;                                                 // would reintroduce conditional skip with  0/1 times command


        default:
            State.Pos = i+1;
            for (j=(m->cmds[i].repeat); j; --j) {
                ResultOfCommandExecution=ExecCommand(view, m->cmds[i].u.num, State);
                if (!(ResultOfCommandExecution || m->cmds[i].ign))
                    return ErFAIL;
            }
        }
        State.Pos=i;
    }
    ENDFUNCRC(ErOK);
}




void EGUI::SetMsg(char *Msg) {
    char CharMap[128] = "";

    if (Msg == 0) {
        strcpy(CharMap, "");
    } else {
        strcat(CharMap, "[");
        strcat(CharMap, Msg);
        strcat(CharMap, "]");
    }
    if (ActiveModel)
        ActiveModel->Msg(S_INFO, CharMap);
}

void EGUI::SetOverrideMap(EKeyMap *aMap, char *ModeName) {
    OverrideMap = aMap;
    if (aMap == 0)
        SetMsg(0);
    else
        SetMsg(ModeName);
}

void EGUI::SetMap(EKeyMap *aMap, KeySel *ks) {
    char key[32] = "";

    ActiveMap = aMap;
    if (ActiveMap == 0) {
        SetMsg(0);
    } else {
        if (ks != 0) {
            GetKeyName(key, sizeof(key), *ks);
            SetMsg(key);
        }
    }
}

void EGUI::DispatchKey(GxView *view, TEvent &Event) {
    EEventMap *EventMap;
    EKeyMap *map;
    EKey *key = 0;
    char Ch;

    if (Event.Key.Code & kfModifier)
        return;

    LastEventChar = -1;
    if (GetCharFromEvent(Event, &Ch))
        LastEventChar = Ch;

    if ((EventMap = view->GetEventMap()) == 0)
        return;

    map = EventMap->KeyMap;

    if (ActiveMap || OverrideMap) {
        map = ActiveMap;
        if (OverrideMap)
            map = OverrideMap;
        while (map) {
            if ((key = map->FindKey(Event.Key.Code)) != 0) {
                if (key->fKeyMap) {
                    SetMap(key->fKeyMap, &key->fKey);
                    Event.What = evNone;
                    return ;
                } else {
                    SetMap(0, &key->fKey);
                    ExecMacro(view, key->Cmd);
                    Event.What = evNone;
                    return ;
                }
            }
            map = map->fParent;
        }
        if (!OverrideMap) {
            SetMap(0, 0);
            Event.What = evNone;
        }
        return ;
    }
    while (EventMap) {
        if (map) {
            if ((key = map->FindKey(Event.Key.Code)) != 0) {
                if (key->fKeyMap) {
                    SetMap(key->fKeyMap, &key->fKey);
                    Event.What = evNone;
                    return ;
                } else {
                    ExecMacro(view, key->Cmd);
                    Event.What = evNone;
                    return ;
                }
            }
        }
        EventMap = EventMap->Parent;
        if (EventMap == 0) break;
        map = EventMap->KeyMap;
    }
    //if (GetCharFromEvent(Event, &Ch))
    //    CharEvent(view, Event, Ch);
    SetMap(0, 0);
}

void EGUI::DispatchCommand(GxView *view, TEvent &Event) {
    if (Event.Msg.Command > 65536 + 16384) { // hack for PM toolbar
        Event.Msg.Command -= 65536 + 16384;
        BeginMacro(view);
        ExState State;
        State.Macro = 0;
        State.Pos = 0;
        ExecCommand(view, Event.Msg.Command, State);
        Event.What = evNone;
    } else if (Event.Msg.Command >= 65536) {
        Event.Msg.Command -= 65536;
        ExecMacro(view, Event.Msg.Command);
        Event.What = evNone;
    }
}

void EGUI::DispatchEvent(GFrame *frame, GView *view, TEvent &Event) {
    GxView *xview = (GxView *) view;

    if (Event.What == evNone ||
            (Event.What == evMouseMove && Event.Mouse.Buttons == 0))
        return ;

    if (Event.What == evNotify && Event.Msg.Command == cmPipeRead) {
        Event.Msg.Model->NotifyPipe(Event.Msg.Param1);
        return;
    }
    if (xview->GetEventMap() != 0) {
        switch (Event.What) {
        case evKeyDown:
            DispatchKey(xview, Event);
            break;
        case evCommand:
            if (Event.Msg.Command >= 65536) {
                DispatchCommand(xview, Event);
            } else {
                switch (Event.Msg.Command) {
                case cmClose: {
                    assert(ActiveView != 0);
                    FrameClose(ActiveView->MView->Win);
                    return;
                }
                }
            }
        }
    }
    GUI::DispatchEvent(frame, view, Event);
#if defined(OS2) && !defined(DBMALLOC) && defined(CHECKHEAP)
    if (_heapchk() != _HEAPOK)
        DieError(0, "Heap memory is corrupt.");
#endif
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

int EGUI::WinNext(GxView *view) {
    view->Parent->SelectNext(0);
    return 1;
}

int EGUI::WinPrev(GxView *view) {
    view->Parent->SelectNext(1);
    return 1;
}

int EGUI::FileCloseX(EView *View, int CreateNew, int XClose) {
    char Path[MAXPATH];

    // this should never fail!
    if (GetDefaultDirectory(View->Model, Path, sizeof(Path)) == 0)
        return 0;

    if (View->Model->ConfQuit(View->MView->Win)) {

        View->Model->DeleteRelated();

        // close everything that can be closed without confirmation if closing all
        if (XClose)
            while (View->Model->Next != View->Model &&
                    View->Model->Next->CanQuit())
                delete View->Model->Next;

        View->DeleteModel(View->Model);

        if (ActiveModel == 0 && CreateNew) {
            EView *V = ActiveView;
            EModel *m = new EDirectory(0, &ActiveModel, Path);
            if (m == 0) {
                View->MView->Win->Choice(GPC_ERROR, "Error", 1, "O&K", "Could not create directory view");
                return 0;
            }

            do {
                V = V->Next;
                V->SelectModel(ActiveModel);
            } while (V != ActiveView);
            return 0;
        }

        if (ActiveModel == 0) {
            StopLoop();
        }
        return 1;
    }
    return 0;
}


int EGUI::FileClose(EView *View, ExState &State) {
    int x = 0;

    if (State.GetIntParam(View, &x) == 0)
        x = OpenAfterClose;

    return FileCloseX(View, x);
}

int EGUI::FileCloseAll(EView *View, ExState &State) {
    int x = 0;

    if (State.GetIntParam(View, &x) == 0)
        x = OpenAfterClose;

    while (ActiveModel)
        if (FileCloseX(View, x, 1) == 0) return 0;
    return 1;
}

int EGUI::WinHSplit(GxView *View) {
    GxView *view;
    ExModelView *edit;
    EView *win;
    int W, H;

    View->ConQuerySize(&W, &H);

    if (H < 8)
        return 0;
    view = new GxView(View->Parent);
    if (view == 0)
        return 0;
    win = new EView(ActiveModel);
    if (win == 0)
        return 0;
    edit = new ExModelView(win);
    if (edit == 0)
        return 0;
    view->PushView(edit);
    view->Parent->SelectNext(0);
    return 1;
}

int EGUI::WinClose(GxView * /*V*/) {
    EView *View = ActiveView;

    if (View->Next == View) {
        // when closing last window, close all files
        if (ExitEditor(View) == 0)
            return 0;
    } else {
        View->MView->Win->Parent->SelectNext(0);
        delete View->MView->Win;
    }
    return 1;
}

int EGUI::WinZoom(GxView *View) {
    GView *V = View->Next;
    GView *V1;

    while (V) {
        V1 = V;
        if (V == View)
            break;
        V = V->Next;
        delete V1;
    }
    return 1;
}

int EGUI::WinResize(ExState &State, GxView *View) {
    int Delta = 1;

    if (State.GetIntParam(0, &Delta)) {
        if (View->ExpandHeight(Delta) == 0)
            return 1;
    }
    return 0;
}

int EGUI::ExitEditor(EView *View) {
    EModel *B = ActiveModel;

    // check/save modified files
    while (ActiveModel) {
        if (ActiveModel->CanQuit()) ;
        else {
            View->SelectModel(ActiveModel);
            int rc = ActiveModel->ConfQuit(View->MView->Win, 1);
            if (rc == -2) {
                View->FileSaveAll();
                break;
            }
            if (rc == 0)
                return 0;
        }

        ActiveModel = ActiveModel->Next;
        if (ActiveModel == B)
            break;
    }

    if (SaveDesktopOnExit && DesktopFileName[0] != 0)
        SaveDesktop(DesktopFileName);
    else if (LoadDesktopMode == 2) {       // Ask about saving?
        GxView* gx = View->MView->Win;

        if (gx->GetStr("Save desktop As",
                       sizeof(DesktopFileName), DesktopFileName,
                       HIST_DEFAULT) != 0) {
            SaveDesktop(DesktopFileName);
        }
    }

    while (ActiveModel) {
        if (View->Model->GetContext() == CONTEXT_ROUTINES) { // Never delete Routine models directly
            ActiveModel = ActiveModel->Next;
            View->SelectModel(ActiveModel);
        }

        View->Model->DeleteRelated();  // delete related views first

        while (View->Model->Next != View->Model &&
                View->Model->Next->CanQuit())
            delete View->Model->Next;

        View->DeleteModel(View->Model);
    }

    StopLoop();
    return 1;
}

int EGUI::ShowEntryScreen() {
    return gui->ShowEntryScreen();
}

int EGUI::RunProgram(ExState &State, GxView *view) {
    static char Cmd[512] = "";

    if (ActiveModel)
        SetDefaultDirectory(ActiveModel);

    if (State.GetStrParam(ActiveView, Cmd, sizeof(Cmd)) == 0)
        if (view->GetStr("Run", sizeof(Cmd), Cmd, HIST_COMPILE) == 0) return 0;
    gui->RunProgram(RUN_WAIT, Cmd);
    return 1;
}

int EGUI::RunProgramAsync(ExState &State, GxView *view) {
    static char Cmd[512] = "";

    if (ActiveModel)
        SetDefaultDirectory(ActiveModel);

    if (State.GetStrParam(ActiveView, Cmd, sizeof(Cmd)) == 0)
        if (view->GetStr("Run", sizeof(Cmd), Cmd, HIST_COMPILE) == 0) return 0;
    gui->RunProgram(RUN_ASYNC, Cmd);
    return 1;
}

int EGUI::MainMenu(ExState &State, GxView *View) {
    char s[3];

    if (State.GetStrParam(0, s, sizeof(s)) == 0)
        s[0] = 0;

    View->Parent->ExecMainMenu(s[0]);
    return 1;
}

int EGUI::ShowMenu(ExState &State, GxView *View) {
    char MName[32] = "";

    if (State.GetStrParam(0, MName, sizeof(MName)) == 0)
        return 0;

    View->Parent->PopupMenu(MName);
    return 0;
}

int EGUI::LocalMenu(GxView *View) {
    EEventMap *Map = View->GetEventMap();
    const char *MName = 0;

    if (Map)
        MName = Map->GetMenu(EM_LocalMenu);
    if (MName == 0)
        MName = "Local";
    View->Parent->PopupMenu(MName);
    return 0;
}

int EGUI::DesktopSaveAs(ExState &State, GxView *view) {
    if (State.GetStrParam(0, DesktopFileName, sizeof(DesktopFileName)) == 0)
        if (view->GetFile("Save Desktop", sizeof(DesktopFileName), DesktopFileName, HIST_PATH, GF_SAVEAS) == 0)
            return 0;

    if (DesktopFileName[0] != 0)
        return SaveDesktop(DesktopFileName);
    return 0;
}

int EGUI::DesktopLoad(ExState &State, GxView *view) {
    if (State.GetStrParam(0, DesktopFileName, sizeof(DesktopFileName)) == 0)
        if (view->GetFile("Load Desktop", sizeof(DesktopFileName), DesktopFileName, HIST_PATH, GF_OPEN) == 0)
            return 0;

    if (DesktopFileName[0] != 0)
        return LoadDesktop(DesktopFileName);
    return 0;
}

int EGUI::FrameNew() {
    GxView *view;
    ExModelView *edit;

    if (!multiFrame() && frames)
        return 0;

    (void) new EFrame(ScreenSizeX, ScreenSizeY);
    assert(frames != 0);

    //frames->SetMenu("Main"); //??

    view = new GxView(frames);
    assert(view != 0);

    (void)new EView(ActiveModel);
    assert(ActiveView != 0);

    edit = new ExModelView(ActiveView);
    assert(edit != 0);
    view->PushView(edit);
    frames->Show();
    return 1;
}

int EGUI::FrameClose(GxView *View) {
    assert(frames != 0);
    assert(View != 0);

    if (!frames->isLastFrame()) {
        deleteFrame(frames);
    } else {
        if (ExitEditor(ActiveView) == 0)
            return 0;
        deleteFrame(frames);
    }
    return 1;
}

int EGUI::FrameNext(GxView * /*View*/) {
    if (!frames->isLastFrame()) {
        frames->Next->Activate();
        return 1;
    }
    return 0;
}

int EGUI::FramePrev(GxView * /*View*/) {
    if (!frames->isLastFrame()) {
        frames->Prev->Activate();
        return 1;
    }
    return 0;
}

int EGUI::findDesktop(char *argv[]) {
    /*
     *  Locates the desktop file depending on the load desktop mode flag:
     *  0:  Try the "current" directory, then the FTE .exe directory (PC) or
     *      the user's homedir (Unix). Fail if not found (original FTE
     *      algorithm).
     *  1:  Try the current directory, then loop {go one directory UP and try}
     *      If not found, don't load a desktop!
     *  2:  As above, but asks to save the desktop if one was not found.
     *  This is called if the desktop is not spec'd on the command line.
     */
    switch (LoadDesktopMode) {
    default:
        //** 0: try curdir then "homedir"..
        //         fprintf(stderr, "ld: Mode 0\n");
        if (FileExists(DESKTOP_NAME))
            ExpandPath(DESKTOP_NAME, DesktopFileName, sizeof(DesktopFileName));
        else {
            //** Use homedir,
#ifdef UNIX
            ExpandPath("~/" DESKTOP_NAME, DesktopFileName, sizeof(DesktopFileName));
#else
            JustDirectory(argv[0], DesktopFileName, sizeof(DesktopFileName));
            strlcat(DesktopFileName, DESKTOP_NAME, sizeof(DesktopFileName));
#endif // UNIX
        }
        return FileExists(DesktopFileName);

    case 1:
    case 2:
        //** Try curdir, then it's owner(s)..
        ExpandPath(".", DesktopFileName, sizeof(DesktopFileName));
        //fprintf(stderr, "ld: Mode 1 (start at %s)\n", DesktopFileName);

        for (;;) {
            //** Try current location,
            char *pe = DesktopFileName + strlen(DesktopFileName);
            Slash(DesktopFileName, 1);      // Add appropriate slash
            strlcat(DesktopFileName, DESKTOP_NAME, sizeof(DesktopFileName));

            //fprintf(stderr, "ld: Mode 1 (trying %s)\n", DesktopFileName);
            if (FileExists(DesktopFileName)) {
                //fprintf(stderr, "ld: Mode 1 (using %s)\n", DesktopFileName);
                return 1;
            }

            //** Not found. Remove added stuff, then go level UP,
            *pe = 0;

            // Remove the current part,
            char *p = SepRChr(DesktopFileName);

            if (p == NULL) {
                //** No desktop! Set default name in current directory,
                ExpandPath(".", DesktopFileName, sizeof(DesktopFileName));
                Slash(DesktopFileName, 1);
                strlcat(DesktopFileName, DESKTOP_NAME, sizeof(DesktopFileName));

                SaveDesktopOnExit = 0;      // Don't save,
                return 0;                   // NOT found!!
            }
            *p = 0;                         // Truncate name at last
        }
    }
}

void EGUI::DoLoadDesktopOnEntry(int &/*argc*/, char **argv) {
    if (DesktopFileName[0] == 0)
        findDesktop(argv);

    if (DesktopFileName[0] != 0) {
        if (IsDirectory(DesktopFileName)) {
            Slash(DesktopFileName, 1);
            strlcat(DesktopFileName, DESKTOP_NAME, sizeof(DesktopFileName));
        }

        if (LoadDesktopOnEntry && FileExists(DesktopFileName))
            LoadDesktop(DesktopFileName);
    }
}

void EGUI::EditorInit() {
    SSBuffer = new EBuffer(0, (EModel **)&SSBuffer, "Scrap");
    assert(SSBuffer != 0);
    BFI(SSBuffer, BFI_Undo) = 0; // disable undo for clipboard
    ActiveModel = 0;
}

int EGUI::InterfaceInit(int &/*argc*/, char ** /*argv*/) {
    if (FrameNew() == 0)
        DieError(1, "Failed to create window\n");
    return 0;
}

void EGUI::DoLoadHistoryOnEntry(int &/*argc*/, char **argv) {
    if (HistoryFileName[0] == 0) {
#ifdef UNIX
        ExpandPath("~/.efte-history", HistoryFileName, sizeof(HistoryFileName));
#else
        JustDirectory(argv[0], HistoryFileName, sizeof(HistoryFileName));
        strlcat(HistoryFileName, "efte.his", sizeof(HistoryFileName));
#endif // UNIX
    } else {
        char p[256];

        ExpandPath(HistoryFileName, p, sizeof(p));
        if (IsDirectory(p)) {
            Slash(p, 1);
            strlcat(p, HISTORY_NAME, sizeof(p));
        }
        strlcpy(HistoryFileName, p, sizeof(HistoryFileName));
    }

    if (KeepHistory && FileExists(HistoryFileName))
        LoadHistory(HistoryFileName);
}

void EGUI::DoSaveHistoryOnExit() {
    if (KeepHistory && HistoryFileName[0] != 0)
        SaveHistory(HistoryFileName);

    // since we are exiting, free history
    ClearHistory();
}

int EGUI::CmdLoadFiles(int &argc, char **argv) {
    int QuoteNext = 0;
    int QuoteAll = 0;
    int GotoLine = 0;
    int LineNum = 1;
    int ColNum = 1;
    int ModeOverride = 0;
    char Mode[32];
    int LCount = 0;
    int ReadOnly = 0;

    for (int Arg = 1; Arg < argc; Arg++) {
        if (!QuoteAll && !QuoteNext && (argv[Arg][0] == '-')) {
            switch (argv[Arg][1]) {
            case '-':
                if (strncmp(argv[Arg], "--debug", 7) != 0)
                    QuoteAll = 1;
                Arg = argc;
                break;
            case '!':
            case 'C':
            case 'c':
            case 'D':
            case 'd':
            case 'e':
            case 'H':
                // handled before
                break;
            case '+':
                QuoteNext = 1;
                break;
            case '#':
            case 'l':
                LineNum = 1;
                ColNum = 1;
                if (strchr(argv[Arg], ',')) {
                    GotoLine = (2 == sscanf(argv[Arg] + 2, "%d,%d", &LineNum, &ColNum));
                } else {
                    GotoLine = (1 == sscanf(argv[Arg] + 2, "%d", &LineNum));
                }
                // printf("Gotoline = %d, line = %d, col = %d\n", GotoLine, LineNum, ColNum);
                break;
            case 'r':
                ReadOnly = 1;
                break;
            case 'm':
                if (argv[Arg][2] == 0) {
                    ModeOverride = 0;
                } else {
                    ModeOverride = 1;
                    strcpy(Mode, argv[Arg] + 2);
                }
                break;
            case 'T':
                TagsAdd(argv[Arg] + 2);
                break;
            case 't':
                TagGoto(ActiveView, argv[Arg] + 2);
                break;
            case 'v': // Verbosity, handled in fte.cpp
                break;
            default:
                DieError(2, "Invalid command line option %s", argv[Arg]);
                return 0;
            }
        } else {
            char Path[MAXPATH];

            QuoteNext = 0;
            if (ExpandPath(argv[Arg], Path, sizeof(Path)) == 0 && IsDirectory(Path)) {
                EModel *m = new EDirectory(cfAppend, &ActiveModel, Path);
                if (m == 0 || ActiveModel == 0) {
                    DieError(2, "Could not open a directory view of path: %s\n", Path);
                }
            } else {
                if (LCount != 0)
                    suspendLoads = 1;
                if (MultiFileLoad(cfAppend, argv[Arg],
                                  ModeOverride ? Mode : 0,
                                  ActiveView) == 0) {
                    suspendLoads = 0;
                    return 0;
                }
                suspendLoads = 0;

                if (GotoLine) {
                    if (((EBuffer *)ActiveModel)->Loaded == 0)
                        ((EBuffer *)ActiveModel)->Load();
                    if (GotoLine) {
                        GotoLine = 0;
                        ((EBuffer *)ActiveModel)->SetNearPosR(ColNum - 1, LineNum - 1);
                    } else {
                        int r, c;

                        if (RetrieveFPos(((EBuffer *)ActiveModel)->FileName, r, c) == 1)
                            ((EBuffer *)ActiveModel)->SetNearPosR(c, r);
                    }
                    //ActiveView->SelectModel(ActiveModel);
                }
                if (ReadOnly) {
                    ReadOnly = 0;
                    BFI(((EBuffer *)ActiveModel), BFI_ReadOnly) = 1;
                }
            }
            suspendLoads = 1;
            ActiveView->SelectModel(ActiveModel->Next);
            suspendLoads = 0;
            LCount++;
        }
    }
    EModel *P = ActiveModel;
    while (LCount-- > 0)
        P = P->Prev;
    ActiveView->SelectModel(P);
    return 1;
}

int EGUI::Start(int &argc, char **argv) {
    {
        int rc;

        rc = GUI::Start(argc, argv);

        if (rc)
            return rc;
    }

    if (InterfaceInit(argc, argv) != 0)
        return 2;

    EditorInit();

    DoLoadHistoryOnEntry(argc, argv);
    DoLoadDesktopOnEntry(argc, argv);

    if (CmdLoadFiles(argc, argv) == 0)
        return 3;

    if (ActiveModel == 0) {
        char Path[MAXPATH];

        GetDefaultDirectory(0, Path, sizeof(Path));
        EModel *m = new EDirectory(0, &ActiveModel, Path);
        if (m == 0 || ActiveModel == 0) {
            DieError(2, "Could not open a directory view of path: %s\n", Path);
        }

        ActiveView->SwitchToModel(ActiveModel);
    }
    return 0;
}

void EGUI::EditorCleanup() {
    if (ActiveModel != NULL) {
        EModel *B, *N, *A;

        B = A = ActiveModel;
        do {
            N = B->Next;
            delete B;
            B = N;
        } while (B != A);
    }
    ActiveModel = NULL;

    delete SSBuffer;
    SSBuffer = NULL;

    if (ActiveView != NULL) {
        EView *BW, *NW;

        // If EView what is about to be deleted is currently ActiveView, ActiveView moves to next one
        // or if there is no next, it will be set as NULL.
        while ((BW = ActiveView) != NULL) {
            NW = BW->Next;
            delete BW;
        }
        //EView *BW, *NW, *AW;
        //BW = AW = ActiveView;
        //do {
        //    NW = BW->Next;
        //    delete BW;
        //    BW = NW;
        //} while (BW != AW);
    }
    ActiveView = NULL;
}

void EGUI::InterfaceCleanup() {
    while (frames)
        delete frames;
}

void EGUI::Stop() {
    DoSaveHistoryOnExit();

    // free macros
    if (Macros != 0) {
        while (CMacros--) {
            free(Macros[CMacros].Name);

            for (int i = 0; i < Macros[CMacros].Count; ++i)
                if (Macros[CMacros].cmds[i].type == CT_STRING)
                    free(Macros[CMacros].cmds[i].u.string);

            free(Macros[CMacros].cmds);
        }

        free(Macros);

        Macros = 0;
    }

    // free colorizers
    while (EColorize *p = Colorizers) {
        Colorizers = Colorizers->Next;
        delete p;
    }

    // free event maps
    while (EEventMap *em = EventMaps) {
        EventMaps = EventMaps->Next;
        delete em;
    }

    // free modes
    while (EMode *m = Modes) {
        Modes = Modes->fNext;
        delete m;
    }

    // free menus
    if (Menus) {
        int mc, c;

        while (MenuCount--) {
            mc = MenuCount;

            free(Menus[mc].Name);

            while (Menus[mc].Count--) {
                c = Menus[mc].Count;
                free(Menus[mc].Items[c].Name);
            }

            free(Menus[MenuCount].Items);
        }

        free(Menus);

        Menus = NULL;
    }

    // free completion rexexp filter
    extern RxNode *CompletionFilter;
    RxFree(CompletionFilter);

    // free CRegexp array from o_messages.cpp
    FreeCRegexp();
    // free CvsIgnoreRegexp array from o_messages.cpp
    FreeCvsIgnoreRegexp();
    // free SvnIgnoreRegexp array from o_messages.cpp
    FreeSvnIgnoreRegexp();

    // free configuration file path
    free(ConfigSourcePath);
    ConfigSourcePath = NULL;

    EditorCleanup();

    InterfaceCleanup();

    GUI::Stop();
}
