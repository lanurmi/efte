/*    g_text.cpp
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <stdarg.h>
#if defined(MSVC)
#include <malloc.h>
#endif

#include "console.h"
#include "gui.h"
#include "c_mode.h"
#include "c_color.h"

#if defined(_DEBUG) && defined(MSVC) && defined(MSVCDEBUG)
#include <crtdbg.h>

#define new new( _CLIENT_BLOCK, __FILE__, __LINE__)

#endif //_DEBUG && MSVC && MSVCDEBUG


int ShowVScroll = 1;
int ShowHScroll = 1;
int ShowMenuBar = 1;
int ShowToolBar = 0;
unsigned long HaveGUIDialogs = 0; // no gui dialogs in text gui

GFrame *frames = 0;
GUI *gui = 0;

GView *MouseCapture = 0;
GView *FocusCapture = 0;

TEvent NextEvent = { 0 };

#define sfFocus   1

class UpMenu;

extern int ExecMainMenu(TEvent &E, char sub);
extern int ExecVertMenu(int x, int y, int id, TEvent &E, UpMenu *up);

class GViewPeer {
public:
    GView *View;
    int wX, wY, wW, wH, wState;
    int cX, cY, cVisible;
    int sbVstart, sbVamount, sbVtotal, sbVupdate;
    int sbHstart, sbHamount, sbHtotal, sbHupdate;
    int SbVBegin, SbVEnd, SbHBegin, SbHEnd;
    bool insertState;

    GViewPeer(GView *view, int XSize, int YSize);
    ~GViewPeer();

    int ConPutBox(int X, int Y, int W, int H, PCell Cell);
    int ConGetBox(int X, int Y, int W, int H, PCell Cell);
    int ConPutLine(int X, int Y, int W, int H, PCell Cell);
    int ConSetBox(int X, int Y, int W, int H, TCell Cell);
    int ConScroll(int Way, int X, int Y, int W, int H, TAttr Fill, int Count);

    int ConSetSize(int X, int Y);
    int ConQuerySize(int *X, int *Y);

    int ConSetCursorPos(int X, int Y);
    int ConQueryCursorPos(int *X, int *Y);
    int ConShowCursor();
    int ConHideCursor();
    int ConCursorVisible();
    void ConSetInsertState(bool insert);

    int CaptureMouse(int grab);
    int CaptureFocus(int grab);

    int QuerySbVPos();
    int SetSbVPos(int Start, int Amount, int Total);
    int SetSbHPos(int Start, int Amount, int Total);
    int ExpandHeight(int DeltaY);

    int DrawScrollBar();
    int UpdateCursor();
};

class GFramePeer {
public:
    int fW, fH;
    GFrame *Frame;

    GFramePeer(GFrame *aFrame, int Width, int Height);
    ~GFramePeer();

    int ConSetTitle(char *Title, char *STitle);
    int ConGetTitle(char *Title, int MaxLen, char *STitle, int SMaxLen);

    int ConSetSize(int X, int Y);
    int ConQuerySize(int *X, int *Y);

};

///////////////////////////////////////////////////////////////////////////

GViewPeer::GViewPeer(GView *view, int XSize, int YSize) {
    View = view;
    wX = 0;
    wY = 0;
    wW = XSize;
    wH = YSize;
    sbVtotal = 0;
    sbVstart = 0;
    sbVamount = 0;
    sbVupdate = 1;
    sbHtotal = 0;
    sbHstart = 0;
    sbHamount = 0;
    sbHupdate = 1;
    wState = 0;
    cVisible = 1;
    cX = cY = 0;
}

GViewPeer::~GViewPeer() {
    if (MouseCapture == View)
        MouseCapture = 0;
    if (FocusCapture == View)
        FocusCapture = 0;
}

int GViewPeer::ConPutBox(int X, int Y, int W, int H, PCell Cell) {
    return ::ConPutBox(X + wX, Y + wY, W, H, Cell);
}

int GViewPeer::ConGetBox(int X, int Y, int W, int H, PCell Cell) {
    return ::ConGetBox(X + wX, Y + wY, W, H, Cell);
}

int GViewPeer::ConPutLine(int X, int Y, int W, int H, PCell Cell) {
    return ::ConPutLine(X + wX, Y + wY, W, H, Cell);
}

int GViewPeer::ConSetBox(int X, int Y, int W, int H, TCell Cell) {
    return ::ConSetBox(X + wX, Y + wY, W, H, Cell);
}

int GViewPeer::ConScroll(int Way, int X, int Y, int W, int H, TAttr Fill, int Count) {
    return ::ConScroll(Way, X + wX, Y + wY, W, H, Fill, Count);
}

int GViewPeer::ConSetSize(int X, int Y) {
    wW = X;
    wH = Y;
    return 1;
}

int GViewPeer::ConQuerySize(int *X, int *Y) {
    if (X) *X = wW;
    if (Y) *Y = wH;
    return 1;
}

int GViewPeer::ConSetCursorPos(int X, int Y) {
    if (X < 0) X = 0;
    if (X >= wW) X = wW - 1;
    if (Y < 0) Y = 0;
    if (Y >= wH) Y = wH - 1;
    cX = X;
    cY = Y;
    if (wState & sfFocus)
        return ::ConSetCursorPos(cX + wX, cY + wY);
    else
        return 1;
}

int GViewPeer::ConQueryCursorPos(int *X, int *Y) {
    if (X) *X = cX;
    if (Y) *Y = cY;
    return 1;
}

int GViewPeer::ConShowCursor() {
    cVisible = 1;
    if (wState & sfFocus)
        return ::ConShowCursor();
    else
        return 1;
}

int GViewPeer::ConHideCursor() {
    cVisible = 0;
    if (wState & sfFocus)
        return ::ConHideCursor();
    else
        return 1;
}

int GViewPeer::ConCursorVisible() {
    return cVisible;
}

void GViewPeer::ConSetInsertState(bool insert) {
    insertState = insert;
    ::ConSetInsertState(insert);
}

int GViewPeer::CaptureMouse(int grab) {
    if (MouseCapture == 0) {
        if (grab)
            MouseCapture = View;
        else
            return 0;
    } else {
        if (grab || MouseCapture != View)
            return 0;
        else
            MouseCapture = 0;
    }
    return 1;
}

int GViewPeer::CaptureFocus(int grab) {
    if (FocusCapture == 0) {
        if (grab)
            FocusCapture = View;
        else
            return 0;
    } else {
        if (grab || FocusCapture != View)
            return 0;
        else
            FocusCapture = 0;
    }
    return 1;
}

int GViewPeer::ExpandHeight(int DeltaY) {
    if (View->Parent->Top == View->Next)
        return -1;
    if (DeltaY + wH < 3)
        DeltaY = - (wH - 3);
    if (View->Next->Peer->wH - DeltaY < 3)
        DeltaY = View->Next->Peer->wH - 3;
    View->Peer->ConSetSize(wW, wH + DeltaY);
    View->Next->Peer->wY += DeltaY;
    View->Next->Peer->ConSetSize(View->Next->Peer->wW, View->Next->Peer->wH - DeltaY);
    View->Resize(View->Peer->wW, View->Peer->wH);
    View->Next->Resize(View->Next->Peer->wW, View->Next->Peer->wH);
    return 0;
}

int GViewPeer::QuerySbVPos() {
    return sbVstart;
}

int GViewPeer::SetSbVPos(int Start, int Amount, int Total) {
    if (sbVstart != Start ||
            sbVamount != Amount ||
            sbVtotal != Total) {
        sbVstart = Start;
        sbVamount = Amount;
        sbVtotal = Total;
        sbVupdate = 1;
//        DrawScrollBar();
    }
    return 1;
}

int GViewPeer::SetSbHPos(int Start, int Amount, int Total) {
    if (sbHstart != Start ||
            sbHamount != Amount ||
            sbHtotal != Total) {
        sbHstart = Start;
        sbHamount = Amount;
        sbHtotal = Total;
        sbHupdate = 1;
//        DrawScrollBar();
    }
    return 1;
}

int GViewPeer::UpdateCursor() {
    ConSetCursorPos(cX, cY);
    ConSetInsertState(insertState);
    if (cVisible)
        ConShowCursor();
    else
        ConHideCursor();
    return 0;
}

int GViewPeer::DrawScrollBar() {
    TDrawBuffer B;
    int NRows, NCols, I;
    int W, H;
    char fore = ConGetDrawChar(DCH_HFORE);
    char back = ConGetDrawChar(DCH_HBACK);

    ConQuerySize(&W, &H);

    if (ShowVScroll) {
        MoveCh(B, ConGetDrawChar(DCH_AUP), hcScrollBar_Arrows, 1);
        ConPutBox(W, 0, 1, 1, B);
        MoveCh(B, ConGetDrawChar(DCH_ADOWN), hcScrollBar_Arrows, 1);
        ConPutBox(W, H - 1, 1, 1, B);

        NRows = H - 2;

        if (sbVtotal <= NRows) {
            SbVBegin = 0;
            SbVEnd = NRows - 1;
        } else {
            SbVBegin = NRows * sbVstart / sbVtotal;
            SbVEnd   = SbVBegin + NRows * sbVamount / sbVtotal;
        }

        for (I = 0; I < NRows; I++) {
            if (I >= SbVBegin && I <= SbVEnd)
                MoveCh(B, fore, hcScrollBar_Fore, 1);
            else
                MoveCh(B, back, hcScrollBar_Back, 1);
            ConPutBox(W, I + 1, 1, 1, B);
        }
    }
    if (ShowHScroll) {
        MoveCh(B, ConGetDrawChar(DCH_ALEFT), hcScrollBar_Arrows, 1);
        ConPutBox(0, H, 1, 1, B);
        MoveCh(B, ConGetDrawChar(DCH_ARIGHT), hcScrollBar_Arrows, 1);
        ConPutBox(W - 1, H, 1, 1, B);

        NCols = W - 2;

        if (sbHtotal <= NCols) {
            SbHBegin = 0;
            SbHEnd = NCols - 1;
        } else {
            SbHBegin = NCols * sbHstart / sbHtotal;
            SbHEnd   = SbHBegin + NCols * sbHamount / sbHtotal;
        }

        // could be made faster
        for (I = 0; I < NCols; I++) {
            if (I >= SbHBegin && I <= SbHEnd)
                MoveCh(B, fore, hcScrollBar_Fore, 1);
            else
                MoveCh(B, back, hcScrollBar_Back, 1);
            ConPutBox(I + 1, H, 1, 1, B);
        }
    }
    if (ShowVScroll && ShowHScroll) {
        MoveCh(B, ' ', hcScrollBar_Arrows, 1);
        ConPutBox(W, H, 1, 1, B);
    }

    return 0;
}


///////////////////////////////////////////////////////////////////////////

GView::GView(GFrame *parent, int XSize, int YSize) {
    Parent = parent;
    Prev = Next = 0;
    Peer = new GViewPeer(this, XSize, YSize);
    if (Parent)
        Parent->AddView(this);
}

GView::~GView() {
    if (Parent)
        Parent->RemoveView(this);
    delete Peer;
}

int GView::ConClear() {
    int W, H;
    TDrawBuffer B;

    ConQuerySize(&W, &H);
    MoveChar(B, 0, W, ' ', 0x07, 1);
    ConSetBox(0, 0, W, H, B[0]);
    return 1;
}

int GView::ConPutBox(int X, int Y, int W, int H, PCell Cell) {
    return Peer->ConPutBox(X, Y, W, H, Cell);
}

int GView::ConGetBox(int X, int Y, int W, int H, PCell Cell) {
    return Peer->ConGetBox(X, Y, W, H, Cell);
}

int GView::ConPutLine(int X, int Y, int W, int H, PCell Cell) {
    return Peer->ConPutLine(X, Y, W, H, Cell);
}

int GView::ConSetBox(int X, int Y, int W, int H, TCell Cell) {
    return Peer->ConSetBox(X, Y, W, H, Cell);
}

int GView::ConScroll(int Way, int X, int Y, int W, int H, TAttr Fill, int Count) {
    return Peer->ConScroll(Way, X, Y, W, H, Fill, Count);
}

int GView::ConSetSize(int X, int Y) {
    if (Peer->ConSetSize(X, Y))
        Resize(X, Y);
    else
        return 0;
    return 1;
}

int GView::ConQuerySize(int *X, int *Y) {
    return Peer->ConQuerySize(X, Y);
}

int GView::ConSetCursorPos(int X, int Y) {
    return Peer->ConSetCursorPos(X, Y);
}

int GView::ConQueryCursorPos(int *X, int *Y) {
    return Peer->ConQueryCursorPos(X, Y);
}

int GView::ConShowCursor() {
    return Peer->ConShowCursor();
}

int GView::ConHideCursor() {
    return Peer->ConHideCursor();
}

int GView::ConCursorVisible() {
    return Peer->ConCursorVisible();
}

void GView::ConSetInsertState(bool insert) {
    Peer->ConSetInsertState(insert);
}

int GView::CaptureMouse(int grab) {
    return Peer->CaptureMouse(grab);
}

int GView::CaptureFocus(int grab) {
    return Peer->CaptureFocus(grab);
}

int GView::QuerySbVPos() {
    return Peer->QuerySbVPos();
}

int GView::SetSbVPos(int Start, int Amount, int Total) {
    return Peer->SetSbVPos(Start, Amount, Total);
}

int GView::SetSbHPos(int Start, int Amount, int Total) {
    return Peer->SetSbHPos(Start, Amount, Total);
}

int GView::ExpandHeight(int DeltaY) {
    return Peer->ExpandHeight(DeltaY);
}

void GView::Update() {
}

void GView::Repaint() {
}

void GView::HandleEvent(TEvent &/*Event*/) {
}

