/*
 * u_stack.h
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 */

#ifndef U_CIRCSTACK_H
#define U_CIRCSTACK_H
// CircularStack size must be 2**n - required by the fast wrap used here.
#define CIRCSTACKSIZE 32

#include <vector>
#include <string>

extern std::vector<std::string> sstack;

class CircularStack {
private:
    int stack[CIRCSTACKSIZE];
    int pos;
    int stackdepth;

public:
    CircularStack();
    void push(int integer);
    int pop();
    int peek(int offset=0);
    void dup();
    void swap();
    int depth();
};

#endif /* U_CIRCSTACK_H */


#ifndef U_STACK_H
#define U_STACK_H

#define STACKSIZE 32

class Stack {
private:
    int stack[STACKSIZE];
    int pos;

public:
    Stack();
    void init();
    void push(int integer);
    int pop();
    int peek(int offset=0);
    void dup();
    int depth();
};

#endif /* U_STACK_H */
