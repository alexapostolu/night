#pragma once

#include "value.hpp"

#include <string>
#include <map>
#include <vector>

struct Function;

using var_container  = std::map<std::string, ValueType>;
using func_container = std::map<std::string, Function>;

struct Function
{
	std::vector<ValueType> params;
	std::vector<Bytecode>  codes;
};

struct Scope
{
	var_container  vars;
	func_container funcs;
};