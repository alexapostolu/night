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
    
    node* tmp = s->top;
    Value* val = tmp->data;
    s->top = tmp->next;

    return tmp->val;
}

Value* stack_top(stack* s)
{
    assert(!stack_is_empty(s));

    return s->top->val;
}
