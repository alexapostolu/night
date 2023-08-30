#include "parser_scope.hpp"
#include "value_type.hpp"

#include <iostream>
#include <optional>
#include <string>

scope_func_container ParserScope::funcs = {
	{ "print", ParserFunction{ 0, {}, { ValueType::BOOL }, std::nullopt } },
	{ "print", ParserFunction{ 1, {}, { ValueType::CHAR }, std::nullopt } },
	{ "print", ParserFunction{ 2, {}, { ValueType::INT }, std::nullopt } },
	{ "print", ParserFunction{ 3, {}, { ValueType::FLOAT }, std::nullopt } },
	{ "print", ParserFunction{ 4, {}, { ValueType::STR }, std::nullopt } },
	{ "input", ParserFunction{ 5, {}, {}, ValueType::STR } },
	{ "char",   ParserFunction{ 6, {}, { ValueType::INT }, ValueType::CHAR } },
	{ "int",   ParserFunction{ 7, {}, { ValueType::STR }, ValueType::INT } },
	{ "int",   ParserFunction{ 8, {}, { ValueType::CHAR }, ValueType::INT } },
	{ "str",   ParserFunction{ 9, {}, { ValueType::INT }, ValueType::STR } },
	{ "len",   ParserFunction{ 10, {}, { ValueType::STR }, ValueType::INT } }
};

std::optional<ValueType> ParserScope::curr_rtn_type = std::nullopt;

std::pair<std::string, bytecode_t> ParserScope::create_variable(std::string const& name, ValueType type)
{
	static bytecode_t var_id = 0;

	if (vars.contains(name))
		return std::make_pair("variable '" + name + "' is already defined", 0);

	if (var_id == bytecode_t_lim)
	{
		vars[name] = { type };

		return std::make_pair("only " + std::to_string(bytecode_t_lim) + " variables allowed per scope", 0);
	}

	vars[name] = { type, var_id };

	return std::make_pair("", var_id++);
}

std::pair<std::string, scope_func_container::iterator> ParserScope::create_function(
	std::string const& name,
	std::vector<std::string> const& param_names,
	std::vector<ValueType> param_types,
	std::optional<ValueType> rtn_type)
{
	static bytecode_t func_id = ParserScope::funcs.size();

	auto [it, range_end] = ParserScope::funcs.equal_range(name);

	for (; it != range_end; ++it)
	{
		if (std::equal(std::begin(param_types), std::end(param_types),
					   std::begin(it->second.param_types), std::end(it->second.param_types),
					   compare_absolute_vt))
			return std::make_pair("function is already defined", range_end);
	}

	return std::make_pair(
		"",
		funcs.emplace(name, ParserFunction{ func_id++, param_names, param_types, rtn_type }));
}