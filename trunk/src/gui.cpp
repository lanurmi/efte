/*
 * gui.cpp
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "console.h"
#include "gui.h"

int GFrame::isLastFrame() {
    if (this == Next && frames == this)
        return 1;
    else
        return 0;
}

void GUI::deleteFrame(GFrame *frame) {
    if (frame->isLastFrame()) {
        delete frame;
        frames = 0;
    } else {
        //frame->Prev->Next = frame->Next;
        //frame->Next->Prev = frame->Prev;
        //if (frames == frame)
        //    frames = frame->Next;

        //frames->Activate();
        delete frame;
    }
}

int GUI::Start(int &/*argc*/, char ** /*argv*/) {
    return 0;
}

void GUI::Stop() {
}

void GUI::StopLoop() {
    doLoop = 0;
}
