#pragma once

#include "bytecode.hpp"
#include "value.hpp"

#include <string>
#include <unordered_map>
#include <vector>

struct ParserFunction;
struct Variable;

// has to be ordered
using var_container   = std::unordered_map<std::string, Variable>;
using func_container  = std::unordered_map<std::string, ParserFunction>;
using class_container = std::unordered_map<std::string, int>;

struct Variable
{
	ValueType type;
	bytecode_t id;
};

struct ParserFunction
{
	std::vector<ValueType>  params;
	bytecodes_t codes;
};

struct Scope
{
	var_container   vars;
	func_container  funcs;
	class_container objs;
};