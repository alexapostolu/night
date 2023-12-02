#include "value_type.hpp"
#include "debug.hpp"

#include <string>

ValueType::ValueType(std::string const& _type, int _dim)
	: dim(_dim)
{
	if (_type == "bool")	   type = PrimType::BOOL;
	else if (_type == "char")  type = PrimType::CHAR;
	else if (_type == "int")   type = PrimType::INT;
	else if (_type == "float") type = PrimType::FLOAT;
	else throw debug::unhandled_case(_type);
}

ValueType::ValueType(PrimType _type, int _dim)
	: type(_type), dim(_dim) {}

bool ValueType::operator==(PrimType _type) const
{
	return type == _type && !is_arr();
}

bool ValueType::is_arr() const { return dim; }

bool ValueType::is_str() const { return type == PrimType::CHAR && is_arr(); }

bool is_same(std::optional<ValueType> const& vt1, std::optional<ValueType> const& vt2)
{
	if (!vt1.has_value() || !vt2.has_value())
		return true;

	return vt1->type == vt2->type && vt1->dim == vt2->dim;
}

bool is_same_or_primitive(std::optional<ValueType> const& vt1, std::optional<ValueType> const& vt2)
{
	if (!vt1.has_value() || !vt2.has_value())
		return true;

	return vt1->dim == vt2->dim;
}

std::string night::to_str(ValueType const& vt)
{
	if (vt.type == ValueType::CHAR && vt.is_arr())
		return "string";

	std::string is_arr_str = (vt.is_arr() ? " array" : "");
	switch (vt.type)
	{
	case ValueType::BOOL:  return "bool" + is_arr_str;
	case ValueType::CHAR:  return "char" + is_arr_str;
	case ValueType::INT:   return "int" + is_arr_str;
	case ValueType::FLOAT: return "float" + is_arr_str;
	default: throw debug::unhandled_case(vt.type);
	}
}