void GView::Resize(int /*width*/, int /*height*/) {
    Repaint();
}

void GView::EndExec(int NewResult) {
    Result = NewResult;
}

int GView::Execute() {
    int SaveRc = Result;
    int NewResult;
    int didFocus = 0;

    if (FocusCapture == 0) {
        if (CaptureFocus(1) == 0) return -1;
        didFocus = 1;
    } else
        if (FocusCapture != this)
            return -1;
    Result = -2;
    while (Result == -2 && frames != 0)
        gui->ProcessEvent();
    NewResult = Result;
    Result = SaveRc;
    if (didFocus)
        CaptureFocus(0);
    return NewResult;
}

int GView::IsActive() {
    return (Parent->Active == this);
}

void GView::Activate(int gotfocus) {
    if (gotfocus) {
        Peer->wState |= sfFocus;
        Peer->UpdateCursor();
    } else {
        Peer->wState &= ~sfFocus;
    }
    Repaint();
}

///////////////////////////////////////////////////////////////////////////

GFramePeer::GFramePeer(GFrame *aFrame, int Width, int Height) {
    Frame = aFrame;
    if (Width != -1 && Height != -1)
        ConSetSize(Width, Height);
    ConQuerySize(&fW, &fH);
}

GFramePeer::~GFramePeer() {
}

