/*    o_list.cpp
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"

EListPort::EListPort(EList *L, EView *V): EViewPort(V) {
    List = L;
    OldTopRow = OldLeftCol = OldRow = OldCount = -1;
    GetPos();
}

EListPort::~EListPort() {
    StorePos();
}

void EListPort::StorePos() {
    List->Row = Row;
    List->TopRow = TopRow;
    List->LeftCol = LeftCol;
    List->NeedsUpdate = 1;
}

void EListPort::GetPos() {
    Row = List->Row;
    TopRow = List->TopRow;
    LeftCol = List->LeftCol;
}

void EListPort::HandleEvent(TEvent &Event) {
    int W = 1;
    int H = 1;

    EViewPort::HandleEvent(Event);

    if (View && View->MView && View->MView->Win) {
        View->MView->ConQuerySize(&W, &H);
        H--;
    }

    switch (Event.What) {
    case evCommand:
        switch (Event.Msg.Command) {
        case cmVScrollUp:
            List->ScrollDown(Event.Msg.Param1);
            Event.What = evNone;
            break;
        case cmVScrollDown:
            List->ScrollUp(Event.Msg.Param1);
            Event.What = evNone;
            break;
        case cmVScrollPgUp:
            List->MovePageUp();
            Event.What = evNone;
            break;
        case cmVScrollPgDn:
            List->MovePageDown();
            Event.What = evNone;
            break;
        case cmVScrollMove: {
            int ypos;

            ypos = List->Row - List->TopRow;
            List->TopRow = Event.Msg.Param1;
            List->Row = List->TopRow + ypos;
        }
        Event.What = evNone;
        break;
        case cmHScrollLeft:
            List->ScrollRight(Event.Msg.Param1);
            Event.What = evNone;
            break;
        case cmHScrollRight:
            List->ScrollLeft(Event.Msg.Param1);
            Event.What = evNone;
            break;
        case cmHScrollPgLt:
            List->ScrollRight(W);
            Event.What = evNone;
            break;
        case cmHScrollPgRt:
            List->ScrollLeft(W);
            Event.What = evNone;
            break;
        case cmHScrollMove:
            List->LeftCol = Event.Msg.Param1;
            Event.What = evNone;
            break;
        }
        break;
    case evMouseDown:
    case evMouseUp:
    case evMouseMove:
    case evMouseAuto:
        HandleMouse(Event);
        break;
    }
}

void EListPort::HandleMouse(TEvent &Event) {
    int W, H;
    int x, y, xx, yy;

    View->MView->ConQuerySize(&W, &H);

    x = Event.Mouse.X;
    y = Event.Mouse.Y;
    yy = y + TopRow;
    xx = x + LeftCol;
//    if (yy >= Selected) yy = Window->Buffer->VCount - 1;
    if (yy < 0) yy = 0;
    if (xx < 0) xx = 0;

    switch (Event.What) {
    case evMouseDown:
        if (Event.Mouse.Y == H - 1)
            break;
        if (View->MView->Win->CaptureMouse(1))
            View->MView->MouseCaptured = 1;
        else
            break;

        if (Event.Mouse.Buttons == 1)
            if (yy < List->Count && yy >= 0) {
                List->SetPos(yy, LeftCol);
                if (Event.Mouse.Count == 2) {
                    if (List->CanActivate(List->Row)) {
                        View->MView->Win->CaptureMouse(0);
                        if (List->Activate() == 1) {
                            //View->MView->EndExec(1);
                        }
                    }
                }
            }
        if (Event.Mouse.Buttons == 2)
            if (yy < List->Count && yy >= 0)
                List->SetPos(yy, LeftCol);
        Event.What = evNone;
        break;
    case evMouseAuto:
    case evMouseMove:
        if (View->MView->MouseCaptured) {
            if (Event.Mouse.Buttons == 1 || Event.Mouse.Buttons == 2)
                if (yy < List->Count && yy >= 0) {
                    List->SetPos(yy, LeftCol);
                }
            Event.What = evNone;
        }
        break;
    case evMouseUp:
        if (View->MView->MouseCaptured)
            View->MView->Win->CaptureMouse(0);
        else
            break;
        if (Event.Mouse.Buttons == 2) {
            EEventMap *Map = View->MView->Win->GetEventMap();
            const char *MName = 0;

            if (yy < List->Count && yy >= 0) {
                List->SetPos(yy, LeftCol);
            }

            if (Map)
                MName = Map->GetMenu(EM_LocalMenu);
            if (MName == 0)
                MName = "Local";
            View->MView->Win->Parent->PopupMenu(MName);
        }
        View->MView->MouseCaptured = 0;
        Event.What = evNone;
        break;
    }
}

void EListPort::UpdateView() {
    if (OldLeftCol != LeftCol || OldTopRow != TopRow || OldCount != List->Count)
        List->NeedsRedraw = List->NeedsUpdate = 1;

    if (List->NeedsUpdate) {

        List->UpdateList();

        List->FixPos();

        if (List->View == View)
            GetPos();

        if (OldLeftCol != LeftCol || OldTopRow != TopRow || OldCount != List->Count)
            List->NeedsRedraw = List->NeedsUpdate = 1;

        PaintView(List->NeedsRedraw);
        OldRow = Row;
        OldTopRow = TopRow;
        OldLeftCol = LeftCol;
        OldCount = List->Count;
        List->NeedsUpdate = 0;
        List->NeedsRedraw = 0;
    }
}

void EListPort::RepaintView() {
    PaintView(1);
    OldRow = Row;
    OldTopRow = TopRow;
    OldLeftCol = LeftCol;
    OldCount = List->Count;
    List->NeedsUpdate = 0;
    List->NeedsRedraw = 0;
}

void EListPort::PaintView(int PaintAll) {
    TDrawBuffer B;
    int I;
    ChColor color;
    int W, H;

    if (List->NeedsRedraw)
        PaintAll = 1;

    if (View == 0 || View->MView == 0 || View->MView->Win == 0)
        return ;

    View->MView->ConQuerySize(&W, &H);
    H--;

    if (View->MView->Win->GetViewContext() != View->MView)
        return;
    for (I = 0; I < H; I++) {
        if (PaintAll || I + TopRow == Row || I + TopRow == OldRow) {
            int mark = List->IsMarked(I + TopRow);
            int hilit = List->IsHilited(I + TopRow);
            color = ((Row == I + TopRow) && View->MView->Win->IsActive())
                    ? (mark ? (hilit ? hcList_MarkHilitSel : hcList_MarkSelect) :
                               (hilit ? hcList_HilitSelect : hcList_Selected)) :
                            (mark ? (hilit ? hcList_MarkHilit : hcList_Marked) :
                             (hilit ? hcList_Hilited : hcList_Normal));
            MoveChar(B, 0, W, ' ', color, W);
            if (I + TopRow < List->Count)
                List->DrawLine(B, I + TopRow, LeftCol, color, W);
            View->MView->ConPutBox(0, I, W, 1, B);
        }
    }
}

void EListPort::UpdateStatus() {
    RepaintStatus();
}

void EListPort::RepaintStatus() {
    TDrawBuffer B;
    char s[80];
    int W, H;
    char SColor;

    if (View == 0 || View->MView == 0 || View->MView->Win == 0)
        return ;

    View->MView->ConQuerySize(&W, &H);

    List->UpdateList();

    List->FixPos();

    if (List->View == View)
        GetPos();

    if (View->MView->Win->GetStatusContext() != View->MView)
        return;

    View->MView->Win->SetSbVPos(TopRow, H, List->Count + (WeirdScroll ? H - 1 : 0));
    View->MView->Win->SetSbHPos(LeftCol, W, 1024 + (WeirdScroll ? W - 1 : 0));

    if (View->MView->IsActive()) // hack
        SColor = hcStatus_Active;
    else
        SColor = hcStatus_Normal;
    MoveCh(B, ' ', SColor, W);
    if (View->CurMsg == 0) {
        if (List->Title)
            MoveStr(B, 0, W, List->Title, SColor, W);
        sprintf(s, "%c%d/%d", ConGetDrawChar(DCH_V), Row + 1, List->Count);
        MoveStr(B, W - strlen(s), W, s, SColor, W);
    } else {
        MoveStr(B, 0, W, View->CurMsg, SColor, W);
    }
    View->MView->ConPutBox(0, H - 1, W, 1, B);

    if (View->MView->Win->GetStatusContext() == View->MView &&
            View->MView->Win->IsActive())
        View->MView->Win->ConSetCursorPos(0, Row - TopRow);
}

EList::EList(int createFlags, EModel **ARoot, const char *aTitle): EModel(createFlags, ARoot) {
    Title = strdup(aTitle);
    Row = TopRow = Count = LeftCol = 0;
    NeedsUpdate = 1;
    NeedsRedraw = 1;
    MouseMoved = 0;
    MouseCaptured = 0;
}

EList::~EList() {
    free(Title);
}

EViewPort *EList::CreateViewPort(EView *V) {
    V->Port = new EListPort(this, V);
    AddView(V);

    return V->Port;
}

EListPort *EList::GetViewVPort(EView *V) {
    return (EListPort *)V->Port;
}
EListPort *EList::GetVPort() {
    return (EListPort *)View->Port;
}

void EList::SetTitle(char *ATitle) {
    if (Title)
        free(Title);
    Title = strdup(ATitle);
    if (View && View->MView)
        View->MView->RepaintStatus();
}


int EList::ExecCommand(int Command, ExState &State) {
    int W = 1;
    int H = 1;

    if (View && View->MView && View->MView->Win) {
        View->MView->ConQuerySize(&W, &H);
        H--;
    }
    FixPos();
    switch (Command) {
    case ExMoveLeft:
        return MoveLeft();
    case ExMoveRight:
        return MoveRight();
    case ExMoveUp:
        return MoveUp();
    case ExMoveDown:
        return MoveDown();
    case ExMovePageUp:
        return MovePageUp();
    case ExMovePageDown:
        return MovePageDown();
    case ExScrollLeft:
        return ScrollLeft(8);
    case ExScrollRight:
        return ScrollRight(8);
    case ExMovePageStart:
        return MovePageStart();
    case ExMovePageEnd:
        return MovePageEnd();
    case ExMoveFileStart:
        return MoveFileStart();
    case ExMoveFileEnd:
        return MoveFileEnd();
    case ExMoveLineStart:
        return MoveLineStart();
    case ExMoveLineEnd:
        return MoveLineEnd();
    case ExRescan:
        RescanList();
        return ErOK;
    case ExActivate:
        return Activate();
    case ExListMark:
        return Mark();
    case ExListUnmark:
        return Unmark();
    case ExListToggleMark:
        return ToggleMark();
    case ExListMarkAll:
        return MarkAll();
    case ExListUnmarkAll:
        return UnmarkAll();
    case ExListToggleMarkAll:
        return ToggleMarkAll();
    }
    return EModel::ExecCommand(Command, State);
}

EEventMap *EList::GetEventMap() {
    return FindEventMap("LIST");
}

void EList::HandleEvent(TEvent &/*Event*/) {
}

