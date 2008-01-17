/*    o_routine.cpp
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"

#ifdef CONFIG_OBJ_ROUTINE
RoutineView::RoutineView(int createFlags, EModel **ARoot, EBuffer *AB): EList(createFlags, ARoot, "Routines") {
    Buffer = AB;
    if (Buffer->rlst.Count == 0)
        Buffer->ScanForRoutines();
    Row = 0;
    int Row = Buffer->VToR(Buffer->CP.Row);
    for (int i = Buffer->rlst.Count - 1; i >= 0; --i)
        if (Row >= Buffer->rlst.Lines[i]) {
            Row = i;
            break;
        }
    {
        char CTitle[256];
    
        sprintf(CTitle, "Routines %s: %d",
                Buffer->FileName,
                Buffer->rlst.Count);
        SetTitle(CTitle);
    }
};

RoutineView::~RoutineView() {
    Buffer->Routines = 0;
}

EEventMap *RoutineView::GetEventMap() {
    return FindEventMap("ROUTINES");
}

int RoutineView::ExecCommand(int Command, ExState &State) {
    switch (Command) {
    case ExRescan:
        Buffer->ScanForRoutines();
        UpdateList();
        return ErOK;

    case ExActivateInOtherWindow:
        if (Row < Buffer->rlst.Count) {
            View->Next->SwitchToModel(Buffer);
            Buffer->CenterPosR(0, Buffer->rlst.Lines[Row]);
            return ErOK;
        }
        return ErFAIL;
        
    case ExCloseActivate:
        return ErFAIL;
    }
    return EList::ExecCommand(Command, State);
}
    
void RoutineView::DrawLine(PCell B, int Line, int Col, ChColor color, int Width) {
    if (Buffer->RLine(Buffer->rlst.Lines[Line])->Count > Col) {
        char str[1024];
        int len;

        len = UnTabStr(str, sizeof(str),
                       Buffer->RLine(Buffer->rlst.Lines[Line])->Chars,
                       Buffer->RLine(Buffer->rlst.Lines[Line])->Count);
                    
        if (len > Col)
            MoveStr(B, 0, Width, str + Col, color, len - Col);
    }
}

char* RoutineView::FormatLine(int Line) {
    char *p = 0;
    PELine L = Buffer->RLine(Buffer->rlst.Lines[Line]);
    
    p = (char *) malloc(L->Count + 1);
    if (p) {
        memcpy(p, L->Chars, L->Count);
        p[L->Count] = 0;
    }
    return p;
}

int RoutineView::Activate(int No) {
    if (No < Buffer->rlst.Count) {
        View->SwitchToModel(Buffer);
        Buffer->CenterPosR(0, Buffer->rlst.Lines[No]);
        return 1;
    }
    return 0;
}

void RoutineView::RescanList() {
    Buffer->ScanForRoutines();
    UpdateList();
    NeedsRedraw = 1;
}

void RoutineView::UpdateList() {
    Count = Buffer->rlst.Count;
}

int RoutineView::GetContext() {
    return CONTEXT_ROUTINES;
}

void RoutineView::GetName(char *AName, int MaxLen) {
    strncpy(AName, "Routines", MaxLen);
}

void RoutineView::GetInfo(char *AInfo, int /*MaxLen*/) {
    sprintf(AInfo, "%2d %04d/%03d Routines (%s)", ModelNo, Row + 1, Count, Buffer->FileName);
}

void RoutineView::GetTitle(char *ATitle, int /*MaxLen*/, char *ASTitle, int SMaxLen) {
    sprintf(ATitle, "Routines: %s", Buffer->FileName);
    strncpy(ASTitle, "Routines", SMaxLen);
    ASTitle[SMaxLen - 1] = 0;
}
#endif
