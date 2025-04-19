#pragma once

#include "bytecode.hpp"

#include <unordered_map>
#include <vector>

namespace intpr
{

struct Value;

struct Array
{
	Value* data;
	std::size_t size;
};

struct Value
{
	union {
		int64_t i;
		uint64_t ui;
		double d;
		char* s;
		Array a;
	} as;

	Value() = default;
	Value(int64_t _i);
	Value(uint64_t _ui);
	Value(double _d);
	Value(char* _s);
	Value(Array _a);
	Value(Value const& _v);
	~Value();
};

}

struct Variable
{
	intpr::Value val;
	bool is_param;
};

// <id, val>
using var_container = std::unordered_map<uint64_t, Variable>;

struct InterpreterFunction;
using func_container = std::unordered_map<uint64_t, InterpreterFunction>;

struct InterpreterFunction
{
	std::vector<uint64_t> param_ids;
	bytecodes_t codes;
};

struct InterpreterScope
{
	static func_container funcs;
	var_container vars;
};