void EList::DrawLine(PCell /*B*/, int /*Line*/, int /*Col*/, ChColor /*color*/, int /*Width*/) {
}

char *EList::FormatLine(int /*Line*/) {
    return 0;
}

void EList::RescanList() {}
void EList::UpdateList() {
    NeedsUpdate = 1;
}
void EList::FreeList() {}

void EList::FixPos() {
    int W, H;
    int OTopRow = TopRow;
    int OLeftCol = LeftCol;
    int ORow = Row;

    if (View == 0 || View->MView == 0 || View->MView->Win == 0)
        return ;

    View->MView->Win->ConQuerySize(&W, &H);
    H--;

    //int scrollJumpX = Min(ScrollJumpX, W / 2);
    int scrollJumpY = Min(ScrollJumpY, H / 2);
    //int scrollBorderX = Min(ScrollBorderX, W / 2);
    int scrollBorderY = Min(ScrollBorderY, H / 2);

    if (LeftCol < 0) LeftCol = 0;
    if (Row >= Count) Row = Count - 1;
    if (!WeirdScroll)
        if (TopRow + H > Count) TopRow = Count - H;
    if (Row < 0) Row = 0;

    if (GetVPort()->ReCenter) {
        TopRow = Row - H / 2;
        GetVPort()->ReCenter = 0;
    }
    if (TopRow + scrollBorderY > Row) TopRow = Row - scrollJumpY + 1 - scrollBorderY;
    if (TopRow + H - scrollBorderY <= Row) TopRow = Row - H + 1 + scrollJumpY - 1 + scrollBorderY;
    if (TopRow < 0) TopRow = 0;

    if (OTopRow != TopRow || OLeftCol != LeftCol || ORow != Row) {
        NeedsRedraw = 1;
        NeedsUpdate = 1;
    }
}

