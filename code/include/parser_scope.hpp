#pragma once

#include "bytecode.hpp"
#include "value_type.hpp"
#include "error.hpp"

#include <unordered_map>
#include <vector>
#include <utility>
#include <memory>
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
	
	// Keep track of the number of times used so we can eliminate variables
	// that are never used in the optimizing stage.
	int times_used;
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
public:
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

	ParserVariable const* get_var(std::string const& name);
	bool is_var_used(std::string const& name) const;

	// returns func it if successful
	// throws const char* if unsuccessful
	static scope_func_container::iterator create_function(
		std::string const& name,
		std::vector<std::string> const& param_names,
		std::vector<ValueType> const& param_types,
		std::optional<ValueType> const& rtn_type
	);

	static scope_func_container funcs;

	static bool inside_false_conditional;

	std::optional<ValueType> rtn_type;

private:
	scope_var_container vars;
};