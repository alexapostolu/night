#include "parser/statement_scope.hpp"
#include "parser/ast/expression.hpp"
#include "common/type.hpp"
#include "common/error.hpp"

#include <limits>
#include <optional>
#include <string>
#include <assert.h>
#include <unordered_set>

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
	{ "str",   StatementFunction{ 11, {}, { Type::CHAR			}, Type(Type::CHAR, 1)	} },
	{ "len",   StatementFunction{ 12, {}, { Type(Type::CHAR, 1) }, Type::INT			} }
};

StatementScope::StatementScope()
	: variables() {}

StatementScope::StatementScope(
	StatementScope const& parent_scope)
	: variables(parent_scope.variables)
	, return_type(parent_scope.return_type) {}

StatementScope::StatementScope(
	StatementScope const& parent_scope,
	std::optional<Type> const& _return_type)
	: variables(parent_scope.variables)
	, return_type(_return_type) {}

std::optional<night::id_t> StatementScope::create_variable(
	std::string const& name,
	Location	const& name_location,
	Type const& type)
{
	static night::id_t variable_id = 0;
	static night::id_t const max_variables = std::numeric_limits<night::id_t>::max();

	if (variables.contains(name))
		night::error::get().create_minor_error(
			"Variable '" + name + "' is already defined in the current or parent scope.", name_location);

	if (variable_id == max_variables)
		night::error::get().create_minor_error(
			"Only " + std::to_string(max_variables) + " variables are allowed per scope.", name_location);

	if (night::error::get().has_minor_errors())
		return std::nullopt;

	variables[name] = { variable_id, type, 0 };

	return variable_id++;
}

StatementVariable const* StatementScope::get_variable(
	std::string const& name,
	Location const& name_location)
{
	/*
	 * When an undefined variable is used in multiple locations, Night will only
	 * generate one error for that undefined variable.
	 * 
	 * If a variable is already in the undefined variables set, then no error
	 * will be generated.
	 */
	static std::unordered_set<std::string> undefined_variables;

	auto variable = variables.find(name);

	// Check to see if variable exists.
	if (variable == std::end(variables))
	{
		if (!undefined_variables.contains(name))
		{
			night::error::get().create_minor_error(
				"Variable '" + name + "' is undefined.",
				name_location
			);

			undefined_variables.insert(name);
		}

		return nullptr;
	}

	variable->second.times_used += 1;

	return &variable->second;
}

std::optional<night::id_t> StatementScope::create_function(
	std::string const& name,
	Location	const& name_location,
	std::vector<std::string> const& param_names,
	std::vector<Type>		 const& param_types,
	std::optional<Type> const& _return_type)
{
	static night::id_t function_id = StatementScope::functions.size();
	static night::id_t const max_functions = std::numeric_limits<night::id_t>::max();

	// Check if function has been defined before by searching if there is any
	// function with the same name and parameters.

	auto [it, range_end] = StatementScope::functions.equal_range(name);

	for (; it != range_end; ++it)
	{
		if (std::equal(std::begin(param_types), std::end(param_types),
					   std::begin(it->second.param_types), std::end(it->second.param_types)))
		{
			night::error::get().create_minor_error(
				"Function '" + name + "' has already been defined.", name_location);
			break;
		}
	}

	if (function_id == max_functions)
		night::error::get().create_minor_error(
			"Only " + std::to_string(max_functions) + " functions are allowed.", name_location);

	if (night::error::get().has_minor_errors())
		return std::nullopt;

	functions.emplace(name, StatementFunction{ function_id , param_names, param_types, _return_type });

	return function_id++;
}