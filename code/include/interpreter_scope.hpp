#pragma once

#include "bytecode.hpp"

#include <unordered_map>
#include <vector>

namespace intpr
{

struct Value
{
	union {
		int64_t i;
		uint64_t ui;
		double d;
		char* s;
		Value* a;
	} as;

	Value() = default;
	Value(int64_t _i);
	Value(uint64_t _ui);
	Value(double _d);
	Value(char* _d);
	Value(Value* _a);
	Value(Value const& _v);
};

}

// <id, val>
using var_container = std::unordered_map<bytecode_t, intpr::Value>;

struct InterpreterFunction;
using func_container = std::unordered_map<bytecode_t, InterpreterFunction>;

struct InterpreterFunction
{
	std::vector<bytecode_t> param_ids;
	bytecodes_t codes;
};

struct InterpreterScope
{
	static func_container funcs;
	var_container vars;

	static int new_id();
};
