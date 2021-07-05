#include "../../include/back-end/stmt.hpp";
#include "../../include/back-end/parser.hpp"
#include "../../include/back-end/utils.hpp"
#include "../../include/error.hpp"

#include <memory>
#include <algorithm>

std::shared_ptr<ExprNode>& ExprNode::travel_ast()
{
	return type == ExprNode::UNARY_OP
		? std::get<UnaryOPNode>(data).value
		: std::get<BinaryOPNode>(data).right;
}

bool ExprNode::is_value() const
{
	return type == ExprNode::LITERAL  || type == ExprNode::ARRAY ||
		   type == ExprNode::VARIABLE || type == ExprNode::CALL;
}

template <typename VarContainer>
std::pair<std::string const, VarContainer>* Scope<VarContainer>::get_var(
	std::string const& name)
{
	for (Scope* curr_scope = &scope; curr_scope != nullptr;)
	{
		for (auto& var : curr_scope->vars)
		{
			if (var.first == var_name)
				return &var;
		}

		curr_scope = &curr_scope->upper_scope;
	}

	return nullptr;
}