#include "utils.hpp"
#include "value_type.hpp"
#include "debug.hpp"

#include <string>

value_t token_var_type_to_val_type(std::string const& type)
{
	if (type == "bool") return (value_t)ValueType::BOOL;
	else if (type == "char") return (value_t)ValueType::CHAR;
	else if (type == "int") return (value_t)ValueType::INT;
	else if (type == "float") return (value_t)ValueType::FLOAT;
	else if (type == "str") return (value_t)ValueType::STRING;
	else throw debug::unhandled_case(type);
}