int GFramePeer::ConSetSize(int X, int Y) {
    return ::ConSetSize(X, Y);
}

int GFramePeer::ConQuerySize(int *X, int *Y) {
    ::ConQuerySize(&fW, &fH);
    if (X) *X = fW;
    if (Y) *Y = fH;
    return 1;
}

//int GFrame::ConQuerySize(int *X, int *Y) {
//    ::ConQuerySize(X, Y);
//    if (ShowVScroll)
//        --*X;
//}

int GFramePeer::ConSetTitle(char *Title, char *STitle) {
    ::ConSetTitle(Title, STitle);
    return 0;
}

int GFramePeer::ConGetTitle(char *Title, int MaxLen, char *STitle, int SMaxLen) {
    return ::ConGetTitle(Title, MaxLen, STitle, SMaxLen);
}

///////////////////////////////////////////////////////////////////////////

GFrame::GFrame(int XSize, int YSize) {
    Menu = 0;
    if (frames == 0) {
        frames = Prev = Next = this;
    } else {
        Next = frames->Next;
        Prev = frames;
        frames->Next->Prev = this;
        frames->Next = this;
        frames = this;
    }
    Top = Active = 0;
    Peer = new GFramePeer(this, XSize, YSize);
}

GFrame::~GFrame() {
    if (Peer) {
        delete Peer;
        Peer = 0;
    }
    if (Next == this) {
        frames = 0;
//        printf("No more frames\x7\x7\n");
    } else {
        Next->Prev = Prev;
        Prev->Next = Next;
        frames = Next;
    }
    Next = Prev = 0;
    if (Menu)
        free(Menu);
}

