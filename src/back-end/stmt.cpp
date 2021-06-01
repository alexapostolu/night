#include "../../include/back-end/stmt.hpp";
#include "../../include/back-end/parser.hpp"
#include "../../include/back-end/utils.hpp"
#include "../../include/error.hpp"

#include <memory>
#include <algorithm>




void BinaryOPNode::eval_num_types(
	Parser::ParserScope& scope) const
{
	std::vector<Type> const left_types =
		left->eval_types(scope, { Type::INT, Type::FLOAT });

	if (!night::contains(left_types, Type::INT, Type::FLOAT))
		throw_binary_op_err(left_types, "left", "types 'int' or 'float'");

	std::vector<Type> const right_types =
		right->eval_types(scope, { Type::INT, Type::FLOAT });
	 
	if (!night::contains(right_types, Type::INT, Type::FLOAT))
		throw_binary_op_err(left_types, "right", "types 'int' or 'float'");
}

void BinaryOPNode::throw_binary_op_err(
	Lexer& lexer,
	std::vector<Type> const& types,
	std::string const& side,
	std::string const& used_types) const
{
	throw NIGHT_COMPILE_ERROR(
		side + " hand value of operator '" + op + "' currently contains " + types_as_str(types),
		"operator '" + op + "' can only be used on " + used_types,
		night::learn_operators);
}



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