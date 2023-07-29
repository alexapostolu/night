#include "value.hpp"
#include "bytecode.hpp"

#include <stdexcept>
#include <variant>

val::Value::Value(ValueType _type, int _data)
	: type((value_t)_type)
{
	if (type == (value_t)ValueType::U_INT)
		data = (uint64_t)_data;
	else
		data = (uint64_t)_data;
}

bool compare_value_t(val::value_t type1, val::value_t type2)
{
	return (type1 <= primitive_count && type2 <= primitive_count) ||
		   (type1 == type2);
}

bool is_object_t(val::value_t type)
{
	return type > primitive_count;
}

std::string night::to_str(val::value_t type, bool primitive)
{
	if (primitive)
	{
		switch (type)
		{
		case (val::value_t)val::ValueType::BOOL:
		case (val::value_t)val::ValueType::CHAR:
		case (val::value_t)val::ValueType::S_INT:
		case (val::value_t)val::ValueType::U_INT:
			return "primitive";
		default:
			// return classes[type];
			return "object";
		}
	}
	else
	{
		switch (type)
		{
		case (val::value_t)val::ValueType::BOOL:
			return "bool";
		case (val::value_t)val::ValueType::CHAR:
			return "char";
		case (val::value_t)val::ValueType::S_INT:
			return "signed int";
		case (val::value_t)val::ValueType::U_INT:
			return "unsigned int";
		default:
			// return classes[type];
			return "object";
		}
	}
}