#include "parser_scope.hpp"
#include "value_type.hpp"
#include "error.hpp"

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
	{ "str",   ParserFunction{ 10, {}, { ValueType::FLOAT }, ValueType::STR } },
	{ "len",   ParserFunction{ 11, {}, { ValueType::STR }, ValueType::INT } }
};

ParserScope::ParserScope()
	: vars() {}

ParserScope::ParserScope(ParserScope const& upper_scope)
	: vars(upper_scope.vars), rtn_type(upper_scope.rtn_type) {}

ParserScope::ParserScope(ParserScope const& upper_scope, std::optional<ValueType> const& _rtn_type)
	: vars(upper_scope.vars), rtn_type(_rtn_type) {}

std::optional<bytecode_t> ParserScope::create_variable(
	std::string const& name,
	ValueType const& type,
	Location const& loc)
{
	static bytecode_t var_id = 0;

	if (vars.contains(name))
		night::error::get().create_minor_error("variable '" + name + "' is already defined", loc);

	if (var_id == bytecode_t_lim)
		night::error::get().create_minor_error("only " + std::to_string(bytecode_t_lim) + " variables allowed per scope", loc);

	if (night::error::get().has_minor_errors())
		return std::nullopt;

	vars[name] = { type, var_id };
	return var_id++;
}

scope_func_container::iterator ParserScope::create_function(
	std::string const& name,
	std::vector<std::string> const& param_names,
	std::vector<ValueType> const& param_types,
	std::optional<ValueType> const& rtn_type)
{
	static bytecode_t func_id = ParserScope::funcs.size();

	auto [it, range_end] = ParserScope::funcs.equal_range(name);

	for (; it != range_end; ++it)
	{
		if (std::equal(std::begin(param_types), std::end(param_types),
					   std::begin(it->second.param_types), std::end(it->second.param_types),
					   compare_absolute_vt))
			throw "function is already defined";
	}

	return funcs.emplace(name, ParserFunction{ func_id++, param_names, param_types, rtn_type });
}

void ParserScope::check_return_type(std::optional<ValueType> const& _rtn_type) const
{
	if (_rtn_type.has_value())
	{
		if (!rtn_type.has_value())
			throw "found return type '" + night::to_str(*_rtn_type) + "', expected void return type";

		if (!compare_relative_vt(*rtn_type, *_rtn_type))
			throw "found return type '" + night::to_str(*_rtn_type) + "', expected return type '" + night::to_str(*rtn_type) + "'";
	}
	else
	{
		if (rtn_type.has_value())
			throw "found void return type, expected return type '" + night::to_str(*rtn_type) + "'";
	}
}

std::optional<ValueType> const& ParserScope::get_curr_rtn_type() const
{
	return rtn_type;
}

void ParserScope::set_curr_rtn_type(std::optional<ValueType> const& _curr_rtn_type)
{
	rtn_type = _curr_rtn_type;
}