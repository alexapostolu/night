#include "expression.hpp"
#include "bytecode.hpp"

#include <memory>
#include <stdexcept>

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
	: Expr(ExprType::UNARY, _val, nullptr), type(_type) {}

expr_p& ExprUnary::next()
{
	return lhs;
}

bytecode_t ExprUnary::to_bytecode() const
{
	BytecodeType op_type;
	switch (type)
	{
	case ExprUnaryType::NOT:
		op_type = BytecodeType::NOT;
		break;
	}

	return std::make_shared<Bytecode>(op_type);
}



ExprBinary::ExprBinary(ExprBinaryType _type, expr_p const& _lhs, expr_p const& _rhs)
	: Expr(ExprType::BINARY, _lhs, _rhs), type(_type) {}

bytecode_t ExprBinary::to_bytecode() const
{
	BytecodeType op_type;
	switch (type)
	{
	case ExprBinaryType::ADD:
		op_type = BytecodeType::ADD;
		break;
	case ExprBinaryType::SUB:
		op_type = BytecodeType::SUB;
		break;
	case ExprBinaryType::MULT:
		op_type = BytecodeType::MULT;
		break;
	case ExprBinaryType::DIV:
		op_type = BytecodeType::DIV;
		break;
	}

	return std::make_shared<Bytecode>(op_type);
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
		throw std::runtime_error("unhandled case");
	}
}

int prec(ExprBinaryType type)
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
		throw std::runtime_error("unhandled case");
	}
}