#pragma once

#include "interpreter_scope.hpp"
#include "bytecode.hpp"
#include "debug.hpp"

#include <math.h>
#include <stack>
#include <optional>

std::optional<intpr::Value> interpret_bytecodes(InterpreterScope& scope, bytecodes_t const& codes);

// iterator
//   start: int bytecode type
//   end:   last code of int
template <typename T>
T get_int(bytecodes_t::const_iterator& it)
{
	int count;

	switch ((BytecodeType)(*it))
	{
	case BytecodeType::S_INT1:
	case BytecodeType::U_INT1: count = 8;  break;
	case BytecodeType::S_INT2:
	case BytecodeType::U_INT2: count = 16; break;
	case BytecodeType::S_INT4:
	case BytecodeType::U_INT4: count = 32; break;
	case BytecodeType::S_INT8:
	case BytecodeType::U_INT8: count = 64; break;
	default: throw debug::unhandled_case(*it);
	}

	T num = *(++it);
	for (int i = 8; i < count; i *= 2)
	{
		auto byte = *(++it);
		num += byte * pow(2, i);
	}

	return num;
}


// iterator
//   start: bytecode type
//   end:   last code of float
void push_float(std::stack<intpr::Value>& s, bytecodes_t::const_iterator& it);


// iterator
//   start: bytecode type
//   end:   last code of string
void push_str(std::stack<intpr::Value>& s, bytecodes_t::const_iterator& it);

// iterator
//   start: bytecode type
//   end:   last code of strarring
void push_arr(std::stack<intpr::Value>& s, bytecodes_t::const_iterator& it);

intpr::Value pop(std::stack<intpr::Value>& s);