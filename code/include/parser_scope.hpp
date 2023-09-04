#pragma once

#include "bytecode.hpp"
#include "value_type.hpp"
#include "error.hpp"

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
	ParserScope();
	ParserScope(ParserScope const& upper_scope);
	ParserScope(ParserScope const& upper_scope, std::optional<ValueType> const& _rtn_type);

	// returns id for new variable if successful
	// returns nullopt if unsuccessful - redefinition or variable scope limit
	std::optional<bytecode_t> create_variable(
		std::string const& name,
		ValueType const& type,
		Location const& loc
	);

	// returns func it if successful
	// throws const char* if unsuccessful

	static scope_func_container::iterator create_function(
		std::string const& name,
		std::vector<std::string> const& param_names,
		std::vector<ValueType> const& param_types,
		std::optional<ValueType> const& rtn_type
	);

	// throws const char* if types do not match
	void check_return_type(std::optional<ValueType> const& _rtn_type) const;

	std::optional<ValueType> const& get_curr_rtn_type() const;
	void set_curr_rtn_type(std::optional<ValueType> const& _curr_rtn_type);

	static scope_func_container funcs;

	scope_var_container vars;

private:
	std::optional<ValueType> rtn_type;
};