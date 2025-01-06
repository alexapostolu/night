#pragma once

#include "bytecode.h"
#include "type.hpp"
#include "error.hpp"
#include "function.h"

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
	uint64_t id;
	Type type;
	
	// Keep track of the number of times used so unused variables can be
	// eliminated in the optimizing stage.
	// Not meant to be assigned to other variables, so unsigned type is
	// appropriate.
	unsigned times_used;
};

struct StatementFunction
{
	uint64_t id;

	std::vector<std::string> param_names;
	std::vector<Type> param_types;
	std::optional<Type> rtn_type;

	std::vector<uint64_t> param_ids;
	bytes_t bytes;
};

struct StatementScope
{
public:
	StatementScope();
	StatementScope(StatementScope const& upper_scope);
	StatementScope(StatementScope const& upper_scope, std::optional<Type> const& _rtn_type);

	// returns id for new variable if successful
	// returns nullopt if unsuccessful - redefinition or variable scope limit
	std::optional<uint64_t> create_variable(
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
		std::optional<Type> const& rtn_type,
		std::vector<uint64_t> _param_ids
	);

	static scope_func_container functions;

	uint64_t variable_id;

	std::optional<Type> rtn_type;
	static unsigned int max_var_id;

private:
	scope_var_container vars;
};