int EList::GetContext() {
    return CONTEXT_LIST;
}
int EList::BeginMacro() {
    return 1;
}
int EList::CanActivate(int /*Line*/) {
    return 1;
}
int EList::Activate(int /*No*/) {
    return 0;
}
int EList::IsHilited(int /*Line*/) {
    return 0;
}
int EList::IsMarked(int /*Line*/) {
    return 0;
}
int EList::Mark(int /*Line*/) {
    return 1;
}
int EList::Unmark(int /*Line*/) {
    return 1;
}

int EList::SetPos(int ARow, int ACol) {
    Row = ARow;
    LeftCol = ACol;
    NeedsUpdate = 1;
    return ErOK;
}

int EList::MoveLeft() {
    if (LeftCol == 0)
        return ErFAIL;
    LeftCol--;
    NeedsUpdate = 1;
    return ErOK;
}

int EList::MoveRight() {
    LeftCol++;
    NeedsUpdate = 1;
    return ErOK;
}

int EList::MoveUp() {
    if (Row == 0)
        return ErFAIL;
    Row--;
    NeedsUpdate = 1;
    return ErOK;
}

int EList::MoveDown() {
    if (Row == Count - 1)
        return ErFAIL;
    Row++;
    NeedsUpdate = 1;
    return ErOK;
}

