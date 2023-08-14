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
	INT, UINT, FLOAT, STR
};

struct Value
{
	ValueType type;
	
	int64_t i;
	uint64_t ui;
	float f;
	std::string s;

	Value() = default;
	explicit Value(int64_t _i);
	explicit Value(uint64_t _ui);
	explicit Value(float _i);
	explicit Value(std::string _s);
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
