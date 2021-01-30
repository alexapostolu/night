#include "../../include/back-end/token.hpp"

bool VariableType::operator==(const Type& _type) const
{
	return type == _type;
}

bool VariableType::operator!=(const Type& _type) const
{
	return type != _type;
}

bool VariableType::operator==(const VariableType& _type) const
{
	return type == _type.type;
}

bool VariableType::operator!=(const VariableType& _type) const
{
	return type != _type.type;
}

bool Conditional::is_else() const
{
	return condition == nullptr;
}