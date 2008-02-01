#include "u_stack.h"

CircularStack::CircularStack() {
    this->pos = -1;
    for (int i=0; i < STACKSIZE; i++)
        this->stack[i] = 0;
}

void CircularStack::push(int integer) {
    this->pos = (this->pos + 1) & (STACKSIZE - 1);
    this->stack[this->pos] = integer;
}

int CircularStack::pop() {
    int r = this->stack[this->pos];
    this->stack[this->pos] = 0;
    this->pos = (this->pos - 1) & (STACKSIZE - 1);
    return r;
}

int CircularStack::peek(int offset) {
    int p = (this->pos - offset) & (STACKSIZE - 1);
    return this->stack[p];
}
