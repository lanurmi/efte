/*    egui.cpp
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include <vector>

#include "fte.h"
#include "log.h"
#include "throw.h"
#include <sys/time.h>
#include <time.h>

int LastEventChar = -1;
int exception = 0;

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

        if ( !(OMenu) || strcmp(OMenu, NMenu))
            SetMenu(NMenu);
    }
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

// --- virtual machine ---
/*       for a full blown user macros system, a bit of support here will greatly speed up things.
          this implements a virtual machine for user code and data execution (yes, data executes):
          *** this is how our macro executor had looked like, had it been implemented using a
          technique to transfer control between commands using "threaded code". it would have eliminated
          the need for decoding all the special types of things to execute, and made the long
          lists of case statements, to find out what command needs to be executed, unnecessary.

          the macro executur loop itself would have been something like:
          for (ip = macro; ; next())

          or, to speed up a bit,
          for (ip = macro; ; next())
          { next(); next(); next(); next(); next(); next(); next(); next(); }
          ( unroll-loops compiler optimization option could have a similar effect )


 unsigned int ip, w;

 next {
    w = memory[ip++];
    ExecCommand[w++];
}

 nest()  {
    ControlStack.push(ip);
    ip = w;
    next();
}

 unnest() {
    ip = ControlStack.pop();
    next();
 }

 dodata() {
    ParamStack.push(w);
    next();
}
*/

struct timeval tv;

int Now()  {         // wraps every 136y 144d 6h
    gettimeofday(&tv, NULL);
    ParamStack.push(tv.tv_sec);
    return 1;
}

int Milliseconds()  {         // wraps every 49d 17h 2m 47s
    gettimeofday(&tv, NULL);
    ParamStack.push(tv.tv_sec * 1000 + tv.tv_usec/1000);
    return 1;
}

int Microseconds()  {         // wraps every 1h 11m 34s
    gettimeofday(&tv, NULL);
    ParamStack.push((tv.tv_sec * 1000000 + tv.tv_usec));
    return 1;
}




// sleep for <tos> milliseconds
int Ms()  {
//    view->Repaint();
    int tos=ParamStack.pop();
    int nos=tos % 1000;
    tos /= 1000;
    if (tos)
        sleep(tos);
//    if (nos)
//        nanosleep(nos*1000000);
    return 1;
}



// moved these here because i couldn't get around multiple
// definitions of xrayindent, when moving it to w_misc.h, to
// be able to read it here, in spite of a #idndef supposedly
// branching around it.
#define XRAYINDENT 3
unsigned int xrayindent = 0;
void Dodent()  {
    fprintf(stderr, "\n");
    int i = xrayindent;
    for ( ; i>1; i--) {
        fprintf(stderr, "|");
        int j = XRAYINDENT-1;
        for ( ;j ;j--)
            fprintf(stderr, " ");

    }
}

void Redent(int change)  { xrayindent += change; }
void Nodent()            { xrayindent=0; }
void Indent()            { Redent(1);  }
void Undent()            { Redent(-1); }



// -----------------------------------------------------------------------------------------------------------

