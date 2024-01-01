#pragma once

#include "interpreter_scope.hpp"
#include "bytecode.hpp"
#include "debug.hpp"

#include <math.h>
#include <stack>
#include <functional>
#include <optional>
#include <bitset>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

std::optional<intpr::Value> interpret_bytecodes(InterpreterScope& scope, bytecodes_t const& codes);

// template can either be int64_t or uint64_t
// iterator
//   start: int code type
//   end: last code of int
template <typename T>
T get_int(bytecodes_t::const_iterator& it)
{
	int count;

	switch ((BytecodeType)(*it))
	{
	case BytecodeType::S_INT1:
	case BytecodeType::U_INT1: count = 1; break;
	case BytecodeType::S_INT2:
	case BytecodeType::U_INT2: count = 2; break;
	case BytecodeType::S_INT4:
	case BytecodeType::U_INT4: count = 4; break;
	case BytecodeType::S_INT8:
	case BytecodeType::U_INT8: count = 8; break;
	default: throw debug::unhandled_case(*it);
	}

	T num = 0;
	for (int i = 0; i < count; ++i)
	{
		T it_n = *(++it);
		for (int j = 0; j < i; ++j)
			it_n <<= 8;

		num |= it_n;
	}

	return num;
}


// iterator
//   start: bytecode type
//   end:   last code of float
void push_float(std::stack<intpr::Value>& s, bytecodes_t::const_iterator& it);

void push_str(std::stack<intpr::Value>& s);
void push_arr(std::stack<intpr::Value>& s);
void push_arr_and_fill(std::stack<intpr::Value>& s);
void fill_arr(intpr::Value& arr, std::stack<intpr::Value>& s, std::vector<int> const& dimensions, int current_dimension);

void push_subscript(std::stack<intpr::Value>& s, bool is_string);

intpr::Value pop(std::stack<intpr::Value>& s);

char* night_get_line();
