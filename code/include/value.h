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

void create_value(Value* val, int64_t i);
void create_value(Value* val, uint64_t ui);
void create_value(Value* val, double d);
int create_value(Value* val, char* s, size_t len);
int create_value(Value* val, Value* a, size_t len);
int create_value(Value* val, Value* other);

void destroy_value(Value* val);

