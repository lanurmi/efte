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
