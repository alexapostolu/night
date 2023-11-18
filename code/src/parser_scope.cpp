#include "parser_scope.hpp"
#include "ast/expression.hpp"
#include "value_type.hpp"
#include "error.hpp"

#include <optional>
#include <string>
#include <assert.h>

scope_func_container ParserScope::funcs = {
	{ "print", ParserFunction{ 0,  {}, { ValueType::BOOL  }, std::nullopt    } },
	{ "print", ParserFunction{ 1,  {}, { ValueType::CHAR  }, std::nullopt    } },
	{ "print", ParserFunction{ 2,  {}, { ValueType::INT   }, std::nullopt    } },
	{ "print", ParserFunction{ 3,  {}, { ValueType::FLOAT }, std::nullopt    } },
	{ "print", ParserFunction{ 4,  {}, { ValueType(ValueType::CHAR, {std::nullopt})}, std::nullopt}},
	{ "input", ParserFunction{ 5,  {}, {},					 ValueType(ValueType::CHAR, {std::nullopt}) } },
	{ "char",  ParserFunction{ 6,  {}, { ValueType::INT   }, ValueType::CHAR } },
	{ "int",   ParserFunction{ 7,  {}, { value_type_str   }, ValueType::INT  } },
	{ "int",   ParserFunction{ 8,  {}, { ValueType::CHAR  }, ValueType::INT  } },
	{ "str",   ParserFunction{ 9,  {}, { ValueType::INT   }, value_type_str  } },
	{ "str",   ParserFunction{ 10, {}, { ValueType::FLOAT }, value_type_str  } },
	{ "len",   ParserFunction{ 11, {}, { value_type_str   }, ValueType::INT  } }
};

bool ParserScope::inside_false_conditional = false;

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

	vars[name] = { type, var_id, 0 };
	return var_id++;
}

ParserVariable const* ParserScope::get_var(std::string const& name)
{
	auto var = vars.find(name);

	if (var == std::end(vars))
		return nullptr;

	++vars[name].times_used;
	return &var->second;
}

bool ParserScope::is_var_used(std::string const& name) const
{
	return vars.at(name).times_used;
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
					   is_same))
			throw "function is already defined";
	}

	return funcs.emplace(name, ParserFunction{ func_id++, param_names, param_types, rtn_type });
}