int GFrame::ConSetTitle(char *Title, char *STitle) {
    return Peer->ConSetTitle(Title, STitle);
}

int GFrame::ConGetTitle(char *Title, int MaxLen, char *STitle, int SMaxLen) {
    return Peer->ConGetTitle(Title, MaxLen, STitle, SMaxLen);
}

int GFrame::ConSetSize(int X, int Y) {
    return Peer->ConSetSize(X, Y);
}

int GFrame::ConQuerySize(int *X, int *Y) {
    return Peer->ConQuerySize(X, Y);
}

int GFrame::ConSplitView(GView *view, GView *newview) {
    int dmy;

    newview->Parent = this;
    newview->Peer->wX = 0;
    ConQuerySize(&newview->Peer->wW, &dmy);
    if (ShowVScroll)
        newview->Peer->wW--;
    newview->Peer->wY = view->Peer->wY + view->Peer->wH / 2;
    newview->Peer->wH = view->Peer->wH - view->Peer->wH / 2;
    if (ShowHScroll) {
        newview->Peer->wY++;
        newview->Peer->wH--;
    }
    view->Peer->wH /= 2;
    view->ConSetSize(view->Peer->wW, view->Peer->wH);
    newview->ConSetSize(newview->Peer->wW, newview->Peer->wH);
    InsertView(view, newview);
    return 0;
}

