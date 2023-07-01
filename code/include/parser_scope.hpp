#pragma once

#include "bytecode.hpp"
#include "value.hpp"

#include <string>
#include <unordered_map>
#include <vector>

struct ParserFunction;
struct Variable;

// has to be ordered
using scope_var_container   = std::unordered_map<std::string, Variable>;
using scope_func_container  = std::unordered_map<std::string, ParserFunction>;
using scope_class_container = std::unordered_map<std::string, int>;

struct Variable
{
	value_t type;
	bytecode_t id;
};

struct ParserFunction
{
	std::vector<ValueType> param_types;
	ValueType rtn_type;
};

struct ParserScope
{
	scope_var_container   vars;
	scope_func_container  funcs;
	scope_class_container objs;

	std::string const& create_variable(std::string const& name, value_t type);
};