#include "../../include/back-end/token.hpp"
#include "../../include/error.hpp"

VariableType::VariableType(
	const Type& _type, const std::string& _name
) : type(_type), class_name(_name) {}

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
	case VariableType::STRING:
		return "str";
	case VariableType::ARRAY:
		return "array";
	case VariableType::CLASS:
		return class_name;
	default:
		assert(false);
		return {};
	}
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

Scope::Scope(const std::shared_ptr<Scope>& _upper_scope)
	: upper_scope(_upper_scope) {}

CheckVariable::CheckVariable(const VariableTypeContainer& _types, const bool _flag_array)
	: types(_types), is_array(_flag_array) {}

bool CheckVariable::find_type(const VariableType& var_type) const
{
	return types.find(var_type) != types.end();
}

bool CheckVariable::is_param() const
{
	return !is_array && types.empty();
}

std::pair<const std::string, CheckFunction> make_check_function(
	const std::string& name,
	const std::vector<VariableTypeContainer>& params = {},
	const VariableTypeContainer& rtn_types = {}
) {
	return {
		name,
		CheckFunction{ params, rtn_types, rtn_types.empty() }
	};
}

std::pair<const std::string, CheckClass> make_check_class(
	const std::string& name,
	const CheckVariableContainer& vars = {},
	const CheckFunctionContainer& methods = {}
) {
	return {
		name,
		CheckClass{ vars, methods }
	};
}