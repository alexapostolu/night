
#pragma once

#include "bytecode.hpp"
#include "value_type.hpp"

#include <unordered_map>
#include <vector>
#include <utility>
#include <optional>
#include <string>

struct ParserVariable;
struct ParserFunction;

using scope_var_container  = std::unordered_map<std::string, ParserVariable>;
using scope_func_container = std::unordered_multimap<std::string, ParserFunction>;

struct ParserVariable
{
	ValueType type;
	bytecode_t id;
};

struct ParserFunction
{
	// used when generating bytecode for function call
	bytecode_t id;

	std::vector<std::string> param_names;
	std::vector<ValueType> param_types;
	std::optional<ValueType> rtn_type;
};

struct ParserScope
{
	static scope_func_container funcs;

	// this value is set in parse_func()
	// parse_return() will compare this value with its own expression type
	// for type checking
	static std::optional<ValueType> curr_rtn_type;
	
	scope_var_container vars;

	// returns:
	//    <"", id> if successful
	//    <error message, 0> if unsuccessful
	std::pair<std::string, bytecode_t> create_variable(std::string const& name, ValueType type);

	// returns:
	//    <"", it> if successful
	//    <error message, it::end()> if unsuccessful
	static std::pair<std::string, scope_func_container::iterator> create_function(
		std::string const& name,
		std::vector<std::string> const& param_names,
		std::vector<ValueType> param_types,
		std::optional<ValueType> rtn_type);
};