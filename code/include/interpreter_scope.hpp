#pragma once

#include "bytecode.hpp"

#include <unordered_map>
#include <string>

class InterpreterFunction;
using interpreter_func_container = std::unordered_map<std::string, InterpreterFunction>;

struct InterpreterScope
{
	static interpreter_func_container funcs;
};

struct InterpreterFunction
{
	// index of argument in function call (minus 1), is the index of param id
	std::vector<bytecode_t> params;
	bytecodes_t codes;
};