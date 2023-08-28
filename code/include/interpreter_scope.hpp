#pragma once

#include "bytecode.hpp"

#include <unordered_map>
#include <vector>
#include <variant>
#include <string>

namespace intpr
{

enum class ValueType
{
	INT, UINT, FLOAT, STR, ARR
};

struct Value
{
	ValueType type;
	
	int64_t i;
	float f;
	std::string s;
	std::vector<Value> v;

	Value() = default;
	explicit Value(int64_t _i);
	explicit Value(float _i);
	explicit Value(std::string _s);
	explicit Value(std::vector<Value> const& _v);
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
