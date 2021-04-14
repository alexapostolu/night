#include "../../include/back-end/token.hpp"
#include "../../include/back-end/utils.hpp"
#include "../../include/error.hpp"

VariableType::VariableType(const Type& _type)
	: type(_type) {}

VariableType::VariableType(const std::unordered_set<VariableType, HashVariableType>& _elem_types)
	: type(VariableType::ARRAY), special(_elem_types)
{

}

VariableType::VariableType(const std::string& _class_name)
	: type(CLASS), special(_class_name) {}

VariableType::~VariableType() {}

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
		return "arr";
	case VariableType::CLASS:
		return class_name;
	default:
		assert(false);
		return {};
	}
}

std::unordered_set<VariableType, HashVariableType>& VariableType::get_elem_types()
{
	assert(type == VariableType::ARRAY);
	return std::get<std::unordered_set<VariableType, HashVariableType> >(special);
}

bool VariableType::operator==(const VariableType& _type) const
{
	return type == _type.type;
}

bool VariableType::operator!=(const VariableType& _type) const
{
	return type != _type.type;
}

std::size_t HashVariableType::operator()(const VariableType& _type) const
{
	return std::hash<int>()(_type.type);
}

bool Token::operator==(const TokenType& _type) const
{
	return type == _type;
}

Scope::Scope(const std::shared_ptr<Scope>& _upper_scope)
	: upper_scope(_upper_scope) {}

CheckVariable::CheckVariable(const VariableTypeContainer& _types)
	: types(_types) {}

bool CheckVariable::find_type(const VariableType& var_type) const
{
	return types.find(var_type) != types.end();
}

std::pair<const std::string, CheckFunction> make_check_function(
	const std::string& name,
	const std::vector<VariableTypeContainer>& params,
	const VariableTypeContainer& rtn_types
) {
	return {
		name,
		CheckFunction{ params, rtn_types, rtn_types.empty() }
	};
}

std::pair<const std::string, CheckClass> make_check_class(
	const std::string& name,
	const CheckVariableContainer& vars,
	const CheckFunctionContainer& methods
) {
	return {
		name,
		CheckClass{ vars, methods }
	};
}