int EList::MoveLineStart() {
    if (LeftCol != 0) {
        NeedsUpdate = 1;
        LeftCol = 0;
    }
    return ErOK;
}

// Move current column position to end of line, if line is shorter, stay at
// begining of line, if it is long enough move it to end and subtract
// screen width / 2 from actual length
int EList::MoveLineEnd() {
    int W, H, len;

    View->MView->Win->ConQuerySize(&W, &H);
    H--;

    len = GetRowLength(Row);
    if (len < W) {
        if (LeftCol != 0) {
            LeftCol = 0;
            NeedsUpdate = 1;
        }
    } else
        if (LeftCol != len - W / 2) {
            LeftCol = len - W / 2;
            NeedsUpdate = 1;
        }
    return ErOK;
}

int EList::MovePageUp() {
    int W, H;

    if (Row == 0)
        return ErFAIL;

    View->MView->Win->ConQuerySize(&W, &H);
    H--;

    Row -= H;
    TopRow -= H;
    if (Row < 0)
        Row = 0;
    if (TopRow < 0)
        TopRow = 0;
    NeedsUpdate = 1;
    return ErOK;
}

int EList::MovePageDown() {
    int W, H;

    if (Row == Count - 1)
        return ErFAIL;

    View->MView->Win->ConQuerySize(&W, &H);
    H--;

    Row += H;
    TopRow += H;
    if (Row >= Count)
        Row = Count - 1;
    if (TopRow > Row)
        TopRow = Row;
    if (Row < 0)
        Row = 0;
    if (TopRow < 0)
        TopRow = 0;
    NeedsUpdate = 1;
    return ErOK;
}