int GFrame::ConCloseView(GView * /*view*/) {
    return 0;
}

int GFrame::ConResizeView(GView * /*view*/, int /*DeltaY*/) {
    return 0;
}

int GFrame::AddView(GView *view) {
    if (Active != 0) {
        return ConSplitView(Active, view);
    } else {
        int W, H;

        view->Parent = this;
        view->Prev = view->Next = 0;

        view->Peer->wX = 0;
        if (ShowMenuBar)
            view->Peer->wY = 1;
        else
            view->Peer->wY = 0;
        ConQuerySize(&W, &H);
        if (ShowMenuBar)
            H--;
        if (ShowVScroll)
            W--;
        if (ShowHScroll)
            H--;
        view->ConSetSize(W, H);
        InsertView(Top, view);
        return 0;
    }
}

void GFrame::Update() {
    GView *v = Active;

    UpdateMenu();
    while (v) {
        v->Update();
        if ((ShowVScroll || ShowHScroll) && (v->Peer->sbVupdate || v->Peer->sbHupdate)) {
            v->Peer->DrawScrollBar();
            v->Peer->sbVupdate = 0;
            v->Peer->sbHupdate = 0;
        }
        v = v->Next;
        if (v == Active)
            break;
    }
}

void GFrame::UpdateMenu() {
    if (ShowMenuBar)
        DrawMenuBar();
}

void GFrame::Repaint() {
    GView *v = Active;

    if (ShowMenuBar)
        DrawMenuBar();
    while (v) {
        v->Repaint();
        if (ShowVScroll || ShowHScroll) {
            v->Peer->DrawScrollBar();
            v->Peer->sbVupdate = 0;
            v->Peer->sbHupdate = 0;
        }
        v = v->Next;
        if (v == Active)
            break;
    }
}

void GFrame::InsertView(GView *prev, GView *view) {
    if (!view) return ;
    if (prev) {
        view->Prev = prev;
        view->Next = prev->Next;
        prev->Next = view;
        view->Next->Prev = view;
    } else {
        view->Prev = view->Next = view;
        Top = view;
    }
    if (Active == 0) {
        Active = view;
        Active->Activate(1);
    }
}

void GFrame::RemoveView(GView *view) {
    if (!view) return ;

    if (Active == view)
        Active->Activate(0);
    if (view->Next == view) {
        Top = Active = 0;
        delete this;
    } else {
        view->Next->Prev = view->Prev;
        view->Prev->Next = view->Next;

        if (Top == view) {
            Top = view->Next;
            Top->Peer->wY -= view->Peer->wH;
            Top->ConSetSize(Top->Peer->wW, Top->Peer->wH + view->Peer->wH + (ShowHScroll ? 1 : 0));
        } else {
            view->Prev->ConSetSize(view->Prev->Peer->wW,
                                   view->Prev->Peer->wH + view->Peer->wH + (ShowHScroll ? 1 : 0));
        }

        if (Active == view) {
            Active = view->Prev;
            Active->Activate(1);
        }
    }
}

void GFrame::SelectNext(int back) {
    GView *c = Active;

    if (c == 0 && Top == 0)
        return;

    if (FocusCapture != 0)
        return ;

    else if (c == 0)
        c = Active = Top;
    else
        if (back) {
            Active = Active->Prev;
        } else {
            Active = Active->Next;
        }
    if (c != Active) {
        if (c)
            c->Activate(0);
        if (Active)
            Active->Activate(1);
    }
}

int GFrame::SelectView(GView *view) {
    if (Top == 0)
        return 0;

    if (FocusCapture != 0)
        view = view;

    if (Active)
        Active->Activate(0);
    Active = view;
    if (Active)
        Active->Activate(1);
    return 1;
}

