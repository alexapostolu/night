#include "parser_scope.hpp"
#include "error.hpp"

#include <limits>
#include <string>

std::string const& ParserScope::create_variable(std::string const& name, value_t type)
{
	static bytecode_t var_id    = 0;
	const  auto		  var_limit = std::numeric_limits<bytecode_t>::max();

	if (vars.contains(name))
	{
		return "variable '" + name + "' is already defined";
	}

	if (var_id == var_limit)
	{
		vars[name] = { type };

		return "only " + std::to_string(var_limit) + " variables allowed per scope";
	}

	vars[name] = { type, var_id++ };
}