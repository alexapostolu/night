#include "parser_scope.hpp"
#include "value_type.hpp"

#include <iostream>
#include <string>

scope_func_container ParserScope::funcs = {
	{ "print", ParserFunction{ 0, {}, { (value_t)ValueType::BOOL }, std::nullopt } },
	{ "print", ParserFunction{ 1, {}, { (value_t)ValueType::CHAR }, std::nullopt } },
	{ "print", ParserFunction{ 2, {}, { (value_t)ValueType::INT }, std::nullopt } },
	{ "input", ParserFunction{ 1, {}, {}, (value_t)ValueType::STRING } }
};

scope_func_container::iterator ParserScope::curr_func = std::end(ParserScope::funcs);

std::string ParserScope::create_variable(std::string const& name, value_t type)
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