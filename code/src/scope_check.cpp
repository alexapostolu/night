#include "scope_check.hpp"
#include "statement_scope.hpp"

#include <unordered_set>
#include <string>

bool check_variable_defined(StatementScope& scope, std::string const& name, Location const& loc)
{
	static std::unordered_set<std::string> undefined_variables;

	if (!scope.get_var(name))
	{
		if (!undefined_variables.contains(name))
			night::error::get().create_minor_error("variable '" + name + "' is undefined", loc);

		undefined_variables.insert(name);
		return false;
	}

	return true;
}

bool is_function_defined(StatementScope const& scope, std::string const& name, Location const& loc)
{
	static std::unordered_set<std::string> undefined_functions;

	if (!scope.funcs.contains(name))
	{
		if (!undefined_functions.contains(name))
			night::error::get().create_minor_error("function '" + name + "' is undefined", loc);

		undefined_functions.insert(name);
		return false;
	}

	return true;
}