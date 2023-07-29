#pragma once

#include "bytecode.hpp"

#include <unordered_map>
#include <vector>
#include <string>

// <id, val>
using var_container = std::unordered_map<bytecode_t, int>;

struct InterpreterFunction;
using func_container = std::unordered_map<std::string, InterpreterFunction>;

struct InterpreterFunction
{
	std::vector<int> params;
	bytecodes_t codes;
};