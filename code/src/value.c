#include "value.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

void value_create_i(Value** _val, int64_t i)
{
	assert(_val);

	Value* val = (Value*)malloc(sizeof(Value));
	assert(val);

	val->is = Val_Int;
	val->as.i = i;

	*_val = val;
}

void value_create_ui(Value** _val, uint64_t ui)
{
	assert(_val);

	Value* val = (Value*)malloc(sizeof(Value));
	assert(val);

	val->is = Val_uInt;
	val->as.ui = ui;

	*_val = val;
}

void value_create_d(Value** _val, double d)
{
	assert(_val);

	Value* val = (Value*)malloc(sizeof(Value));
	assert(val);

	val->is = Val_Dbl;
	val->as.d = d;

	*_val = val;
}

int value_create_s(Value** _val, char* s, size_t len)
{
	assert(_val);
	assert(s);

	Value* val = (Value*)malloc(sizeof(Value));
	assert(val);

	val->is = Val_Str;

	val->as.s = (char*)malloc((len + 1) * sizeof(char));
	if (!val->as.s)
		return -1;

	strncpy(val->as.s, s, len);
	val->as.s[len] = '\0';

	val->len = len;

	*_val = val;
}

int value_create_a(Value** _val, Value** a, size_t len)
{
	assert(_val);
	assert(a);

	Value* val = (Value*)malloc(sizeof(Value));
	assert(val);

	val->is = Val_Arr;
	val->as.a = a;

	*_val = val;
}

int value_create_val(Value** _val, Value* other)
{
	assert(_val);
	assert(other);

	if (other->is == Val_Int)
	{
		value_create_i(_val, other->as.i);
		return 0;
	}
	else if (other->is == Val_uInt)
	{
		value_create_ui(_val, other->as.ui);
		return 0;
	}
	else if (other->is == Val_Dbl)
	{
		value_create_d(_val, other->as.d);
		return 0;
	}
	else if (other->is == Val_Str)
	{
		return value_create_s(_val, other->as.s, other->len);
	}
	else if (other->is == Val_Arr)
	{
		return value_create_a(_val, other->as.a, other->len);
	}

	return 0;
}

void value_destroy(Value* val)
{
	assert(val);

	if (val->is == Val_Str)
	{
		assert(val->as.s);

		free(val->as.s);
		val->as.s = NULL;
	}
	else if (val->is == Val_Arr)
	{
		assert(val->as.a);

		free(val->as.a);
		val->as.a = NULL;
	}

	free(val);
}
