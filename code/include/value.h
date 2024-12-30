/*
 * Defines Value used in the Interpreter.
 * 
 * As the Interpreter is reading bytecode, it converts the bytecode into Values
 * for easier usage. It also uses Values to store and update values of variables.
 */

#pragma once

#include <stdint.h>

typedef struct {
	enum {
		Int,
		uInt,
		Dbl,
		Str,
		Arr
	} is;

	union {
		int64_t i;
		uint64_t ui;
		double d;
		char* s;
		Value* a;
	} as;

	// Length of the string or array.
	size_t len;
} Value;

void value_create_i(Value** _val, int64_t i);
void value_create_ui(Value** _val, uint64_t ui);
void value_create_d(Value** _val, double d);
int  value_create_s(Value** _val, char* s, size_t len);
int  value_create_a(Value** _val, Value* a, size_t len);
int  value_create_val(Value** _val, Value* other);

void value_destroy(Value* val);