void GFrame::Resize(int width, int height) {
    GView *V;
    int count = 0;


    V = Top;
    while (V) {
        count++;
        if (V == Top) break;
    }
    if (height < 2 * count + 2 || width < 16) {
        ::ConSetSize(16, 2 * count + 1);
        return;
    }

    if (!Top)
        return;

    if (ShowVScroll)
        width--;
    if (ShowHScroll)
        height--;

//    fprintf(stderr, "Resize: %d %d\n", width, height);

    V = Top->Prev;

    while (V != Top) {
        int h, y;

        h = V->Peer->wH;
        y = V->Peer->wY;

        if (y >= height - 2) {
            y = height - 2;
        }
        if (y + h != height) {
            h = height - y;
        }
        V->Peer->wY = y;
        V->ConSetSize(width, h);
        height = y;
        V = V->Prev;
    }
    if (ShowMenuBar)
        height--;
    Top->ConSetSize(width, height);
    Repaint();
    //  fprintf(stderr, "Resize: %d %d Done\n", width, height);
}

int GFrame::ExecMainMenu(char Sub) {
    NextEvent.What = evCommand;
    NextEvent.Msg.Command = cmMainMenu;
    NextEvent.Msg.Param1 = Sub;
    return 0;
}

int GFrame::SetMenu(const char *Name) {
    if (Menu) free(Menu);
    Menu = strdup(Name);
    return 0;
}

void GFrame::Show() {
}

void GFrame::Activate() {
    frames = this;
}

int GUI::ConGrabEvents(TEventMask /*EventMask*/) {
    return 0;
}

void GUI::DispatchEvent(GFrame * /*frame*/, GView *view, TEvent &Event) {
    if (Event.What != evNone) {
        if (view)
            view->HandleEvent(Event);
    }
}

int GUI::ConGetEvent(TEventMask EventMask, TEvent *Event, int WaitTime, int Delete, GView **view) {
    if (view)
        *view = 0;
    return ::ConGetEvent(EventMask, Event, WaitTime, Delete);
}

int GUI::ConPutEvent(TEvent Event) {
    return ::ConPutEvent(Event);
}

int GUI::ConFlush(void) {
    return 0;
}

static inline int scrollBreak(TEvent &E) {
    return (E.What == evMouseUp);
}

static void HandleVScroll(GView *view, TEvent &E) {
    int y; //, x
    int wY, wH;
    TEvent E1;

    //x = E.Mouse.X;
    y = E.Mouse.Y;
    wY = view->Peer->wY;
    wH = view->Peer->wH;
    if (y == wY) {
        do {
            E1.What = evCommand;
            E1.Msg.View = view;
            E1.Msg.Command = cmVScrollUp;
            E1.Msg.Param1 = 1;
            gui->DispatchEvent(frames, view, E1);
            frames->Update();
            do {
                ConGetEvent(evMouse | evNotify, &E, -1, 1);
                if (E.What & evNotify)
                    gui->DispatchEvent(frames, view, E);
            } while (E.What & evNotify);
            if (scrollBreak(E)) break;
        } while (1);
    } else if (y == wY + wH - 1) {
        do {
            E1.What = evCommand;
            E1.Msg.View = view;
            E1.Msg.Command = cmVScrollDown;
            E1.Msg.Param1 = 1;
            gui->DispatchEvent(frames, view, E1);
            frames->Update();
            do {
                ConGetEvent(evMouse | evNotify, &E, -1, 1);
                if (E.What & evNotify)
                    gui->DispatchEvent(frames, view, E);
            } while (E.What & evNotify);
            if (scrollBreak(E)) break;
        } while (1);
    } else if (y < wY + view->Peer->SbVBegin + 1) {
        do {
            E1.What = evCommand;
            E1.Msg.View = view;
            E1.Msg.Command = cmVScrollPgUp;
            gui->DispatchEvent(frames, view, E1);
            frames->Update();
            do {
                ConGetEvent(evMouse | evNotify, &E, -1, 1);
                if (E.What & evNotify)
                    gui->DispatchEvent(frames, view, E);
            } while (E.What & evNotify);
            if (scrollBreak(E)) break;
        } while (1);
    } else if (y > wY + view->Peer->SbVEnd + 1) {
        do {
            E1.What = evCommand;
            E1.Msg.View = view;
            E1.Msg.Command = cmVScrollPgDn;
            gui->DispatchEvent(frames, view, E1);
            frames->Update();
            do {
                ConGetEvent(evMouse | evNotify, &E, -1, 1);
                if (E.What & evNotify)
                    gui->DispatchEvent(frames, view, E);
            } while (E.What & evNotify);
            if (scrollBreak(E)) break;
        } while (1);
    } else {
        int delta = y - 1 - view->Peer->SbVBegin - wY;

        do {
            E1.What = evCommand;
            E1.Msg.View = view;
            E1.Msg.Command = cmVScrollMove;
            E1.Msg.Param1 = (E.Mouse.Y - wY - 1 - delta + 1) * view->Peer->sbVtotal / (wH - 2);
//            printf("YPos = %d %d %d \n\x7", E.Mouse.Y, wY, delta);
            gui->DispatchEvent(frames, view, E1);
            frames->Update();
            do {
                ConGetEvent(evMouse | evNotify, &E, -1, 1);
                if (E.What & evNotify)
                    gui->DispatchEvent(frames, view, E);
            } while (E.What & evNotify);
            if (scrollBreak(E)) break;
        } while (1);
    }
    E.What = evNone;
}

