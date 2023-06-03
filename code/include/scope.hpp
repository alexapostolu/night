#pragma once

#include "value.hpp"

#include <string>
#include <map>
#include <vector>

struct ParserFunction;

// has to be ordered
using var_container  = std::map<std::string, ValueType>;
using func_container = std::map<std::string, ParserFunction>;

struct ParserFunction
{
	std::vector<ValueType> params;
	std::vector<bytecode_t>  codes;
};

struct Scope
{
	var_container  vars;
	func_container funcs;
};