#include "expression.hpp"
#include "bytecode.hpp"

#include <memory>

Expr::Expr(ExprType _type, expr_p const& _lhs, expr_p const& _rhs)
	: type(_type), lhs(_lhs), rhs(_rhs) {}

expr_p& Expr::next() { return lhs; }
int Expr::prec() const { return -1; }



ExprValue::ExprValue(ValueType _type, std::variant<char, int> const& _val)
	: Expr(ExprType::VALUE, nullptr, nullptr), type(_type), val(_val) {}

bytecode_t ExprValue::to_bytecode() const
{
	return std::make_shared<CreateConstant>(type, val);
}



ExprUnary::ExprUnary(ExprUnaryType _type, expr_p const& _val)
	: Expr(ExprType::UNARY, nullptr, nullptr), type(_type), val(_val) {}

expr_p& ExprUnary::next()
{
	return lhs;
}

bytecode_t ExprUnary::to_bytecode() const
{
	OperationType op_type;
	switch (type)
	{
	case ExprUnaryType::NOT:
		op_type = OperationType::NOT;
		break;
	}

	return std::make_shared<Operation>(op_type);
}



ExprBinary::ExprBinary(ExprBinaryType _type, expr_p const& _lhs, expr_p const& _rhs)
	: Expr(ExprType::BINARY, lhs, rhs), type(_type) {}

bytecode_t ExprBinary::to_bytecode() const
{
	OperationType op_type;
	switch (type)
	{
	case ExprBinaryType::ADD:
		op_type = OperationType::ADD;
		break;
	case ExprBinaryType::SUB:
		op_type = OperationType::SUB;
		break;
	}

	return std::make_shared<Operation>(op_type);
}

expr_p& ExprBinary::next()
{
	return rhs;
}

int ExprBinary::prec() const
{
	switch (type)
	{
	case ExprBinaryType::ADD:
	case ExprBinaryType::SUB:
		return 1;

	case ExprBinaryType::MULT:
	case ExprBinaryType::DIV:
		return 2;

	default:
		throw "eyyy no";
	}
}