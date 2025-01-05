#include "statement_scope.hpp"
#include "ast/expression.hpp"
#include "type.hpp"
#include "error.hpp"
#include "function.h"

#include <limits>
#include <optional>
#include <string>
#include <assert.h>

unsigned int StatementScope::max_var_id = 0;

scope_func_container StatementScope::functions = {
	{ "print", StatementFunction{ 0,  {}, { Type::BOOL		    }, std::nullopt			} },
	{ "print", StatementFunction{ 1,  {}, { Type::CHAR		    }, std::nullopt			} },
	{ "print", StatementFunction{ 2,  {}, { Type::INT		    }, std::nullopt			} },
	{ "print", StatementFunction{ 3,  {}, { Type::FLOAT		    }, std::nullopt			} },
	{ "print", StatementFunction{ 4,  {}, { Type(Type::CHAR, 1) }, std::nullopt			} },
	{ "input", StatementFunction{ 5,  {}, {						}, Type(Type::CHAR, 1 ) } },
	{ "char",  StatementFunction{ 6,  {}, { Type::INT			}, Type::CHAR			} },
	{ "int",   StatementFunction{ 7,  {}, { Type(Type::CHAR, 1) }, Type::INT			} },
	{ "int",   StatementFunction{ 8,  {}, { Type::CHAR			}, Type::INT			} },
	{ "str",   StatementFunction{ 9,  {}, { Type::INT			}, Type(Type::CHAR, 1)  } },
	{ "str",   StatementFunction{ 10, {}, { Type::FLOAT			}, Type(Type::CHAR, 1)  } },
	{ "len",   StatementFunction{ 11, {}, { Type(Type::CHAR, 1) }, Type::INT			} }
};

StatementScope::StatementScope()
	: vars(), variable_id(0) {}

StatementScope::StatementScope(StatementScope const& upper_scope)
	: vars(upper_scope.vars), rtn_type(upper_scope.rtn_type), variable_id(0) {}

StatementScope::StatementScope(StatementScope const& upper_scope, std::optional<Type> const& _rtn_type)
	: vars(upper_scope.vars), rtn_type(_rtn_type), variable_id(0) {}

std::optional<uint64_t> StatementScope::create_variable(
	std::string const& name,
	Type const& type,
	Location const& loc)
{
	if (vars.contains(name))
		night::error::get().create_minor_error("variable '" + name + "' is already defined", loc);

	if (variable_id == std::numeric_limits<unsigned int>::max())
		night::error::get().create_minor_error("only " + std::to_string(std::numeric_limits<unsigned int>::max()) +
			" variables allowed per scope and its children scope", loc);

	if (night::error::get().has_minor_errors())
		return std::nullopt;

	vars[name] = { variable_id, type, 0 };

	variable_id++;

	if (variable_id > max_var_id)
		max_var_id = variable_id;

	return variable_id;
}

StatementVariable const* StatementScope::get_var(std::string const& name)
{
	auto var = vars.find(name);

	if (var == std::end(vars))
		return nullptr;

	++vars[name].times_used;
	return &var->second;
}

scope_func_container::iterator StatementScope::create_function(
	std::string const& name,
	std::vector<std::string> const& param_names,
	std::vector<Type> const& param_types,
	std::optional<Type> const& rtn_type,
	std::vector<uint64_t> _param_ids)
{
	static uint64_t func_id = StatementScope::functions.size();

	auto [it, range_end] = StatementScope::functions.equal_range(name);

	for (; it != range_end; ++it)
	{
		if (std::equal(std::begin(param_types), std::end(param_types),
					   std::begin(it->second.param_types), std::end(it->second.param_types),
					   is_same))
			throw "function is already defined";
	}

	return functions.emplace(name, StatementFunction{ func_id++, param_names, param_types, rtn_type, _param_ids });
}