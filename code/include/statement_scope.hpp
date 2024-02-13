#pragma once

#include "bytecode.hpp"
#include "type.hpp"
#include "error.hpp"

#include <unordered_map>
#include <vector>
#include <optional>
#include <string>

struct StatementVariable;
struct StatementFunction;

using scope_var_container  = std::unordered_map<std::string, StatementVariable>;
using scope_func_container = std::unordered_multimap<std::string, StatementFunction>;

struct StatementVariable
{
	bytecode_t id;
	Type type;
	
	// Keep track of the number of times used so unused variables can be
	// eliminated in the optimizing stage.
	// Not meant to be assigned to other variables, so unsigned type is
	// appropriate.
	unsigned times_used;
};

struct StatementFunction
{
	bytecode_t id;

	std::vector<std::string> param_names;
	std::vector<Type> param_types;
	std::optional<Type> rtn_type;
};

struct StatementScope
{
public:
	StatementScope();
	StatementScope(StatementScope const& upper_scope);
	StatementScope(StatementScope const& upper_scope, std::optional<Type> const& _rtn_type);

	// returns id for new variable if successful
	// returns nullopt if unsuccessful - redefinition or variable scope limit
	std::optional<bytecode_t> create_variable(
		std::string const& name,
		Type const& type,
		Location const& loc
	);

	StatementVariable const* get_var(std::string const& name);

	// returns func it if successful
	// throws const char* if unsuccessful
	static scope_func_container::iterator create_function(
		std::string const& name,
		std::vector<std::string> const& param_names,
		std::vector<Type> const& param_types,
		std::optional<Type> const& rtn_type
	);

	static scope_func_container funcs;

	std::optional<Type> rtn_type;

private:
	scope_var_container vars;
};