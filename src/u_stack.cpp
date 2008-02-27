/*
 * u_stack.cpp
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 */

#include "u_stack.h"
#define STACKMASK (STACKSIZE-1)

std::vector<std::string> sstack;

// CircularStack is used for macro data stack, which is used by macros - including user written macros -
// as data working and storage space. because there is no way of statically knowing how many times macros
// will be executed, and how many stack items they leave on stack, and how much care the writer of the
// macro takes to keep the stack balanced, it is expressed as a circular stack which can never overflow.
// Control Stack in macro space will be of the same type.
// this keeps the stack from gobbling up more and more memory, creating the impression of a memory leak.
// But just as we can't really overflow a CircularStack, detection of stack underflow is not easily
// accomplished, but needs extra provision implemented here.

CircularStack::CircularStack() {
    pos = -1;
    stackdepth = 0;
    for (int i=0; i < STACKSIZE; i++)
        stack[i] = 0;
}

void CircularStack::push(int integer) {
    pos = (pos + 1) & STACKMASK;
    stack[pos] = integer;
    if (stackdepth < STACKMASK)
        stackdepth++;
}

int CircularStack::pop() {
    int r = 0;
    r = stack[pos];
    stack[pos] = 0;
    pos = (pos - 1) & STACKMASK;
    stackdepth--;
    return r;
}

int CircularStack::peek(int offset) {
    int p = (pos - offset) & STACKMASK;
    return stack[p];
}

void CircularStack::dup() {
    int p = pos;
    pos = (pos + 1) & STACKMASK;
    stack[pos] = stack[p];
    stackdepth++;
}

void CircularStack::swap() {
    int p1 = (pos);
    int p2 = (pos - 1) & STACKMASK;
    stack[p1] ^= stack[p2];
    stack[p2] ^= stack[p1];
    stack[p1] ^= stack[p2];
}

int CircularStack::size() {
    return stackdepth;
}

// Where the danger of continuously adding to stack by faulty macros is not given, and we want a test
// for stack emptyness, a non-wrapping stack is used. cefte/cfte macro compiler uses (should use) this
// type of stack for tracking flow control branch offsets.

Stack::Stack() {
    pos = 0;
    for (int i=0; i < STACKSIZE; i++)
        stack[i] = 0;
}

void Stack::init() {
    pos = 0;
}

void Stack::push(int stackitem) {
    if (pos < STACKSIZE) {
        stack[pos] = stackitem;
        pos = (pos + 1);
        //} else {
        //fatal: stack overflow
    }
}

int Stack::pop() {
    if (pos) {
        pos = (pos - 1);
        int r = stack[pos];
        return r;
    } else {
        //        "error: stack underflow attempt"
        return 0;    // may not be what one expects but i have no idea how to deal with exceptions here
        // throw(stack_underflow);  // like this, maybe?
    }
}

int Stack::peek(int offset) {
    if (offset < pos) {
        int p = (pos - offset - 1);
        return stack[p];
    } else {
        // error: attempt to access stack below bottom
        return 0;
    }
}

void Stack::dup() {
    if (pos) {
        stack[pos] = stack[(pos - 1)];
    } else {
        stack[pos] = 0;
    }
    pos = (pos + 1);
}

int Stack::size() {
    return pos;
}
