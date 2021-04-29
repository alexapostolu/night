#include "../../include/back-end/token.hpp"
#include "../../include/back-end/utils.hpp"
#include "../../include/error.hpp"

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

Scope::Scope(const std::shared_ptr<Scope>& _upper_scope)
	: upper_scope(_upper_scope) {}

CheckVariable::CheckVariable(const VariableTypeContainer& _types)
	: types(_types) {}

bool CheckVariable::find_type(const VariableType& var_type) const
{
	return types.find(var_type) != types.end();
}

std::pair<const std::string, CheckFunction> make_check_function(
	std::string const& name,
	std::vector<TypeContainer> const& params,
	TypeContainer const& rtn_types
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