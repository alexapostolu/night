#include "../../include/back-end/stmt.hpp"
#include "../../include/back-end/parser.hpp"
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