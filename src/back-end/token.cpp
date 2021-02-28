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
		return "str";
	case VariableType::ARRAY:
		return "array";
	case VariableType::CLASS:
		return class_name;
	}

	return {};
}

bool VariableType::operator==(const VariableType& _type) const
{
	return type == _type.type;
}

bool VariableType::operator!=(const VariableType& _type) const
{
	return type != _type.type;
}

bool Token::operator==(const TokenType& _type) const
{
	return type == _type;
}

CheckVariable::CheckVariable()
	: flag_array(false) {}

CheckVariable::CheckVariable(const VariableTypeContainer& _types, const bool _flag_array)
	: types(_types), flag_array(_flag_array) {}

bool CheckVariable::find_type(const VariableType& var_type) const
{
	return types.find(var_type) != types.end();
}

bool CheckVariable::is_array() const
{
	return flag_array;
}

bool CheckVariable::is_param() const
{
	return !flag_array && types.empty();
}

void CheckVariable::set_array(bool flag)
{
	flag_array = flag;
}

Scope::Scope(const std::shared_ptr<Scope>& _upper_scope)
	: upper_scope(_upper_scope) {}