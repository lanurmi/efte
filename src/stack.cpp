/* Stack Impementation */

#include <stdlib.h>
#include <string.h>

#include "stack.h"

StackItem::StackItem() {
    this->typ = Stack::ST_EMPTY;
}

StackItem::~StackItem() {
    if (this->typ == Stack::ST_STRING)
        free( this->string );
}

void StackItem::empty() {
    if (this->typ == Stack::ST_STRING) {
        free( this->string );
    }

    this->typ = Stack::ST_EMPTY;
}

void StackItem::set( int integer ) {
    this->empty();

    this->typ = Stack::ST_INTEGER;
    this->integer = integer;
}

void StackItem::set( const char *string ) {
    this->empty();
    this->typ = Stack::ST_STRING;
    this->string = strdup( string );
}

Stack::Stack() {
    this->position = 0;
}

void Stack::inc() {
    this->position++;

    if (this->position == MAXSTACK)
        this->position = 0;
}

void Stack::dec() {
    this->position--;
    if (this->position == -1)
        this->position = MAXSTACK - 1;
}

void Stack::pushEmpty() {
    this->inc();
    this->items[this->position].empty();
}

void Stack::push( StackItem item ) {
    switch (item.typ) {
    case Stack::ST_EMPTY:
        this->pushEmpty();
        break;

    case Stack::ST_INTEGER:
        this->push( item.integer );
        break;

    case Stack::ST_STRING:
        this->push( item.string );
        break;
    }
}

void Stack::push( int num ) {
    this->inc();
    this->items[this->position].set( num );
}

void Stack::push( const char *string ) {
    this->inc();
    this->items[this->position].set( string );
}

StackItem *Stack::peek( int offset ) {
    int peekAt = this->position + offset;
    if (peekAt < 0)
        peekAt += MAXSTACK;

    return &this->items[peekAt];
}
