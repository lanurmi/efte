/* w_number.cpp
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 */

#include "fte.h"
#include "throw.h"

int Print(GxView *view, ExState &State) {
    PSCHECK(1, "Print");
    SSCHECK(1, "Print");

    int whereTo = ParamStack.pop();
    FILE *f = whereTo == 2 ? stderr : stdout;

    fprintf(f, sstack.back().c_str()); sstack.pop_back();

    return 1;
}


int EGUI::Diag(ExState &State) {
    fprintf(stderr, "Param stack: ");
    int i = ParamStack.size();
    if (i)
        for ( ; i; )
            fprintf(stderr, "%i ", ParamStack.peek(--i));
    else
        fprintf(stderr, "empty");

    if (memory[verbosity] <= 1)                // don't linefeed if verbosity >1:  command trace will advance.
        fprintf(stderr, "\n");

    return 1;
}

int ParamDepth() {
    ParamStack.push(ParamStack.size());
    return 1;
}

// --- arithmetic ---
int Plus() {
    PSCHECK(2, "Plus");
    ParamStack.push(ParamStack.pop()+ParamStack.pop());
    return 1;
}

int Minus() {
    PSCHECK(2, "Minus");
    int tos=ParamStack.pop();
    ParamStack.push(+ParamStack.pop()-tos);
    return 1;
}

int Mul() {
    PSCHECK(2, "Mul");
    ParamStack.push(ParamStack.pop()*ParamStack.pop());
    return 1;
}

int Div() {
    PSCHECK(2, "Div)");
    int tos=ParamStack.pop();

    if (!tos) {
        ActiveView->Msg(S_ERROR, "Divide by zero, macro aborted");
        exception = DIVZERO;
        SetBranchCondition(0);
        return 0;
    }

    ParamStack.push(ParamStack.pop()/tos);
    SetBranchCondition(1);
    return 1;
}

int Random() {
    ParamStack.push(random());
    return 1;
}

// --- bits ---

int And() {
    PSCHECK(2, "And");
    ParamStack.push(ParamStack.pop() & ParamStack.pop());
    return 1;
}

int Or() {
    PSCHECK(2, "Or");
    ParamStack.push(ParamStack.pop() | ParamStack.pop());
    return 1;
}

int Xor() {
    PSCHECK(2, "Xor");
    ParamStack.push(ParamStack.pop() ^ ParamStack.pop());
    return 1;
}

int Shift() {
    PSCHECK(2, "Shift");
    int tos = ParamStack.pop();                         // shift count and direction
    if (tos) {
        unsigned int nos = ParamStack.pop();            // shift value
        if (tos < 0)  {
            ParamStack.push(nos>>(-tos));
        } else {
            ParamStack.push(nos << tos);
        }
    }
    return 1;
}

// --- comparison ---

// replace top two stack items against identity flag
int Equals() {
    PSCHECK(2, "Equals");
    ParamStack.push(-(ParamStack.pop() == ParamStack.pop()));
    return 1;
}

// true if 2nd item less than top item:
//  3 4 Less   ( true )
int Less() {
    PSCHECK(2, "Less");
    ParamStack.push(-(ParamStack.pop() > ParamStack.pop()));
    return 1;
}

// interface condition, provided by old commands, to
// condition reading of new commands (passed on stack)
// old commands buffer their conditions, until read
// and transported to stack by "Flag"
// That way, the number of test results and conditions
// provided by old commands is irrelevant. we can choose
// to use or ignore as we see fit.
// as soon we need one of the last n result flags, each
// execution of Flag delivers the next, back into history.
int Flag() {
    // TODO: warning C4146: unary minus operator applied to unsigned type, result still unsigned
    ParamStack.push(-(int)(BranchCondition & 1));
    BranchCondition = (BranchCondition >> 1);
    return 1;
}

int Fail() {
    exception = ABORTED;
    return 0;
}

// --- stack ---

int Dup() {
    PSCHECK(1, "Dup");
    ParamStack.dup();
    return 1;
}

int Drop() {
    PSCHECK(1, "Drop");
    ParamStack.pop();
    return 1;
}

int Swap() {
    PSCHECK(2, "Swap");
    ParamStack.swap();
    return 1;
}

int Over() {
    PSCHECK(2, "Over");
    ParamStack.push(ParamStack.peek(1));
    return 1;
}

int Rot() {
    PSCHECK(3, "Rot");
    int tos = ParamStack.pop();
    ParamStack.swap();
    ParamStack.push(tos);
    ParamStack.swap();
    return 1;
}

// --- stack2 ---
int ToR() {
    PSCHECK(1, "ToR");
    ControlStack.push(ParamStack.pop());
    return 1;
}

int RFrom() {
    CSCHECK(1, "RFrom");
    ParamStack.push(ControlStack.pop());
    return 1;
}

int RFetch() {
    CSCHECK(1, "RFetch");
    ParamStack.push(ControlStack.peek(0));
    return 1;
}

int I() {
    CSCHECK(2, "I");
    ParamStack.push(ControlStack.peek(0));
    return 1;
}

int J() {
    CSCHECK(4, "J");
    ParamStack.push(ControlStack.peek(2));
    return 1;
}
