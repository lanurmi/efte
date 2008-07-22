/*
 * u_stack.h
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 */

#define PSCHECK(x,f) if (x < 0) {\
    ActiveView->Msg(S_ERROR, "Attempt to access negative integer stack index in " f); \
    SetBranchCondition(0);\
    return 0; \
    } else if (ParamStack.size() < x) { \
    ActiveView->Msg(S_ERROR, "Stack underflow in " f " (%i)", ParamStack.size()); \
    SetBranchCondition(0); \
    return 0; \
    }
#define SSCHECK(x,f) if (x < 0) {\
    ActiveView->Msg(S_ERROR, "Attempt to access negative string stack index in " f); \
    SetBranchCondition(0);\
    return 0;\
    } else if (sstack.size() < x) { \
    ActiveView->Msg(S_ERROR, "String stack underflow in " f " (%i)", sstack.size()); \
    SetBranchCondition(0); \
    return 0; \
    }
#define CSCHECK(x,f) // No checking yet

#ifndef U_CIRCSTACK_H
#define U_CIRCSTACK_H
// CircularStack size must be 2**n - required by the fast wrap used here.
#define CIRCSTACKSIZE 128

#include <vector>
#include <string>

extern std::vector<std::string> sstack;
extern std::vector<int> memory;

class CircularStack {
private:
    int stack[CIRCSTACKSIZE];
    int pos;
    int stackdepth;

public:
    CircularStack();
    void empty();
    void push(int integer);
    int pop();
    int peek(int offset=0);
    void dup();
    void swap();
    int size();
};

#endif /* U_CIRCSTACK_H */


#ifndef U_STACK_H
#define U_STACK_H

#define STACKSIZE 128

class Stack {
private:
    int stack[STACKSIZE];
    int pos;

public:
    Stack();
    void init();
    void empty();
    void push(int integer);
    int pop();
    int peek(int offset=0);
    void dup();
    int size();
};

#endif /* U_STACK_H */

