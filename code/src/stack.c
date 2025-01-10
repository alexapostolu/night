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
    assert(val);
    node* new_node = (node*)malloc(sizeof(node));
    assert(new_node);
    new_node->val = val;
    new_node->next = s->top;
    s->top = new_node;

    printf("pushed type {%d} onto stack\n", (int)val->is);
}

Value* stack_pop(stack* s)
{
    printf("popped value off stack\n");
 
    assert(!stack_is_empty(s));

    node* top = s->top;
    s->top = s->top->next;

    Value* val = top->val;
    //free(top);

    return val;
}

int64_t stack_pop_as_i(stack* s)
{
    Value* val = stack_pop(s);
    assert(val);
    assert(val->is == Val_Int);

    int64_t i = val->as.i;
    //free(val);

    printf("  value as int {%lld}\n", i);

    return i;
}

double stack_pop_as_d(stack* s)
{
    Value* val = stack_pop(s);
    assert(val);
    assert(val->is == Val_Dbl);

    double d = val->as.d;
    //free(val);

    printf("  value as {%f}\n", d);

    return d;
}

char* stack_pop_as_s(stack* s)
{
    Value* val = stack_pop(s);
    assert(val);
    assert(val->is == Val_Str);

    char* str = val->as.s;
    //free(val);

    printf("  value as string {%s}\n", str);

    return str;
}

Value* stack_top(stack* s)
{
    assert(!stack_is_empty(s));

    return s->top->val;
}
