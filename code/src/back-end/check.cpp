#include "back-end/check.hpp"

#include <string>
#include <vector>
#include <stdexcept>

std::pair<std::string const, CheckFunction> make_check_function(
	std::string const& name,
	std::vector<TypeContainer> const& params,
	TypeContainer			   const& rtn_types)
{
	return { name, { params, rtn_types } };
}

std::pair<std::string const, CheckClass> make_check_class(
	std::string const& name,

	CheckVariableContainer const& vars,
	CheckFunctionContainer const& methods)
{
	return { name, { vars, methods } };
}



Type::Type(T _type)
	: type(_type) {}

Type::Type(T _type, TypeContainer const& _elem_types)
	: type(_type), elem_types(_elem_types) {}

std::string Type::to_str() const
{
	switch (type)
	{
	case T::BOOL:
		return "bool";
	case T::INT:
		return "int";
	case T::FLOAT:
		return "float";
	case T::STR:
		return "str";
	case T::ARR:
		return "arr";
	case T::RNG:
		return "rng";
	default:
		throw std::runtime_error("Type::to_str(), missing type to string conversion");
	}

}

bool Type::operator==(Type _t) const
{
	return type == _t.type;
}
