#include "expression.hpp"

Expr::Expr(ExprType _type)
	: type(_type) {}

std::optional<std::shared_ptr<Expr>&> Expr::next()
{
	return std::nullopt;
}

int Expr::prec() const
{
	return -1;
}

ExprValue::ExprValue(ExprValueType _type, std::variant<ExprConstant, expr_p> const& _val)
	: Expr(ExprType::VALUE), type(_type), val(_val) {}

ExprUnary::ExprUnary(ExprUnaryType _type, expr_p const& _val)
	: Expr(ExprType::UNARY), type(_type), val(_val) {}

std::optional<expr_p&> ExprUnary::next()
{
	return val;
}

ExprBinary::ExprBinary(ExprBinaryType _type, expr_p const& _lhs, expr_p const& _rhs)
	: Expr(ExprType::BINARY), type(_type), lhs(_lhs), rhs(_rhs) {}

std::optional<expr_p&> ExprBinary::next()
{
	return rhs;
}

int ExprBinary::prec() const
{
	return (int)type;
}