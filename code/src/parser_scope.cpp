#include "parser_scope.hpp"
#include "value.hpp"

#include <iostream>
#include <string>

scope_func_container ParserScope::funcs = {
	{ "print", ParserFunction{ 0, {}, { (val::value_t)val::ValueType::S_INT }, std::nullopt } },
	{ "input", ParserFunction{ 1, {}, { }, (val::value_t)val::ValueType::S_INT } }
};

scope_func_container::iterator ParserScope::curr_func = std::end(ParserScope::funcs);

std::string ParserScope::create_variable(std::string const& name, val::value_t type)
{
	static bytecode_t var_id = 0;

	if (vars.contains(name))
		return std::string("variable '" + name + "' is already defined");

	if (var_id == bytecode_t_lim)
	{
		vars[name] = { type };

		return std::string("only " + std::to_string(bytecode_t_lim) + " variables allowed per scope");
	}

	vars[name] = { type, var_id++ };

	return "";
}