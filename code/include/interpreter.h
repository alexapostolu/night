/*
 * @brief Interprets a sequence of bytecodes.
 * 
 * The Interpreter works on two main datatypes, a sequence of bytecodes and an
 * array of variables.
 *
 * **Design Decision #1**
 * Variables are stored in an array.
 * 
 * Variables can be indexed with their id, and variables that are sequential in
 * the code have sequential ids. For this reason they are stored in an array
 * which has constant insertion and access times.
 * 
 * A linked list may have better memory efficiency by has linear access
 * times and a hash map is unnecessarily as it has no performance gain over an
 * array for insertion and access while adding unnecessary complexity.
 * 
 * But how is it guaranteed for variables to have sequential ids if they are
 * separated by a child scope, and the child scope variables are overwritten by
 * the parent variables?
 * 
 * **Design Decision #1.5**
 * Sequential variables in the variables array are guaranteed to have sequential
 * ids.
 * 
 * This is guaranteed because a variables id is an increment away from the
 * previous variable in the same scope. So it is not dependent on any variables
 * the parent may have created in a child scope. And since any variables in the
 * child scope become obsolete once the child scope ends and can be overwritten
 * by any new variable, then all the variables with sequential IDs are guaranteed
 * to be sequential in the variables array.
 * 
 * Example demonstrating how variables of the same scope have sequential IDs.
 * @code
 *   var1 int = 1; // id of 0       variables = { id:0 }
 *   if (true) {
 *     var2 int = 2; // id of 1     variables = { id:0, id:1 }        // child scope inherits variable id:0 from parent scope
 *     var3 int = 3; // id of 2     variables = { id:0, id:1, id:2 }
 *	 }                              variables = { id:0 }              // child scope ends, the child variables are now obsolete (deleted)
 *   var4 int = 4; // id of 1       variables = { id:0, id:1 }
 * @endcode
 */

#pragma once

#include "bytecode.h"
#include "value.h"
#include "stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief Interprets a sequence of bytecodes while maintaining an array of
 * variables.
 * 
 * The variables are inherited from the parent scope, and any new variables
 * added in the scope become obsolete once the scope ends and will get
 * overwritten by any new variables in the parent scope.
 * 
 * @param codes A sequence of bytecodes to interpret.
 * @param variables An array of variables from the parent scope. Can be NULL if
 *   no parent (global scope).
 * 
 * @returns the return value of the bytecodes.
 */
Value interpret_bytecodes(
	byte_t const* codes,
	size_t codes_count,
	Value* variables
);

int interpret_int(
	byte_t const** byte,
	stack* s,
	byte_t size,
	int u
);


// iterator
//   start: bytecode type
//   end:   last code of float
void push_float(std::stack<Value>& s, bytecodes_t::const_iterator& it, int count);

/**
 * Iterator starts at bytecode type (either FLOAT4 or FLOAT8) and ends at last
 * code of the float.
 */
double get_float(bytecodes_t::const_iterator& it);

void push_str(std::stack<Value>& s);
void push_arr(std::stack<Value>& s);
void push_arr_and_fill(std::stack<Value>& s);
void fill_arr(Value& arr, std::stack<Value>& s, std::vector<int> const& dimensions, int current_dimension);

void push_subscript(std::stack<Value>& s, bool is_string);

void push_string_input(std::stack<Value>& s);

Value pop(std::stack<Value>& s);

char* night_get_line();

#ifdef __cplusplus
}
#endif