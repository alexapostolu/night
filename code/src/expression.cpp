#include "expression.hpp"

Expr::Expr(ExprType _type)
	: type(_type) {}

ExprValue::ExprValue(ExprValueType _type, std::variant<ExprConstant, expr_p> const& _val)
	: Expr(ExprType::VALUE), type(_type), val(_val) {}

expr_p& ExprValue::next()
{
	return nullptr;
}

int ExprValue::prec() const
{
	return 0;
}

ExprUnary::ExprUnary(ExprUnaryType _type, expr_p const& _val)
	: Expr(ExprType::UNARY), type(_type), val(_val) {}

expr_p& ExprUnary::next()
{
	return val;
}

int ExprUnary::prec() const
{
	return (int)type;
}

ExprBinary::ExprBinary(ExprBinaryType _type, std::vector<expr_p> const& _lhs, std::vector<expr_p> const& _rhs)
	: Expr(ExprType::Binary), type(_type), lhs(_lhs), rhs(_rhs) {}

expr_p& ExprBinary::next()
{
	return rhs;
}

int ExprBinary::prec() const
{
	return (int)type;
}