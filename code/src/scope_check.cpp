#include "scope_check.hpp"
#include "parser_scope.hpp"

#include <unordered_set>
#include <string>

bool check_variable_defined(ParserScope const& scope, std::string const& var_name, Location const& loc)
{
	static std::unordered_set<std::string> undefined_variables;

	if (!scope.vars.contains(var_name))
	{
		// Make sure this error message is only displayed once for each undefined variable
		// as to not crowd the error message list with indentical messages.

		if (!undefined_variables.contains(var_name))
			night::error::get().create_minor_error("variable '" + var_name + "' is undefined", loc);

		undefined_variables.insert(var_name);
		return false;
	}

	return true;
}