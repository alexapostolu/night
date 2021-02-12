#include "../../include/back-end/token.hpp"

std::string VariableType::to_str() const
{
	switch (type)
	{
	case VariableType::BOOL:
		return "bool";
	case VariableType::INT:
		return "int";
	case VariableType::FLOAT:
		return "float";
	case VariableType::STR:
		return "string";
	case VariableType::ARRAY:
		return "array";
	case VariableType::CLASS:
		return class_name;
	}

	return {};
}

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

bool CheckFunction::operator==(const std::string& _name) const
{
	return name == _name;
}

bool CheckClass::operator==(const std::string& _name) const
{
	return name == _name;
}

bool Token::operator==(const TokenType& _type) const
{
	return type == _type;
}

bool CheckVariable::operator==(const std::string& _name) const
{
	return name == _name;
}