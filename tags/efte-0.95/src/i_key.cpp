/*    i_key.cpp
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"

ExKey::ExKey(const char *APrompt): ExView() {
    if (APrompt)
        Prompt = strdup(APrompt);
    else
        Prompt = 0;
}

ExKey::~ExKey() {
    free(Prompt);
}

void ExKey::Activate(int gotfocus) {
    ExView::Activate(gotfocus);
}

int ExKey::BeginMacro() {
    return 1;
}

void ExKey::HandleEvent(TEvent &Event) {
    switch (Event.What) {
    case evKeyDown:
        Key = Event.Key.Code;
        if (!(Key & kfModifier)) // not ctrl,alt,shift, ....
            EndExec(1);
        Event.What = evNone;
        break;
    }
}

void ExKey::UpdateView() {
    if (Next) {
        Next->UpdateView();
    }
}

void ExKey::RepaintView() {
    if (Next) {
        Next->RepaintView();
    }
}

void ExKey::UpdateStatus() {
    RepaintStatus();
}

void ExKey::RepaintStatus() {
    TDrawBuffer B;
    int W, H;

    ConQuerySize(&W, &H);

    MoveCh(B, ' ', 0x17, W);
    ConPutBox(0, H - 1, W, 1, B);
}
