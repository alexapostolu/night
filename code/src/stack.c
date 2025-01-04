#include "stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void stack_create(stack* s)
{
    s->top = NULL;
}

int stack_is_empty(stack* s)
{
    return !s->top;
}

void stack_push(stack* s, Value* val)
{
    node new_node;
    new_node->val = val;
    new_node->next = stack->top;
    stack->top = new_node;
}

Value* stack_pop(stack* s)
{
    assert(!stack_is_empty(s));
    
    node* top = s->top;
    s->top = s_top->next;

    return top->val;
}

int64_t stack_pop_as_i(stack* s)
{
    Value* val = stack_pop(s);
    assert(val);
    assert(val->is == Value::Int);

    int64_t i = val->as.i;
    free(val);

    return i;
}

uint64_t stack_pop_as_ui(stack* s)
{
    Value* val = stack_pop(s);
    assert(val);
    assert(val->is == Value::uInt);

    uint64_t ui = val->as.ui;
    free(val);

    return ui;
}

double stack_pop_as_d(stack* s)
{
    Value* val = stack_pop(s);
    assert(val);
    assert(val->is == Value::Dbl);

    double d = val->as.d;
    free(val);

    return d;
}

char* stack_pop_as_s(stack* s)
{
    Value* val = stack_pop(s);
    assert(val);
    assert(val->is == Value::Str);

    char* s = val->as.s;
    free(val);

    return s;
}

Value* stack_top(stack* s)
{
    assert(!stack_is_empty(s));

    return s->top->val;
}