int EList::ScrollRight(int Cols) {
    if (LeftCol >= Cols) {
        LeftCol -= Cols;
        NeedsUpdate = 1;
    } else if (LeftCol != 0) {
        LeftCol = 0;
        NeedsUpdate = 1;
    } else
        return ErFAIL;
    return ErOK;
}

int EList::ScrollLeft(int Cols) {
    LeftCol += Cols;
    NeedsUpdate = 1;
    return ErOK;
}

int EList::ScrollUp(int Rows) {
    if (TopRow == Count - 1)
        return ErFAIL;

    TopRow += Rows;
    Row += Rows;

    if (Row >= Count)
        Row = Count - 1;
    if (Row < 0)
        Row = 0;
    if (TopRow > Row)
        TopRow = Row;
    NeedsUpdate = 1;
    return ErOK;
}

int EList::ScrollDown(int Rows) {
    TopRow -= Rows;
    Row -= Rows;

    if (Row < 0)
        Row = 0;
    if (TopRow < 0)
        TopRow = 0;
    NeedsUpdate = 1;
    return ErOK;
}

int EList::MovePageStart() {
    if (Row <= TopRow)
        return ErFAIL;
    Row = TopRow;
    NeedsUpdate = 1;
    return ErOK;
}

int EList::MovePageEnd() {
    int W, H;

    if (Row == Count - 1)
        return ErOK;

    View->MView->Win->ConQuerySize(&W, &H);
    H--;
    if (Row == TopRow + H - 1)
        return ErOK;

    Row = TopRow + H - 1;
    if (Row >= Count)
        Row = Count - 1;
    if (Row < 0)
        Row = 0;
    NeedsUpdate = 1;
    return ErOK;
}

int EList::MoveFileStart() {
    if (Row == 0 && LeftCol == 0)
        return ErOK;
    Row = 0;
    LeftCol = 0;
    NeedsUpdate = 1;
    return ErOK;
}

int EList::MoveFileEnd() {
    if (Row == Count - 1 && LeftCol == 0)
        return 0;
    Row = Count - 1;
    if (Row < 0)
        Row = 0;
    NeedsUpdate = 1;
    LeftCol = 0;
    return ErOK;
}

int EList::Activate() {
    if (Count > 0)
        if (CanActivate(Row))
            if (Activate(Row) == 1)
                return ErOK;
    return ErFAIL;
}

int EList::Mark() {
    if (Count > 0 && ! IsMarked(Row) && Mark(Row) == 1) {
        NeedsRedraw = 1;
        return ErOK;
    } else return ErFAIL;
}

int EList::Unmark() {
    if (Count > 0 && IsMarked(Row) && Unmark(Row) == 1) {
        NeedsRedraw = 1;
        return ErOK;
    } else return ErFAIL;
}

int EList::ToggleMark() {
    if (Count > 0) {
        if (IsMarked(Row)) {
            if (Unmark(Row) == 1) {
                NeedsRedraw = 1;
                return ErOK;
            }
        } else {
            if (Mark(Row) == 1) {
                NeedsRedraw = 1;
                return ErOK;
            }
        }
    }
    return ErFAIL;
}

int EList::MarkAll() {
    NeedsRedraw = 1;
    for (int i = 0; i < Count; i++) {
        if (! IsMarked(i))
            if (Mark(i) != 1) return ErFAIL;
    }
    return ErOK;
}

int EList::UnmarkAll() {
    NeedsRedraw = 1;
    for (int i = 0; i < Count; i++) {
        if (IsMarked(i))
            if (Unmark(i) != 1) return ErFAIL;
    }
    return ErOK;
}

int EList::ToggleMarkAll() {
    NeedsRedraw = 1;
    for (int i = 0; i < Count; i++) {
        if (IsMarked(i)) {
            if (Unmark(i) != 1) return ErFAIL;
        } else {
            if (Mark(i) != 1) return ErFAIL;
        }
    }
    return ErOK;
}
