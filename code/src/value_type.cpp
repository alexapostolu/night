#include "value_type.hpp"
#include "debug.hpp"

#include <string>
#include <memory>
#include <iostream>

ValueType::ValueType(PrimType _type, int _dim)
	: type(_type), dim(_dim) {}

bool ValueType::operator==(PrimType _type) const
{
	return !dim && type == _type;
}

bool ValueType::is_prim() const
{
	return !dim && type <= primitive_count;
}

bool compare_absolute_vt(ValueType const& vt1, ValueType const& vt2)
{
	return vt1.dim == vt2.dim && vt1.type == vt2.type;
}

bool compare_relative_vt(ValueType const& vt1, ValueType const& vt2)
{
	return vt1.dim == vt2.dim &&
		((vt1.type <= primitive_count && vt2.type <= primitive_count) ||
			vt1.type == vt2.type);
}

std::string night::to_str(ValueType const& vt)
{
	std::string type_s;
	switch (vt.type)
	{
	case ValueType::BOOL:  type_s = "bool"; break;
	case ValueType::CHAR:  type_s = "char"; break;
	case ValueType::INT:   type_s = "int"; break;
	case ValueType::FLOAT: type_s = "float"; break;
	case ValueType::STR:   type_s = "str"; break;
	default:
		throw debug::unhandled_case(vt.type);
	}

	if (vt.dim)
		return std::to_string(vt.dim) + " dimensional " + type_s + " array";

	return type_s;
}