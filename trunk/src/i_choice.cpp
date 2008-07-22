/*    i_choice.cpp
 *
 *    Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"

ExChoice::ExChoice(const char *ATitle, int NSel, va_list ap): ExView() {
    char msg[1024];
    int i;
    char *fmt;

    Cur = 0;
    MouseCaptured = 0;

    Title = strdup(ATitle);
    lTitle = strlen(Title);
    NOpt = NSel;
    lChoice = 0;

    for (i = 0; i < NSel; i++) {
        SOpt[i] = strdup(va_arg(ap, char *));
        lChoice += CStrLen(SOpt[i]) + 1;
    }
    fmt = va_arg(ap, char *);
    vsprintf(msg, fmt, ap);
    strncpy(Prompt, msg, sizeof(Prompt));
    Prompt[sizeof(Prompt) - 1] = 0;
}

ExChoice::~ExChoice() {
    free(Title);
    for (int i = 0; i < NOpt; i++)
        free(SOpt[i]);
}

void ExChoice::Activate(int gotfocus) {
    ExView::Activate(gotfocus);
}

int ExChoice::BeginMacro() {
    return 1;
}

int ExChoice::FindChoiceByPoint(int x, int y) {
    int pos, i;
    int W, H;

    Win->ConQuerySize(&W, &H);

    if (y != H - 1)
        return -1;

    pos = W - lChoice;
    if (x < pos)
        return -1;

    for (i = 0; i < NOpt; i++) {
        int clen = CStrLen(SOpt[i]);

        if (x > pos && x <= pos + clen)
            return i;
        pos += clen + 1;
    }
    return -1;
}

void ExChoice::HandleEvent(TEvent &Event) {
    int i;

    switch (Event.What) {
    case evKeyDown:
        switch (kbCode(Event.Key.Code)) {
        case kbTab | kfShift:
        case kbLeft:
            if (Cur == -1) Cur = 0;
            Cur--;
            if (Cur < 0) Cur = NOpt - 1;
            Event.What = evNone;
            break;
        case kbTab:
        case kbRight:
            if (Cur == -1) Cur = 0;
            Cur++;
            if (Cur >= NOpt) Cur = 0;
            Event.What = evNone;
            break;
        case kbHome:
            Cur = 0;
            Event.What = evNone;
            break;
        case kbEnd:
            Cur = NOpt - 1;
            Event.What = evNone;
            break;
        case kbEnter:
            if (Cur >= 0 && NOpt > 0) EndExec(Cur);
            Event.What = evNone;
            break;
        case kbEsc:
            EndExec(-1);
            Event.What = evNone;
            break;
        default:
            if (isAscii(Event.Key.Code)) {
                char c = char(Event.Key.Code & 0xFF);
                char s[3];

                s[0] = '&';
                s[1] = (char)(toupper((char)c) & 0xFF);
                s[2] = 0;

                for (i = 0; i < NOpt; i++) {
                    if (strstr(SOpt[i], s) != 0) {
                        Win->EndExec(i);
                        break;
                    }
                }
                Event.What = evNone;
            }
            break;
        }
        break;
    case evMouseDown:
        if (Win->CaptureMouse(1))
            MouseCaptured = 1;
        else
            break;
        Cur = FindChoiceByPoint(Event.Mouse.X, Event.Mouse.Y);
        Event.What = evNone;
        break;
    case evMouseMove:
        if (MouseCaptured)
            Cur = FindChoiceByPoint(Event.Mouse.X, Event.Mouse.Y);
        Event.What = evNone;
        break;
    case evMouseUp:
        if (MouseCaptured)
            Win->CaptureMouse(0);
        else
            break;
        MouseCaptured = 0;
        Cur = FindChoiceByPoint(Event.Mouse.X, Event.Mouse.Y);
        Event.What = evNone;
        if (Cur >= 0 && Cur < NOpt && NOpt > 0)
            EndExec(Cur);
        else
            Cur = 0;
        break;
    }
}

void ExChoice::UpdateView() {
    if (Next) {
        Next->UpdateView();
    }
}

void ExChoice::RepaintView() {
    if (Next) {
        Next->RepaintView();
    }
}

void ExChoice::UpdateStatus() {
    RepaintStatus();
}

void ExChoice::RepaintStatus() {
    TDrawBuffer B;
    int W, H;
    int pos, i;
    TAttr color1, color2;

    ConQuerySize(&W, &H);


    if (Cur != -1) {
        if (Cur >= NOpt) Cur = NOpt - 1;
        if (Cur < 0) Cur = 0;
    }

    MoveCh(B, ' ', hcChoice_Background, W);
    MoveStr(B, 0, W, Title, hcChoice_Title, W);
    MoveChar(B, lTitle, W, ':', hcChoice_Background, 1);
    MoveStr(B, lTitle + 2, W, Prompt, hcChoice_Param, W);

    pos = W - lChoice;
    for (i = 0; i < NOpt; i++) {
        if (i == Cur) {
            color1 = hcChoice_ActiveItem;
            color2 = hcChoice_ActiveChar;
        } else {
            color1 = hcChoice_NormalItem;
            color2 = hcChoice_NormalChar;
        }
        if (i == Cur)
            ConSetCursorPos(pos + 1, H - 1);
        MoveChar(B, pos, W, ConGetDrawChar(DCH_V), hcChoice_Background, 1);
        MoveCStr(B, pos + 1, W, SOpt[i], color1, color2, W);
        pos += CStrLen(SOpt[i]) + 1;
    }
    ConPutBox(0, H - 1, W, 1, B);
}
