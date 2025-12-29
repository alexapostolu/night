#include "parser/statement_scope.hpp"
#include "parser/ast/expression.hpp"
#include "common/type.hpp"
#include "common/error.hpp"
#include "language.hpp"

#include <limits>
#include <optional>
#include <string>
#include <assert.h>
#include <unordered_set>

scope_func_container StatementScope::functions = {
	{ "print", StatementFunction{ PredefinedFunctions::PRINT_BOOL, {}, { Primitive::BOOL }, std::nullopt } },
	{ "print", StatementFunction{ PredefinedFunctions::PRINT_CHAR, {}, { Primitive::CHAR }, std::nullopt } },
	{ "print", StatementFunction{ PredefinedFunctions::PRINT_INT8, {}, { Primitive::INT8 }, std::nullopt } },
	{ "print", StatementFunction{ PredefinedFunctions::PRINT_INT16, {}, { Primitive::INT16 }, std::nullopt } },
	{ "print", StatementFunction{ PredefinedFunctions::PRINT_INT32, {}, { Primitive::INT32 }, std::nullopt } },
	{ "print", StatementFunction{ PredefinedFunctions::PRINT_INT64, {}, { Primitive::INT64 }, std::nullopt } },
	{ "print", StatementFunction{ PredefinedFunctions::PRINT_uINT8, {}, { Primitive::uINT8 }, std::nullopt } },
	{ "print", StatementFunction{ PredefinedFunctions::PRINT_uINT16, {}, { Primitive::uINT16 }, std::nullopt } },
	{ "print", StatementFunction{ PredefinedFunctions::PRINT_uINT32, {}, { Primitive::uINT32 }, std::nullopt } },
	{ "print", StatementFunction{ PredefinedFunctions::PRINT_uINT64, {}, { Primitive::uINT64 }, std::nullopt } },
	{ "print", StatementFunction{ PredefinedFunctions::PRINT_FLOAT, {}, { Primitive::FLOAT }, std::nullopt } },
	{ "print", StatementFunction{ PredefinedFunctions::PRINT_STR, {}, { Type(Primitive::CHAR, 1) }, std::nullopt } },

	{ "input", StatementFunction{ PredefinedFunctions::INPUT, {}, {}, Type(Primitive::CHAR, 1) } },

	{ "char",  StatementFunction{ PredefinedFunctions::INT8_TO_CHAR, {}, { Primitive::INT8 }, Primitive::CHAR } },
	{ "char",  StatementFunction{ PredefinedFunctions::INT16_TO_CHAR, {}, { Primitive::INT16 }, Primitive::CHAR } },
	{ "char",  StatementFunction{ PredefinedFunctions::INT32_TO_CHAR, {}, { Primitive::INT32 }, Primitive::CHAR } },
	{ "char",  StatementFunction{ PredefinedFunctions::INT64_TO_CHAR, {}, { Primitive::INT64 }, Primitive::CHAR } },
	{ "char",  StatementFunction{ PredefinedFunctions::uINT8_TO_CHAR, {}, { Primitive::uINT8 }, Primitive::CHAR } },
	{ "char",  StatementFunction{ PredefinedFunctions::uINT16_TO_CHAR, {}, { Primitive::uINT16 }, Primitive::CHAR } },
	{ "char",  StatementFunction{ PredefinedFunctions::uINT32_TO_CHAR, {}, { Primitive::uINT32 }, Primitive::CHAR } },
	{ "char",  StatementFunction{ PredefinedFunctions::uINT64_TO_CHAR, {}, { Primitive::uINT64 }, Primitive::CHAR } },
	{ "char",  StatementFunction{ PredefinedFunctions::STR_TO_CHAR, {}, { Type(Primitive::CHAR, 1) }, Primitive::CHAR } },

	{ "int8",   StatementFunction{ PredefinedFunctions::BOOL_TO_INT8, {}, { Primitive::BOOL }, Primitive::INT8 } },
	{ "int8",   StatementFunction{ PredefinedFunctions::CHAR_TO_INT8, {}, { Primitive::CHAR }, Primitive::INT8 } },
	{ "int8",   StatementFunction{ PredefinedFunctions::FLOAT_TO_INT8, {}, { Primitive::FLOAT }, Primitive::INT8 } },
	{ "int8",   StatementFunction{ PredefinedFunctions::STR_TO_INT8, {}, { Type(Primitive::CHAR, 1) }, Primitive::INT8 } },
	{ "int16",   StatementFunction{ PredefinedFunctions::BOOL_TO_INT16, {}, { Primitive::BOOL }, Primitive::INT16 } },
	{ "int16",   StatementFunction{ PredefinedFunctions::CHAR_TO_INT16, {}, { Primitive::CHAR }, Primitive::INT16 } },
	{ "int16",   StatementFunction{ PredefinedFunctions::FLOAT_TO_INT16, {}, { Primitive::FLOAT }, Primitive::INT16 } },
	{ "int16",   StatementFunction{ PredefinedFunctions::STR_TO_INT16, {}, { Type(Primitive::CHAR, 1) }, Primitive::INT16 } },
	{ "int32",   StatementFunction{ PredefinedFunctions::BOOL_TO_INT32, {}, { Primitive::BOOL }, Primitive::INT32 } },
	{ "int32",   StatementFunction{ PredefinedFunctions::CHAR_TO_INT32, {}, { Primitive::CHAR }, Primitive::INT32 } },
	{ "int32",   StatementFunction{ PredefinedFunctions::FLOAT_TO_INT32, {}, { Primitive::FLOAT }, Primitive::INT32 } },
	{ "int32",   StatementFunction{ PredefinedFunctions::STR_TO_INT32, {}, { Type(Primitive::CHAR, 1) }, Primitive::INT32 } },
	{ "int64",   StatementFunction{ PredefinedFunctions::BOOL_TO_INT64, {}, { Primitive::BOOL }, Primitive::INT64 } },
	{ "int64",   StatementFunction{ PredefinedFunctions::CHAR_TO_INT64, {}, { Primitive::CHAR }, Primitive::INT64 } },
	{ "int64",   StatementFunction{ PredefinedFunctions::FLOAT_TO_INT64, {}, { Primitive::FLOAT }, Primitive::INT64 } },
	{ "int64",   StatementFunction{ PredefinedFunctions::STR_TO_INT64, {}, { Type(Primitive::CHAR, 1) }, Primitive::INT64 } },

	{ "uint8",   StatementFunction{ PredefinedFunctions::BOOL_TO_uINT8, {}, { Primitive::BOOL }, Primitive::uINT8 } },
	{ "uint8",   StatementFunction{ PredefinedFunctions::CHAR_TO_uINT8, {}, { Primitive::CHAR }, Primitive::uINT8 } },
	{ "uint8",   StatementFunction{ PredefinedFunctions::FLOAT_TO_uINT8, {}, { Primitive::FLOAT }, Primitive::uINT8 } },
	{ "uint8",   StatementFunction{ PredefinedFunctions::STR_TO_uINT8, {}, { Type(Primitive::CHAR, 1) }, Primitive::uINT8 } },
	{ "uint16",   StatementFunction{ PredefinedFunctions::BOOL_TO_uINT16, {}, { Primitive::BOOL }, Primitive::uINT16 } },
	{ "uint16",   StatementFunction{ PredefinedFunctions::CHAR_TO_uINT16, {}, { Primitive::CHAR }, Primitive::uINT16 } },
	{ "uint16",   StatementFunction{ PredefinedFunctions::FLOAT_TO_uINT16, {}, { Primitive::FLOAT }, Primitive::uINT16 } },
	{ "uint16",   StatementFunction{ PredefinedFunctions::STR_TO_uINT16, {}, { Type(Primitive::CHAR, 1) }, Primitive::uINT16 } },
	{ "uint32",   StatementFunction{ PredefinedFunctions::BOOL_TO_uINT32, {}, { Primitive::BOOL }, Primitive::uINT32 } },
	{ "uint32",   StatementFunction{ PredefinedFunctions::CHAR_TO_uINT32, {}, { Primitive::CHAR }, Primitive::uINT32 } },
	{ "uint32",   StatementFunction{ PredefinedFunctions::FLOAT_TO_uINT32, {}, { Primitive::FLOAT }, Primitive::uINT32 } },
	{ "uint32",   StatementFunction{ PredefinedFunctions::STR_TO_uINT32, {}, { Type(Primitive::CHAR, 1) }, Primitive::uINT32 } },
	{ "uint64",   StatementFunction{ PredefinedFunctions::BOOL_TO_uINT64, {}, { Primitive::BOOL }, Primitive::uINT64 } },
	{ "uint64",   StatementFunction{ PredefinedFunctions::CHAR_TO_uINT64, {}, { Primitive::CHAR }, Primitive::uINT64 } },
	{ "uint64",   StatementFunction{ PredefinedFunctions::FLOAT_TO_uINT64, {}, { Primitive::FLOAT }, Primitive::uINT64 } },
	{ "uint64",   StatementFunction{ PredefinedFunctions::STR_TO_uINT64, {}, { Type(Primitive::CHAR, 1) }, Primitive::uINT64 } },

	{ "float", StatementFunction{ PredefinedFunctions::BOOL_TO_FLOAT, {}, { Primitive::BOOL }, Primitive::FLOAT } },
	{ "float", StatementFunction{ PredefinedFunctions::CHAR_TO_FLOAT, {}, { Primitive::CHAR }, Primitive::FLOAT } },
	{ "float", StatementFunction{ PredefinedFunctions::INT8_TO_FLOAT, {}, { Primitive::INT8 }, Primitive::FLOAT } },
	{ "float", StatementFunction{ PredefinedFunctions::INT16_TO_FLOAT, {}, { Primitive::INT16 }, Primitive::FLOAT } },
	{ "float", StatementFunction{ PredefinedFunctions::INT32_TO_FLOAT, {}, { Primitive::INT32 }, Primitive::FLOAT } },
	{ "float", StatementFunction{ PredefinedFunctions::INT64_TO_FLOAT, {}, { Primitive::INT64 }, Primitive::FLOAT } },
	{ "float", StatementFunction{ PredefinedFunctions::uINT8_TO_FLOAT, {}, { Primitive::uINT8 }, Primitive::FLOAT } },
	{ "float", StatementFunction{ PredefinedFunctions::uINT16_TO_FLOAT, {}, { Primitive::uINT16 }, Primitive::FLOAT } },
	{ "float", StatementFunction{ PredefinedFunctions::uINT32_TO_FLOAT, {}, { Primitive::uINT32 }, Primitive::FLOAT } },
	{ "float", StatementFunction{ PredefinedFunctions::uINT64_TO_FLOAT, {}, { Primitive::uINT64 }, Primitive::FLOAT } },
	{ "float", StatementFunction{ PredefinedFunctions::STR_TO_FLOAT, {}, { Type(Primitive::CHAR, 1) }, Primitive::FLOAT } },

	{ "str",   StatementFunction{ PredefinedFunctions::CHAR_TO_STR, {}, { Primitive::CHAR }, Type(Primitive::CHAR, 1) } },
	{ "str",   StatementFunction{ PredefinedFunctions::INT8_TO_STR, {}, { Primitive::INT8 }, Type(Primitive::CHAR, 1) } },
	{ "str",   StatementFunction{ PredefinedFunctions::INT16_TO_STR, {}, { Primitive::INT16 }, Type(Primitive::CHAR, 1) } },
	{ "str",   StatementFunction{ PredefinedFunctions::INT32_TO_STR, {}, { Primitive::INT32 }, Type(Primitive::CHAR, 1) } },
	{ "str",   StatementFunction{ PredefinedFunctions::INT64_TO_STR, {}, { Primitive::INT64 }, Type(Primitive::CHAR, 1) } },
	{ "str",   StatementFunction{ PredefinedFunctions::uINT8_TO_STR, {}, { Primitive::uINT8 }, Type(Primitive::CHAR, 1) } },
	{ "str",   StatementFunction{ PredefinedFunctions::uINT16_TO_STR, {}, { Primitive::uINT16 }, Type(Primitive::CHAR, 1) } },
	{ "str",   StatementFunction{ PredefinedFunctions::uINT32_TO_STR, {}, { Primitive::uINT32 }, Type(Primitive::CHAR, 1) } },
	{ "str",   StatementFunction{ PredefinedFunctions::uINT64_TO_STR, {}, { Primitive::uINT64 }, Type(Primitive::CHAR, 1) } },
	{ "str",   StatementFunction{ PredefinedFunctions::FLOAT_TO_STR, {}, { Primitive::FLOAT }, Type(Primitive::CHAR, 1) } },

	{ "len",   StatementFunction{ PredefinedFunctions::LEN, {}, { Type(Primitive::CHAR, 1) }, Primitive::INT32 } }
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
	{
		night::error::get().create_minor_error(
			"Variable '" + name + "' is already defined in the current or parent scope.", name_location);
		return std::nullopt;
	}

	if (variable_id == max_variables)
	{
		night::error::get().create_minor_error(
			"Only " + std::to_string(max_variables) + " variables are allowed per scope.", name_location);
		return std::nullopt;
	}

	variables[name] = { variable_id, type, 0 };
	variables[name].type.set_category(TypeCategory::Addressable);

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