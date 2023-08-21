#include "value_type.hpp"
#include "debug.hpp"

#include <string>

ValueType::ValueType(_type _type, bool _is_arr)
	: type((value_t)_type), is_arr(_is_arr) {}

ValueType::ValueType(value_t _type, bool _is_arr)
	: type(_type), is_arr(_is_arr) {}

bool ValueType::operator==(ValueType const& _vt) const
{
	return type == _vt.type && is_arr == _vt.is_arr;
}

bool ValueType::is_object() const
{
	return type > primitive_count;
}

bool compare_value_t(ValueType vt1, ValueType vt2)
{
	return (!vt1.is_object() && !vt2.is_object()) ||
		   (vt1 == vt2);
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
		case ValueType::BOOL:
		case ValueType::CHAR:
		case ValueType::INT:
		case ValueType::STRING:
			return "primitive";
		default:
			throw debug::unhandled_case((int)type);
		}
	}
	else
	{
		switch (type)
		{
		case ValueType::BOOL:  return "bool";
		case ValueType::CHAR:  return "char";
		case ValueType::INT:	return "int";
		case ValueType::STRING: return "string";
		default:
			throw debug::unhandled_case((int)type);
		}
	}
}

std::string night::to_str(ValueType vt, bool primitive)
{
	return night::to_str(vt.type, primitive);
}