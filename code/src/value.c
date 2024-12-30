#include "value.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

void value_create_i(Value** _val, int64_t i)
{
	assert(_val);

	Value* val = (Value*)malloc(sizeof(Value));

	val->is = Value::Int;
	val->as.i = i;

	*_val = val;
}

void value_create_ui(Value** _val, uint64_t ui)
{
	assert(_val);

	Value* val = (Value*)malloc(sizeof(Value));

	val->is = Value::uInt;
	val->as.ui = ui;

	*_val = val;
}

void value_create_d(Value** _val, double d)
{
	assert(_val);

	Value* val = (Value*)malloc(sizeof(Value));

	val->is = Value::Dbl;
	val->as.d = d;

	*_val = val;
}

int value_create_s(Value** _val, char* s, size_t len)
{
	assert(_val);
	assert(s);

	Value* val = (Value*)malloc(sizeof(Value));

	val->is = Value::Str;

	val->as.s = (char*)malloc((len + 1) * sizeof(char));
	if (!val->as.s)
		return -1;

	strncpy(val->as.s, s, len);
	val->as.s[len] = '\0';

	val->len = len;

	*_val = val;
}

int value_create_a(Value** _val, Value* a, size_t len)
{
	assert(_val);
	assert(a);

	Value* val = (Value*)malloc(sizeof(Value));

	val->is = Value::Arr;

	val->as.a = (Value*)malloc(len * sizeof(Value));
	if (!val->as.a)
		return -1;

	memcpy(val->as.a, a, len * sizeof(Value));

	*_val = val;
}

int value_create_val(Value** _val, Value* other)
{
	assert(_val);
	assert(other);

	if (other->is == Value::Int)
	{
		create_value(_val, other->as.i);
		return 0;
	}
	else if (other->is == Value::uInt)
	{
		create_value(_val, other->as.ui);
		return 0;
	}
	else if (other->is == Value::Dbl)
	{
		create_value(_val, other->as.d);
		return 0;
	}
	else if (other->is == Value::Str)
	{
		return create_value(_val, other->as.s, other->len);
	}
	else if (other->is == Value::Arr)
	{
		return create_value(_val, other->as.a, other->len);
	}
}

void value_destroy(Value* val)
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

	free(val);
}
