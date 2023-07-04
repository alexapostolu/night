#include "parser_scope.hpp"
#include "value.hpp"

#include <string>

scope_func_container::iterator curr_func = std::end(ParserScope::funcs);

std::string const& ParserScope::create_variable(std::string const& name, value_t type)
{
	static bytecode_t var_id = 0;

	if (vars.contains(name))
	{
		return "variable '" + name + "' is already defined";
	}

	if (var_id == bytecode_t_lim)
	{
		vars[name] = { type };

		return "only " + std::to_string(bytecode_t_lim) + " variables allowed per scope";
	}

	vars[name] = { type, var_id++ };
}