/*
 * Defines Value used in the Interpreter.
 * 
 * As the Interpreter is reading bytecode, it converts the bytecode into Values
 * for easier usage. It also uses Values to store and update values of variables.
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Value {
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

void value_create_i(struct Value** _val, int64_t i);
void value_create_ui(struct Value** _val, uint64_t ui);
void value_create_d(struct Value** _val, double d);
int  value_create_s(struct Value** _val, char* s, size_t len);
int  value_create_a(struct Value** _val, struct Value* a, size_t len);
int  value_create_val(struct Value** _val, struct Value* other);

void value_destroy(struct Value* val);

#ifdef __cplusplus
}
#endif
