#pragma once

#include "bytecode.hpp"
#include "value.hpp"

#include <string>
#include <optional>
#include <unordered_map>
#include <vector>
#include <string>

struct ParserFunction;
struct Variable;

// has to be ordered
using scope_var_container   = std::unordered_map<std::string, Variable>;
using scope_func_container  = std::unordered_map<std::string, ParserFunction>;
using scope_class_container = std::unordered_map<std::string, int>;

struct Variable
{
	val::value_t type;
	bytecode_t id;
};

struct ParserFunction
{
	std::vector<value_t> param_types;
	std::optional<value_t> rtn_type;
};

struct ParserScope
{
	static scope_func_container funcs;
	static scope_func_container::iterator curr_func;
	
	scope_var_container   vars;
	scope_class_container objs;

	value_t rtn_type;

	// returns:
	//    empty string if successful
	//    error message if not successful
	std::string const& create_variable(std::string const& name, value_t type);
};