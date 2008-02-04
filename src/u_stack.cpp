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
//        fprintf(stderr, "stack access attempted below first deepest item\n");
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
