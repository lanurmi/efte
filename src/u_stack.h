/*
 * u_stack.h
 *
 * Copyright (c) 2008, eFTE SF Group (see AUTHORS file)
 *
 * You may distribute under the terms of either the GNU General Public
 * License or the Artistic License, as specified in the README file.
 *
 */

#ifndef U_STACK_H
#define U_STACK_H

#define STACKSIZE 16

class CircularStack {
private:
    int stack[STACKSIZE];
    int pos;

public:
    CircularStack();

    void push(int integer);
    int pop();
    int peek(int offset=0);
};

#endif /* U_STACK_H */
