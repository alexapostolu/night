#include "common/type.hpp"
#include "common/debug.hpp"

#include <string>

Type::Type(std::string const& _prim, int _dim)
	: dim(_dim)
{
	if (_prim == "bool")	   prim = Primitive::BOOL;
	else if (_prim == "char")  prim = Primitive::CHAR;
	else if (_prim == "int")   prim = Primitive::INT;
	else if (_prim == "float") prim = Primitive::FLOAT;
	else throw debug::unhandled_case(_prim);
}

Type::Type(Primitive _prim, int _dim)
	: prim(_prim), dim(_dim) {}

bool Type::operator==(Primitive _prim) const
{
	return prim == _prim && !is_arr();
}

bool Type::is_prim() const { return !is_arr(); }

bool Type::is_arr() const { return dim; }

bool Type::is_str() const { return prim == Primitive::CHAR && dim == 1; }

bool is_same(std::optional<Type> const& type1, std::optional<Type> const& type2)
{
	return is_same_or_primitive(type1, type2) && type1->prim == type2->prim;
}

bool is_same_or_primitive(std::optional<Type> const& type1, std::optional<Type> const& type2)
{
	if (!type1.has_value() || !type2.has_value())
		return true;

	return type1->dim == type2->dim;
}

std::string night::to_str(Type const& type)
{
	if (type.prim == Type::CHAR && type.dim == 1)
		return "string";

	std::string is_arr_str = (type.is_arr() ? " array" : "");
	switch (type.prim)
	{
	case Type::BOOL:  return "boolean" + is_arr_str;
	case Type::CHAR:  return "character" + is_arr_str;
	case Type::INT:   return "integer" + is_arr_str;
	case Type::FLOAT: return "float" + is_arr_str;
	default: throw debug::unhandled_case(type.prim);
	}
}