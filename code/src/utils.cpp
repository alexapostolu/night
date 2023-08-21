#include "utils.hpp"
#include "value_type.hpp"
#include "debug.hpp"

#include <string>

ValueType token_var_type_to_val_type(std::string const& type, std::source_location const& s_loc)
{
	if (type == "bool") return ValueType::BOOL;
	else if (type == "char") return ValueType::CHAR;
	else if (type == "int") return ValueType::INT;
	else if (type == "float") return ValueType::FLOAT;
	else if (type == "str") return ValueType::STRING;
	else throw debug::unhandled_case("type", s_loc);
}