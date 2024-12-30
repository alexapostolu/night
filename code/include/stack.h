/*
 * Stack used by Interpreter.
 *
 * All allocations should be made before pushing to stack.
 */

#pragma once

#include "value.h"

typedef struct node {
    Value* val;
    struct node* next;
} node;

typedef struct {
    node* top;
} stack;

void stack_create(stack* s);

int stack_is_empty(stack* s);

void stack_push(stack* s, Value* val);

Value* stack_pop(stack* s);

Value* stack_top(stack* s);
