/*    i_modelview.cpp
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"

ExModelView::ExModelView(EView *AView): ExView() {
    View = AView;
    View->MView = this;
    MouseCaptured = 0;
    MouseMoved = 0;
}

ExModelView::~ExModelView() {
    if (View) { // close it
        delete View;
        View = 0;
    }
}

int ExModelView::GetContext() {
    return View->GetContext();
}

void ExModelView::Activate(int gotfocus) {
    ExView::Activate(gotfocus);
    View->Activate(gotfocus);
}

EEventMap *ExModelView::GetEventMap() {
    return View->GetEventMap();
}

int ExModelView::ExecCommand(int Command, ExState &State) {
    return View->ExecCommand(Command, State);
}

int ExModelView::BeginMacro() {
    return View->BeginMacro();
}

void ExModelView::HandleEvent(TEvent &Event) {
    ExView::HandleEvent(Event);
    View->HandleEvent(Event);
}

void ExModelView::UpdateView() {
    View->UpdateView();
}

void ExModelView::RepaintView() {
    View->RepaintView();
}

void ExModelView::RepaintStatus() {
    View->RepaintStatus();
}

void ExModelView::UpdateStatus() {
    View->UpdateStatus();
}

void ExModelView::Resize(int width, int height) {
    View->Resize(width, height);
}

void ExModelView::WnSwitchBuffer(EModel *B) {
    if (View)
        View->SwitchToModel(B);
}