int EGUI::ExecCommand(GxView *view, int Command, ExState &State) {
    // somehow stack display is behind the wrong command.
    // more intuitive would be:
    //     command        stack after command has been executed
    // but it is:
    //     command        stack before command execution
    // i'll try to split this, and keep only execution trace here for now,
    // looking for a place where to stick in stack dump after command has executed.
    // what i don't quite manage to do is to align stack display, after command name has been printed.

    if (memory[verbosity] > xrayindent) {
        Dodent();
        fprintf(stderr, "%-15s ", GetCommandName(Command));
    }

    if (Command & CMD_EXT) {
        return ExecMacro(view, Command & ~CMD_EXT);
    }

    // Commands that will run regardless of a View or Buffer
    switch (Command) {
    case ExDepth:
        return ParamDepth();
    case ExStore:
        return MemoryStore();
    case ExFetch:
        return MemoryFetch();
    case ExStore2:
        return MemoryStore2();
    case ExFetch2:
        return MemoryFetch2();
    case ExMemEnd:
        return MemoryEnd();
    case ExHere:
        return MemoryHere();
    case ExAllot:
        return MemoryAllot();

        // Shared Variables
    case ExVerbosity:
        return Verbosity();
    case ExBase:
        return Base();
    case ExAutoTrim:
        return AutoTrim();
    case ExInsert:
        return Insert();
    case ExStatusline:
        return Statusline();
    case ExMouse:
        return Mouse();

        // command calls
    case ExTick:
        return Tick();
    case ExExecute:
        return Execute(State, view);

        // Arithmetics
    case ExPlus:
        return Plus();
    case ExMinus:
        return Minus();
    case ExMul:
        return Mul();
    case ExDiv:
        return Div();
    case ExStarSlash:
        return StarSlash();
    case ExInvert:
        return Invert();
    case ExNegate:
        return Negate();
    case ExNot:
        return Not();
    case ExNotZero:
        return NotZero();
    case ExPlusStore:
        return PlusStore();
    case ExBetween:
        return Between();
    case ExSlashMod:
        return SlashMod();
    case ExEquals2:
        return Equals2();
    case ExMinSigned:
        return MinSigned();
    case ExMaxSigned:
        return MaxSigned();
    case ExMinUnsigned:
        return MinUnsigned();
    case ExMaxUnsigned:
        return MaxUnsigned();

    case ExRandom:
        return Random();
    case ExNow:
        return Now();
    case ExMilliseconds:
        return Milliseconds();
    case ExMicroseconds:
        return Microseconds();
    case ExMs:
        return Ms();


        // Bool
    case ExAnd:
        return And();
    case ExOr:
        return Or();
    case ExXor:
        return Xor();
    case ExShift:
        return Shift();

        // Compare
    case ExEquals:
        return Equals();
    case ExLess:
        return Less();
    case ExMore:
        return More();
    case ExFlag:
        return Flag();
    case ExFail:
        return Fail();

        // Stack
    case ExDup:
        return Dup();
    case ExQDup:
        return QDup();
    case ExDrop:
        return Drop();
    case ExSwap:
        return Swap();
    case ExSwap2:
        return Swap2();
    case ExOver:
        return Over();
    case ExRot:
        return Rot();
    case ExMinRot:
        return MinRot();
    case ExPick:
        return Pick();

        // Return stack
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

        // Diagnostics
    case ExPrint:
        return Print(view, State);
    case ExDiag:
        return Diag(State);
    case ExDiagStr:
        return DiagStr();

        // String stack
    case ExDupStr:
        return DupStr();
    case ExDropStr:
        return DropStr();
    case ExSwapStr:
        return SwapStr();
    case ExRotStr:
        return RotStr();
    case ExCompareStr:
        return CompareStr();
    case ExOverStr:
        return OverStr();
    case ExPickStr:
        return PickStr();
    case ExDepthStr:
        return DepthStr();
    case ExSubSearchStr:
        return SubSearchStr();
    case ExAppendStr:
        return AppendStr();
    case ExLenStr:
        return LenStr();
    case ExMidStr:
        return MidStr();
    case ExGetString:
        return GetString(view);
    case ExAsc:
        return SSAsc();
    case ExChar:
        return PSChar();
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

            SSCHECK(1, "ChangeKeys");

            strcpy(kmaps, sstack.back().c_str()); sstack.pop_back();

            if (strlen(kmaps) == 0) {
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
    return ExecMacro(view, num);
}

unsigned int doesindex = 0;
int faillevel = 0;

int EGUI::ExecMacro(GxView *view, int Macro) {
    Indent();
    STARTFUNC("EGUI::ExecMacro");

    int i, j, tos, nos, rtos, rnos, ResultOfCommandExecution;
    unsigned int unos, utos;
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
        switch (m->cmds[i].type) {
        case CT_NUMBER:
            ParamStack.push(m->cmds[i].u.num);
            continue;

        case CT_STRING:
            sstack.push_back(m->cmds[i].u.string);
            continue;
        }
        unsigned int macroclass;
        switch (m->cmds[i].u.num) {

        case ExNop:
            break;


        case ExExit:                                                // works now as exit: early terminate a macro
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


        case ExPlusLoopRuntime:                                     // Executed once per loop iteration:
            rtos = ControlStack.pop();                              // as loop increment can be either positive or negative, the
            rnos = ControlStack.peek(0);                            // method to determine when index has passed limit is a bit
            tos = rtos - rnos;                                      // complicated. simple =, < or > won't do. we need something like
            rtos += ParamStack.pop();                               // "if it was < before adding, is it > now?" and "if it was >
            if ( (( rtos - rnos ) ^ tos ) > 0 )  {                  // before, is it < now?" done by xoring diff before and after adding loop increment.
                                                                    // result will be negative when limit has been passed.
                ControlStack.push(rtos);                            //       keep loop index for next round
                i += m->cmds[i].repeat;                             //       branch back to behind DO
            } else {
                ControlStack.pop();                                 // loop limit passed: clean up
            }
            break;


        case ExMinLoopRuntime:                                      // Executed once per loop iteration:
            rtos = ControlStack.pop();                              // almost identical to ExPlusLoopRuntime. Read up there.
            rnos = ControlStack.peek(0)+1;                          // This one exists for better symmetry with plusloop for
            tos = rtos - rnos;                                      // count down loops: I think the standard has messed up there.
            rtos -= ParamStack.pop();                               // Providing MinLoop next to PlusLoop seems the only way
            if ( (( rtos - rnos ) ^ tos ) > 0 )  {                  // to provide symmetricity without violating the standard
                ControlStack.push(rtos);                            // (by changing plusloop to behave symmetrically). Thus, the
                i += m->cmds[i].repeat;                             // deal is: for symmetric up/down loops, don't use negative
            } else {                                                // increment with plusloop. Use positive increment with minloop
                ControlStack.pop();                                 // instead. For standard-conform down loops, use (standard-
            }                                                       // compliant) plusloop with negative increment.
            break;


        case ExLeaveRuntime:
            ControlStack.pop();
            ControlStack.pop();
            i += m->cmds[i].repeat;
            i += m->cmds[i].repeat;
            break;


        case ExVectorRuntime:
            utos = ParamStack.pop();
            nos = m->cmds[i++].repeat;
            if (nos < utos)
                utos = nos;
            ExecCommand(view, m->cmds[i+utos].u.num, State);
            i += nos;
            break;


        case ExTimes:
            tos = ParamStack.pop();
            i++;
            if (tos)
                m->cmds[i--].repeat = tos;
            break;                                                  // would reintroduce conditional skip with  0/1 times command


        case ExWill:
            tos = ParamStack.pop();
            if (tos == 0)
                i++;
            break;


        case ExUnless:
            tos = ParamStack.pop();
            if (tos)
                i = m->Count;
            break;

        case ExLest:
            tos = ParamStack.pop();
            if (!tos)
                i = m->Count;
            break;


// ------------------------------------------------------------------------------------------------------

            // the next three, ExOld, ExNew and ExDoes, implement a very cheeky data structure applicator.
            // this is quite wild, relying on self-modifying macros, but it's the best extendable
            // scheme conceived as far. its use, in terms of "syntax", is even quite satisfactory.
            // these are made to mimick the Forth  "create ... does> ..." construct. Given the limitations
            // of the underlying macro executor, these are only a crude approximation. Yet. :D

            // Terminology in comments starts to get confusing, therefore definition of new terms here.
            // Borrowing from object orientation, in the hope that this won't confuse matters more than necessary.
            // But in fact, there are some similarities to OO. But we can't subclass, have no inheritance
            // and no polymorphism. There is some concept of encapsulation, but not forced, and we don't
            // require methods to access instance data, as instances share the address to their instance data freely.

            // when we say:        we mean:                                            example:
            // class               the macro, describing the data structure            sub var { here cell allot }
            // instance            an object, based on class                           sub foo { var new }
            // instantiation       first time execution of instance, finalizing it     { ... 123 foo store }
            // method              run time code in class, after does                  sub const { here cell allot
            //                     like an implied method                                          does fetch }

        case ExOld:
            ParamStack.push(m->cmds[0].repeat);                     // push address of instance data
            i = m->cmds[1].repeat;                                  // yes: continue execution at class/index
            if (i) {                                                // does class have a "does" runtime part?
                State.Pos = i;
                State.Macro = m->cmds[1].u.num;
                m = &Macros[State.Macro];
            } else {
                i = m->Count;                                       // no: instance data is all we want. good bye.
            }
            break;


        case ExNew:                                                 // instantiator
            if (i) {                                                // prevent    "sub foo { new }" by skipping.
                macroclass = m->cmds[i-1].u.num & ~CMD_EXT;
                m->cmds[0].u.num = ExOld;                           // code to push address of instance data.
                m->cmds[0].repeat = ParamStack.pop();               // instance data address
                m->cmds[1].u.num = macroclass;                      // macro exec token
                m->cmds[1].repeat = doesindex;                      // index to run time code in class. this code is sort
                doesindex = 0;                                      // of an implied method, when instance is executed
            } else {
                i = m->Count;
            }
            break;


        case ExDoes:                                                // class publishes method location
            doesindex = i;
            i = m->Count;
            break;

// ------------------------------------------------------------------------------------------------------

        default:
            State.Pos = i+1;
            for (j=(m->cmds[i].repeat); j; --j) {
                ResultOfCommandExecution=ExecCommand(view, m->cmds[i].u.num, State);

                if (!(ResultOfCommandExecution || m->cmds[i].ign)) {
                    // interesting ... executed twice on Fail .. does ExecCommand (two lines above) run
                    // through this loop, execute fail, and then continue after return from ExecCommand,
                    // falling through to this fail handler again?

                    if (exception || memory[verbosity] > 1)
                        fprintf(stderr,"\n*** exception %d ***", exception);

                    faillevel++;
                    if (faillevel > 1) {
                        fprintf(stderr,"\nFail condition in OnFail macro - recursive exception handler execution, level %d", faillevel);
                        fprintf(stderr,"\nOnDoubleFail hook catches this condition\n");
                        ActiveView->ExecMacro("OnDoubleFail");
                    } else {
                        ActiveView->ExecMacro("OnFail");
                        ParamStack.empty();
                        ControlStack.empty();
                        tos = sstack.size();
                        while (tos--)
                            sstack.pop_back();
                    }
                    Nodent();                      // reset indent level - fail can happen on any level.
                    exception = 0;
                    faillevel--;
                    if (memory[verbosity])
                        fprintf(stderr,"\nReturning from fail handler level %d with fail code: %d\n", faillevel, ErFAIL);
                    return ErFAIL;
                }
            }
        }
        State.Pos=i;
    }
    Undent();
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
                    if (ExecMacro(view, key->Cmd) == ErFAIL)
                        if (memory[verbosity])
                            fprintf(stderr,"continues after fail at DispatchKey 1 - fail condition is lost here");
                    Event.What = evNone;
                    return;
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
                    if (ExecMacro(view, key->Cmd) == ErFAIL)
                        if (memory[verbosity])
                            fprintf(stderr,"continues after fail at DispatchKey 2 - fail conditions is lost here");
                    Event.What = evNone;
                    return;
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
        if (ExecMacro(view, Event.Msg.Command) == ErFAIL)
            if (memory[verbosity])
                fprintf(stderr,"continues after fail at DispatchCommand 1 - fail condition is lost here");
        Event.What = evNone;
        return;
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

// TODO: Find a way to set X
int EGUI::FileClose(EView *View, ExState &State) {
    int x = OpenAfterClose;
    return FileCloseX(View, x);
}

// TODO: Find a way to set X
int EGUI::FileCloseAll(EView *View, ExState &State) {
    int x = OpenAfterClose;
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
    int Delta = ParamStack.pop();

    if (View->ExpandHeight(Delta) == 0) {
        SUCCESS
    }
    FAIL
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
    if (sstack.size() == 0) {
        if (ActiveView)
            ActiveView->Msg(S_ERROR, "String stack underflow in RunProgram");
        FAIL
    }

    std::string cmd = sstack.back(); sstack.pop_back();

    if (ActiveModel)
        SetDefaultDirectory(ActiveModel);

    if (cmd.empty()) {
        char Cmd[MAXPATH];
        if (view->GetStr("Run", sizeof(Cmd), Cmd, HIST_COMPILE) == 0) {
            FAIL
        }
        cmd = Cmd;
    }

    gui->RunProgram(RUN_WAIT, cmd.c_str());
    SUCCESS
}

int EGUI::RunProgramAsync(ExState &State, GxView *view) {
    if (sstack.size() == 0) {
        if (ActiveView)
            ActiveView->Msg(S_ERROR, "String stack underflow in RunProgram");
        FAIL
    }

    std::string cmd = sstack.back(); sstack.pop_back();

    if (ActiveModel)
        SetDefaultDirectory(ActiveModel);

    if (cmd.empty()) {
        char Cmd[MAXPATH];
        if (view->GetStr("Run", sizeof(Cmd), Cmd, HIST_COMPILE) == 0) {
            FAIL
        }
        cmd = Cmd;
    }

    gui->RunProgram(RUN_ASYNC, cmd.c_str());
    SUCCESS
}

int EGUI::MainMenu(ExState &State, GxView *view) {
    ExModelView *V = (ExModelView *)view->Top;
    EView *View = V->View;

    if (sstack.size() == 0) {
        View->Msg(S_ERROR, "String stack underflow in MainMenu");
        FAIL
    }

    std::string mname = sstack.back(); sstack.pop_back();

    view->Parent->ExecMainMenu(mname[0]);
    return 1;
}

int EGUI::ShowMenu(ExState &State, GxView *View) {
    if (sstack.size() == 0) {
        if (ActiveView)
            ActiveView->Msg(S_ERROR, "String stack underflow in ShowMenu");
        FAIL
    }

    std::string mname = sstack.back(); sstack.pop_back();

    View->Parent->PopupMenu(mname.c_str());

    // SetBranchCondition(0); // TODO: Is this right? This is what it was to start, return 0
    // return 0;
    SUCCESS                   // TODO: as expected, showmenu fails. trying the opposite
}

int EGUI::LocalMenu(GxView *View) {
    EEventMap *Map = View->GetEventMap();
    const char *MName = 0;

    if (Map)
        MName = Map->GetMenu(EM_LocalMenu);
    if (MName == 0)
        MName = "Local";
    View->Parent->PopupMenu(MName);
    // return 0;    // TODO: again, this aborts script. not sure what the intention of menus failing is.
    return 1;
}

int EGUI::DesktopSaveAs(ExState &State, GxView *view) {
    if (sstack.size() == 0) {
        if (ActiveView)
            ActiveView->Msg(S_ERROR, "String stack underflow in DesktopSaveAs");
        FAIL
    }

    strcpy(DesktopFileName, sstack.back().c_str()); sstack.pop_back();

    if (strlen(DesktopFileName) == 0) {
        if (view->GetFile("Save Desktop", sizeof(DesktopFileName), DesktopFileName, HIST_PATH, GF_SAVEAS) == 0) {
            FAIL
        }
    }

    return SaveDesktop(DesktopFileName);
}

int EGUI::DesktopLoad(ExState &State, GxView *view) {
    if (sstack.size() == 0) {
        if (ActiveView)
            ActiveView->Msg(S_ERROR, "String stack underflow in DesktopLoad");
        FAIL
    }

    strcpy(DesktopFileName, sstack.back().c_str()); sstack.pop_back();

    if (strlen(DesktopFileName) == 0) {
        if (view->GetFile("Load Desktop", sizeof(DesktopFileName), DesktopFileName, HIST_PATH, GF_SAVEAS) == 0) {
            FAIL
        }
    }

    return LoadDesktop(DesktopFileName);
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

    int res = ExecMacro(view, "OnFrameNew");
     SetBranchCondition(res);
    return res;
}

int EGUI::FrameClose(GxView *View) {
    assert(frames != 0);
    assert(View != 0);

    if (!frames->isLastFrame()) {
        deleteFrame(frames);
    } else {
        if (ExitEditor(ActiveView) == 0) {
            FAIL
        }
        deleteFrame(frames);
    }

    int res = ExecMacro(View, "OnFrameClose");
    SetBranchCondition(res);
    return res;
}

int EGUI::FrameNext(GxView * View) {
    int res = 0;

    if (!frames->isLastFrame()) {
        frames->Next->Activate();

        res = ExecMacro(View, "OnFrameNext");
    }

    // SetBranchCondition(res);
    return res;
}

int EGUI::FramePrev(GxView *View) {
    int res = 0;

    if (!frames->isLastFrame()) {
        frames->Prev->Activate();

        res = ExecMacro(View, "OnFramePrev");
    }

    // SetBranchCondition(res);
    return res;
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

    ActiveView->ExecMacro("OnBoot");
    //ActiveView->ExecMacro("OnUserBoot");  // OnBoot can provide this
    if (StartupMacroCommand != NULL)
        ActiveView->ExecMacro(StartupMacroCommand);

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

    if (memory[verbosity] > 1) {
        unsigned mem=0;
        int mc = CMacros;
        while (mc--) {
            if (Macros[mc].Name != NULL)
                mem += strlen(Macros[mc].Name);
            for (int i = 0; i < Macros[mc].Count; ++i)
                if (Macros[mc].cmds[i].type == CT_STRING)
                    mem += strlen(Macros[mc].cmds[i].u.string);
            mem += sizeof(ExMacro);
        }
        fprintf(stderr, "Macro count: %i memory: %i\n", CMacros, mem);
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