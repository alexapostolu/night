#pragma once

#include "interpreter_scope.hpp"
#include "common/bytecode.hpp"
#include "common/debug.hpp"

#include <math.h>
#include <stack>
#include <functional>
#include <optional>
#include <bitset>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

std::optional<intpr::Value> interpret_bytecodes(
	InterpreterScope& scope,
	bytecodes_t const& codes,
	bool is_global,
	char* buf = NULL
);

// template can either be int64_t or uint64_t
// iterator
//   start: int code type
//   end: last code of int
template <typename T>
T interpret_int(bytecodes_t::const_iterator& it, unsigned short size)
{
	T num = 0;
	for (unsigned short i = 0; i < size; ++i)
	{
		T byte = *(++it);
		for (unsigned short j = 0; j < i; ++j)
			byte <<= 8;

		num |= byte;
	}

	return num;
}

double interpret_flt(bytecodes_t::const_iterator& it, unsigned short size);

char* interpret_predefined_input();

// iterator
//   start: bytecode type
//   end:   last code of float
void push_float(std::stack<intpr::Value>& s, bytecodes_t::const_iterator& it, int count);

void push_str(std::stack<intpr::Value>& s);
void push_arr(std::stack<intpr::Value>& s);
void push_arr_and_fill(std::stack<intpr::Value>& s);
void fill_arr(intpr::Value& arr, std::stack<intpr::Value>& s, std::vector<int> const& dimensions, int current_dimension);

void push_subscript(std::stack<intpr::Value>& s, bool is_string);

void push_string_input(std::stack<intpr::Value>& s);

intpr::Value pop(std::stack<intpr::Value>& s);

char* night_get_line();
