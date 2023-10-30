#include "value_type.hpp"
#include "debug.hpp"

#include <string>

ValueType::ValueType(std::string const& _type, array_dim const& _dim)
	: dim(_dim)
{
	if (_type == "bool")	   type = PrimType::BOOL;
	else if (_type == "char")  type = PrimType::CHAR;
	else if (_type == "int")   type = PrimType::INT;
	else if (_type == "float") type = PrimType::FLOAT;
	else throw debug::unhandled_case(_type);
}

ValueType::ValueType(PrimType _type, array_dim const& _dim)
	: type(_type), dim(_dim) {}

bool ValueType::operator==(PrimType _type) const
{
	return dim.empty() && type == _type;
}

bool ValueType::is_arr() const { return !dim.empty(); }
bool ValueType::is_str() const { return type == PrimType::CHAR && dim.size() == 1; }

bool is_same(ValueType const& vt1, ValueType const& vt2)
{
	if (vt1.type != vt2.type || vt1.dim.size() != vt2.dim.size())
		return false;

	for (auto i = 0; i < vt1.dim.size(); ++i)
	{
		if ((!vt1.dim[i].has_value() && !vt2.dim[i].has_value()) ||
			(vt1.dim[i] != vt2.dim[i]))
			return false;
	}

	return true;
}

bool is_same_or_primitive(ValueType const& vt1, ValueType const& vt2)
{
	if (vt1.dim.empty() && vt2.dim.empty())
		return true;

	if (vt1.type != vt2.type || vt1.dim.size() != vt2.dim.size())
		return false;

	for (auto i = 0; i < vt1.dim.size(); ++i)
	{
		if ((!vt1.dim[i].has_value() && !vt2.dim[i].has_value()) ||
			(vt1.dim[i] != vt2.dim[i]))
			return false;
	}

	return true;
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
	default: throw debug::unhandled_case(vt.type);
	}

	if (!vt.dim.empty())
		return std::to_string(vt.dim.size()) + " dimensional " + type_s + " array";

	return type_s;
}