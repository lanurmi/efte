/*    o_routine.cpp
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"

RoutineView::RoutineView(int createFlags, EModel **ARoot, EBuffer *AB): EList(createFlags, ARoot, "Routines") {
    SearchLen = 0;

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
}

RoutineView::~RoutineView() {
    Buffer->Routines = 0;
}

EEventMap *RoutineView::GetEventMap() {
    return FindEventMap("ROUTINES");
}

int RoutineView::ExecCommand(int Command, ExState &State) {
    switch (Command) {
    case ExRescan:
        CancelSearch();
        Buffer->ScanForRoutines();
        UpdateList();
        return ErOK;

    case ExActivateInOtherWindow:
        CancelSearch();
        if (Row < Buffer->rlst.Count) {
            View->Next->SwitchToModel(Buffer);
            Buffer->CenterPosR(0, Buffer->rlst.Lines[Row]);
            return ErOK;
        }
        return ErFAIL;

    case ExCloseActivate:
        CancelSearch();
        return ErFAIL;
    }
    return EList::ExecCommand(Command, State);
}

void RoutineView::HandleEvent(TEvent &Event) {
    int resetSearch = 1;
    EModel::HandleEvent(Event);
    switch (Event.What) {
    case evKeyUp:
        resetSearch = 0;
        break;
    case evKeyDown:
        switch (kbCode(Event.Key.Code)) {
        case kbBackSp:
            resetSearch = 0;
            if (SearchLen > 0) {
                SearchString[--SearchLen] = 0;
                Row = SearchPos[SearchLen];
                Msg(S_INFO, "Search: [%s]", SearchString);
            } else
                Msg(S_INFO, "");
            break;
        case kbEsc:
            Msg(S_INFO, "");
            break;
        default:
            resetSearch = 0;
            if (isAscii(Event.Key.Code) && (SearchLen < MAXISEARCH)) {
                char Ch = (char) Event.Key.Code;

                SearchPos[SearchLen] = Row;
                SearchString[SearchLen] = Ch;
                SearchString[++SearchLen] = 0;
                int i = getMatchingLine(Row, 1);
                if (i == -1)
                    SearchString[--SearchLen] = 0;
                else
                    Row = i;
                Msg(S_INFO, "Search: [%s]", SearchString);
            }
            break;
        }
    }
    if (resetSearch) {
        SearchLen = 0;
    }
}

/**
 * Search for next line containing SearchString starting from line 'start'.
 * Direction should be 1 for ascending and -1 for descending.
 * Returns line found or -1 if none.
 */
int RoutineView::getMatchingLine(int start, int direction) {
    int i = start;
    do {
        char *str = Buffer->RLine(Buffer->rlst.Lines[i])->Chars;
        for (int j = 0; str[j]; j++) {
            if (str[j] == SearchString[0] && strnicmp(SearchString, str + j, SearchLen) == 0) {
                return i;
            }
        }
        i += direction;
        if (i == Count) i = 0;
        else if (i == -1) i = Count - 1;
    } while (i != start);

    return -1;
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
    CancelSearch();
    if (No < Buffer->rlst.Count) {
        View->SwitchToModel(Buffer);
        Buffer->CenterPosR(0, Buffer->rlst.Lines[No]);
        return 1;
    }
    return 0;
}

void RoutineView::CancelSearch() {
    SearchLen = 0;
    Msg(S_INFO, "");
}

void RoutineView::RescanList() {
    Buffer->ScanForRoutines();
    UpdateList();
    NeedsRedraw = 1;
}

void RoutineView::UpdateList() {
    Count = Buffer->rlst.Count;
    NeedsUpdate = 1;
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
