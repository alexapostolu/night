#pragma once

#include "common/bytecode.hpp"
#include "common/type.hpp"
#include "common/error.hpp"

#include <unordered_map>
#include <vector>
#include <optional>
#include <string>

struct StatementVariable;
struct StatementFunction;

using scope_var_container  = std::unordered_map<std::string, StatementVariable>;
using scope_func_container = std::unordered_multimap<std::string, StatementFunction>;

namespace night {

using id_t = uint64_t;

} // night::

struct StatementVariable
{
	night::id_t id;
	Type type;
	
	// Keep track of the number of times used so unused variables can be
	// eliminated in the optimizing stage.
	unsigned times_used;
};

struct StatementFunction
{
	night::id_t id;

	std::vector<std::string> param_names;
	std::vector<Type> param_types;
	std::optional<Type> rtn_type;
};

class StatementScope
{
public:
	StatementScope();

	StatementScope(
		StatementScope const& parent_scope
	);

	StatementScope(
		StatementScope const& parent_scope,
		std::optional<Type> const& _return_type
	);

	/*
	 * Creates a new variables and returns its ID.
	 * Returns nullopt if the variable is already defined or the maximum
	 * allowed variables has been reached.
	 * 
	 * @param name_location is to display the correct error location for the
	 *   variable name.
	 */
	std::optional<night::id_t> create_variable(
		std::string const& name,
		Location	const& name_location,
		Type const& type
	);

	/*
	 * Returns a pointer to the variable if the variable is defined in the scope.
	 * Otherwise creates a minor error and returns nullptr.
	 */
	StatementVariable const* get_variable(
		std::string const& name,
		Location const& name_location
	);

	/*
	 * Creates a new function and returns its ID.
	 * Returns nullopt if the function has already been defined or the maximum
	 * allowed functions has been reached.
	 */
	static std::optional<night::id_t> create_function(
		std::string const& name,
		Location	const& name_location,
		std::vector<std::string> const& _param_names,
		std::vector<Type> const& _param_types,
		std::optional<Type> const& _return_type
	);

	static scope_func_container functions;

	std::optional<Type> return_type;

private:
	scope_var_container variables;
};