#include "value.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

void create_value(Value* val, int64_t i)
{
	assert(val);

	val->is = Value::Int;
	val->as.i = i;
}

void create_value(Value* val, uint64_t ui)
{
	assert(val);

	val->is = Value::uInt;
	val->as.ui = ui;
}

void create_value(Value* val, double d)
{
	assert(val);

	val->is = Value::Dbl;
	val->as.d = d;
}

int create_value(Value* val, char* s, size_t len)
{
	assert(val);
	assert(s);

	val->is = Value::Str;

	val->as.s = (char*)malloc((len + 1) * sizeof(char));
	if (!val->as.s)
		return -1;

	strncpy(val->as.s, s, len);
	val->as.s[len] = '\0';

	val->len = len;
}

int create_value(Value* val, Value* a, size_t len)
{
	assert(val);
	assert(a);

	val->is = Value::Arr;

	val->as.a = (Value*)malloc(len * sizeof(Value));
	if (!val->as.a)
		return -1;

	memcpy(val->as.a, a, len * sizeof(Value));
}

int create_value(Value* val, Value* other)
{
	assert(val);
	assert(other);

	if (other->is == Value::Int)
	{
		create_value(val, other->as.i);
		return 0;
	}
	else if (other->is == Value::uInt)
	{
		create_value(val, other->as.ui);
		return 0;
	}
	else if (other->is == Value::Dbl)
	{
		create_value(val, other->as.d);
		return 0;
	}
	else if (other->is == Value::Str)
	{
		return create_value(val, other->as.s, other->len);
	}
	else if (other->is == Value::Arr)
	{
		return create_value(val, other->as.a, other->len);
	}
}

void destroy_value(Value* val)
{
	assert(val);

	if (val->is == Value::Str)
	{
		assert(val->as.s);

		free(val->as.s);
		val->as.s = NULL;
	}
	else if (val->is == Value::Arr)
	{
		assert(val->as.a);

		free(val->as.a);
		val->as.a = NULL;
	}
}
