#include "value_type.hpp"
#include "debug.hpp"

#include <string>

bool compare_value_t(value_t type1, value_t type2)
{
	return (type1 <= primitive_count && type2 <= primitive_count) ||
		   (type1 == type2);
}

bool is_object_t(value_t type)
{
	return type > primitive_count;
}

std::string night::to_str(value_t type, bool primitive)
{
	if (primitive)
	{
		switch (type)
		{
		case (value_t)ValueType::BOOL:
		case (value_t)ValueType::CHAR:
		case (value_t)ValueType::INT:
		case (value_t)ValueType::STRING:
			return "primitive";
		default:
			throw debug::unhandled_case((int)type);
		}
	}
	else
	{
		switch (type)
		{
		case (value_t)ValueType::BOOL:  return "bool";
		case (value_t)ValueType::CHAR:  return "char";
		case (value_t)ValueType::INT:	return "int";
		case (value_t)ValueType::STRING: return "string";
		default:
			throw debug::unhandled_case((int)type);
		}
	}
}