static void HandleHScroll(GView *view, TEvent &E) {
    int x; //, x
    int wX, wW;
    TEvent E1;

    //x = E.Mouse.X;
    x = E.Mouse.X;
    wX = view->Peer->wX;
    wW = view->Peer->wW;
    if (x == wX) {
        do {
            E1.What = evCommand;
            E1.Msg.View = view;
            E1.Msg.Command = cmHScrollLeft;
            E1.Msg.Param1 = 1;
            gui->DispatchEvent(frames, view, E1);
            frames->Update();
            do {
                ConGetEvent(evMouse | evNotify, &E, -1, 1);
                if (E.What & evNotify)
                    gui->DispatchEvent(frames, view, E);
            } while (E.What & evNotify);
            if (scrollBreak(E)) break;
        } while (1);
    } else if (x == wX + wW - 1) {
        do {
            E1.What = evCommand;
            E1.Msg.View = view;
            E1.Msg.Command = cmHScrollRight;
            E1.Msg.Param1 = 1;
            gui->DispatchEvent(frames, view, E1);
            frames->Update();
            do {
                ConGetEvent(evMouse | evNotify, &E, -1, 1);
                if (E.What & evNotify)
                    gui->DispatchEvent(frames, view, E);
            } while (E.What & evNotify);
            if (scrollBreak(E)) break;
        } while (1);
    } else if (x < wX + view->Peer->SbHBegin + 1) {
        do {
            E1.What = evCommand;
            E1.Msg.View = view;
            E1.Msg.Command = cmHScrollPgLt;
            gui->DispatchEvent(frames, view, E1);
            frames->Update();
            do {
                ConGetEvent(evMouse | evNotify, &E, -1, 1);
                if (E.What & evNotify)
                    gui->DispatchEvent(frames, view, E);
            } while (E.What & evNotify);
            if (scrollBreak(E)) break;
        } while (1);
    } else if (x > wX + view->Peer->SbHEnd + 1) {
        do {
            E1.What = evCommand;
            E1.Msg.View = view;
            E1.Msg.Command = cmHScrollPgRt;
            gui->DispatchEvent(frames, view, E1);
            frames->Update();
            do {
                ConGetEvent(evMouse | evNotify, &E, -1, 1);
                if (E.What & evNotify)
                    gui->DispatchEvent(frames, view, E);
            } while (E.What & evNotify);
            if (scrollBreak(E)) break;
        } while (1);
    } else {
        int delta = x - 1 - view->Peer->SbHBegin - wX;

        do {
            E1.What = evCommand;
            E1.Msg.View = view;
            E1.Msg.Command = cmHScrollMove;
            E1.Msg.Param1 = (E.Mouse.X - wX - 1 - delta + 1) * view->Peer->sbHtotal / (wW - 2);
//            printf("YPos = %d %d %d \n\x7", E.Mouse.Y, wY, delta);
            gui->DispatchEvent(frames, view, E1);
            frames->Update();
            do {
                ConGetEvent(evMouse | evNotify, &E, -1, 1);
                if (E.What & evNotify)
                    gui->DispatchEvent(frames, view, E);
            } while (E.What & evNotify);
            if (scrollBreak(E)) break;
        } while (1);
    }
    E.What = evNone;
}

