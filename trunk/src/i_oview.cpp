/*    i_oview.cpp
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"

ExView::ExView() {
}

ExView::~ExView() {
}

void ExView::Activate(int /*gotfocus*/) {
}

int ExView::IsActive() {
    if (Win)
        return Win->IsActive();
    return 0;
}

EEventMap *ExView::GetEventMap() {
    return 0;
}

int ExView::ExecCommand(int /*Command*/, ExState &/*State*/) {
    return ErFAIL;
}

int ExView::BeginMacro() {
    return 1;
}

void ExView::HandleEvent(TEvent &Event) {
    if (Event.What == evKeyDown && kbCode(Event.Key.Code) == kbF12)
        Win->Parent->SelectNext(0);
}

void ExView::EndExec(int NewResult) {
    if (Win->Result == -2) { // hack
        Win->EndExec(NewResult);
    } else {
        if (Next) {
            delete Win->PopView(); // self
        }
    }
}

void ExView::UpdateView() {
}

void ExView::UpdateStatus() {
}

void ExView::RepaintView() {
}

void ExView::RepaintStatus() {
}

void ExView::Resize(int /*width*/, int /*height*/) {
    Repaint();
}

int ExView::ConPutBox(int X, int Y, int W, int H, PCell Cell) {
    if (Win)
        return Win->ConPutBox(X, Y, W, H, Cell);
    return -1;
}

int ExView::ConScroll(int Way, int X, int Y, int W, int H, TAttr Fill, int Count) {
    if (Win)
        return Win->ConScroll(Way, X, Y, W, H, Fill, Count);
    return -1;
}

int ExView::ConQuerySize(int *X, int *Y) {
    if (Win)
        return Win->ConQuerySize(X, Y);
    return -1;
}

int ExView::ConSetCursorPos(int X, int Y) {
    if (Win)
        return Win->ConSetCursorPos(X, Y);
    return -1;
}

int ExView::ConShowCursor() {
    if (Win)
        return Win->ConShowCursor();
    return -1;
}

int ExView::ConHideCursor() {
    if (Win)
        return Win->ConHideCursor();
    return -1;
}

void ExView::ConSetInsertState(bool insert) {
    if (Win)
        Win->ConSetInsertState(insert);
}
