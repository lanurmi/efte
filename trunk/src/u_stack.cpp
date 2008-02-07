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

// CircularStack is used for macro data stack, which is used by macros - including user written macros -
// as data working and storage space. because there is no way of statically knowing how many times macros
// will be executed, and how many stack items they leave on stack, and how much care the writer of the
// macro takes to keep the stack balanced, it is expressed as a circular stack which can never overflow.
// Control Stack in macro space will be of the same type.
// this keeps the stack from gobbling up more and more memory, creating the impression of a memory leak.
// But just as we can't really overflow a CircularStack, detection of stack underflow is not easily
// accomplished, but needs extra provision implemented here.

CircularStack::CircularStack() {
    this->pos = -1;
    this->stackdepth = 0;
    for (int i=0; i < STACKSIZE; i++)
        this->stack[i] = 0;
}

void CircularStack::push(int integer) {
    this->pos = (this->pos + 1) & STACKMASK;
    this->stack[this->pos] = integer;
//    if (this->stackdepth < STACKMASK)
//        this->stackdepth++;
}

int CircularStack::pop() {
//    if (this->stackdepth) {
        int r = this->stack[this->pos];
        this->stack[this->pos] = 0;
        this->pos = (this->pos - 1) & STACKMASK;
//        this->stackdepth--;
        return r;
//    } else {
//        fprintf(stderr, "attempt to underflow stack\n");
//        return 0;
//    }
}

int CircularStack::peek(int offset) {
    int p = (this->pos - offset) & STACKMASK;
//    if (offset < this->stackdepth) {
        return this->stack[p];
//    } else {
//        fprintf(stderr, "stack access attempted outside of stack boundaries\n");
//        return 0;
//    }
}

void CircularStack::dup() {
    int p = this->pos;
    this->pos = (this->pos + 1) & STACKMASK;
    this->stack[this->pos] = this->stack[p];
}

void CircularStack::swap() {
    int p1 = (this->pos);
    int p2 = (this->pos - 1) & STACKMASK;
    this->stack[p1] ^= this->stack[p2];
    this->stack[p2] ^= this->stack[p1];
    this->stack[p1] ^= this->stack[p2];
}

int CircularStack::depth() {
    return this->stackdepth;
}





// Where the danger of continuously adding to stack by faulty macros is not given, and we want a test
// for stack emptyness, a non-wrapping stack is used. cefte/cfte macro compiler uses (should use) this
// type of stack for tracking flow control branch offsets.

Stack::Stack() {
    this->pos = -1;
    for (int i=0; i < STACKSIZE; i++)
        this->stack[i] = 0;
}

void Stack::push(int integer) {
    if (this->pos+1 < STACKSIZE) {
        this->pos = (this->pos + 1);
        this->stack[this->pos] = integer;
//  } else {
//  fatal: stack overflow
    }

}

int Stack::pop() {
    if (pos) {
        int r = this->stack[this->pos];
        this->pos = (this->pos - 1);
        return r;
    } else {
        //        "error: stack underflow attempt"
        return 0;    // may not be what one expects but i have no idea how to deal with exceptions here
// throw(stack_underflow);  // like this, maybe?
    }
}

int Stack::peek(int offset) {
    if (offset <= this->pos) {
        int p = (this->pos - offset);
        return this->stack[p];
    } else {
        // error: attempt to access stack below bottom
        return 0;
    }
}


int Stack::depth() {
    return this->pos;
}
