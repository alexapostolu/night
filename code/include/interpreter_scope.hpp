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

// <id, val>
using var_container = std::unordered_map<uint64_t, intpr::Value>;

struct InterpreterFunction;
using func_container = std::unordered_map<uint64_t, InterpreterFunction>;

struct InterpreterFunction
{
	std::vector<bytecode_t> param_ids;
	bytecodes_t codes;
};

struct InterpreterScope
{
	static func_container funcs;
	var_container vars;
};