void GUI::ProcessEvent() {
    TEvent E;

    E = NextEvent;
    if (E.What != evNone) {
        NextEvent.What = evNone;
    }
    if (E.What == evNone &&
            (ConGetEvent(evMouse | evCommand | evKeyboard, &E, 0, 1, 0) == -1 ||
             E.What == evNone)
       ) {
        frames->Update();
        while (ConGetEvent(evMouse | evCommand | evKeyboard, &E, -1, 1, 0) == -1 ||
                (E.What == evMouseMove && E.Mouse.Buttons == 0));
    }
    if (E.What != evNone) {
        GView *view = frames->Active;

        if (E.What & evMouse) {
            if (E.What == evMouseDown && E.Mouse.Y == 0 && ShowMenuBar &&
                    MouseCapture == 0 && FocusCapture == 0) {
                frames->Update(); // sync before menu
                if (ExecMainMenu(E, 0) == -1) {
                    if (E.What == evCommand && E.Msg.Command == cmResize) {
                        int X, Y;

                        ConQuerySize(&X, &Y);
                        frames->Resize(X, Y);
                    }
                    E.What = evNone;
                }
//                fprintf(stderr, "Command got = %d\n", E.Msg.Command);
            }
            if (E.What == evMouseDown && MouseCapture == 0 && FocusCapture == 0) {
                GView *V = frames->Active;

                while (V) {
                    if (E.Mouse.Y >= V->Peer->wY &&
                            E.Mouse.Y <  V->Peer->wY + V->Peer->wH + (ShowHScroll ? 1 : 0)) {
                        frames->SelectView(V);
                        view = V;
                        break;
                    }
                    V = V->Next;
                    if (V == frames->Active)
                        break;
                }
            }
            if (ShowVScroll && ShowHScroll && E.What == evMouseDown &&
                    MouseCapture == 0 && FocusCapture == 0 &&
                    E.Mouse.Y == view->Peer->wY + view->Peer->wH &&
                    E.Mouse.X == view->Peer->wX + view->Peer->wW) {
            } else {
                if (ShowVScroll && E.What == evMouseDown && MouseCapture == 0 && FocusCapture == 0 &&
                        E.Mouse.X == view->Peer->wX + view->Peer->wW) {
                    HandleVScroll(view, E);
                    return ;
                }
                if (ShowHScroll && E.What == evMouseDown && MouseCapture == 0 && FocusCapture == 0 &&
                        E.Mouse.Y == view->Peer->wY + view->Peer->wH) {
                    HandleHScroll(view, E);
                    return ;
                }
            }
            if (E.What & evMouse) {
                E.Mouse.Y -= view->Peer->wY;
                E.Mouse.X -= view->Peer->wX;
            }
        }
        if (E.What == evCommand) {
            switch (E.Msg.Command) {
            case cmResize: {
                int X, Y;

                ConQuerySize(&X, &Y);
                frames->Resize(X, Y);
            }
            break;
            case cmMainMenu: {
                char Sub = (char)E.Msg.Param1;

                frames->Update(); // sync before menu
                if (::ExecMainMenu(E, Sub) != 1) {
                    ;
                    if (E.What == evCommand && E.Msg.Command == cmResize) {
                        int X, Y;

                        ConQuerySize(&X, &Y);
                        frames->Resize(X, Y);
                    }
                    E.What = evNone;
                }
            }
            break;
            case cmPopupMenu: {
                int id = E.Msg.Param1;
                int Cols, Rows;

                if (id == -1) return;
                frames->ConQuerySize(&Cols, &Rows);
                int x = Cols / 2, y = Rows / 2;
                ConQueryMousePos(&x, &y);

                frames->Update(); // sync before menu
                if (::ExecVertMenu(x, y, id, E, 0) != 1) {
                    if (E.What == evCommand && E.Msg.Command == cmResize) {
                        int X, Y;

                        ConQuerySize(&X, &Y);
                        frames->Resize(X, Y);
                    }
                    E.What = evNone;
                }
            }
            break;
            }
        }
        if (E.What != evNone)
            DispatchEvent(frames, view, E);
    }
}

// Temporary hack: This function has different implementation
// for the experimental Cocoa version
#ifndef COMPILING_FOR_COCOA
int GUI::Run() {
    if (Start(fArgc, fArgv) == 0) {
        doLoop = 1;
        while (doLoop)
            ProcessEvent();
        Stop();
        return 0;
    }
    return 1;
}
#endif // !COMPILING_FOR_COCOA

int GUI::multiFrame() {
    return 0;
}

void DieError(int rc, const char *msg, ...) {
    va_list ap;

    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    exit(rc);
}
