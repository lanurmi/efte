#ifndef STACK_H
#define STACK_H

#define MAXSTACK    16

class StackItem {
public:
    int typ;

    union {
        int integer;
        char *string;
    };

    StackItem();
    ~StackItem();

    void empty();
    void set( int integer );
    void set( const char *string );
};

class Stack {
private:
    StackItem items[MAXSTACK];
    int position;

public:
    enum {
        ST_EMPTY = 0,
        ST_INTEGER,
        ST_STRING
    };

    Stack();

    void inc();
    void dec();

    void pushEmpty();
    void push( StackItem item );
    void push( int num );
    void push( const char *string );
    StackItem *peek( int offset=0 );
    void pop(int count=1);
};

#endif /* STACK_H */
