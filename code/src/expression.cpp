#include "expression.hpp"
#include "expression.hpp"
#include "bytecode.hpp"
#include "scope.hpp"
#include "parser.hpp"
#include "error.hpp"

#include <memory>
#include <stdexcept>
#include <limits>
#include <string>

Expr::Expr(ExprType _type, expr_p const& _lhs, expr_p const& _rhs)
	: type(_type), lhs(_lhs), rhs(_rhs), gaurd(false) {}

expr_p& Expr::next() { return lhs; }
int Expr::prec() const { return -1; }
void Expr::set_guard() { gaurd = true; };



ExprValue::ExprValue(ValueType _type, Value const& _val)
	: Expr(ExprType::VALUE, nullptr, nullptr), val(_val) {}

bytecodes_t ExprValue::to_bytecode() const
{
	switch (val.type)
	{
	case ValueType::BOOL:
		return { (bytecode_t)BytecodeType::BOOL, (bytecode_t)val.data };
	case ValueType::CHAR:
		return { (bytecode_t)BytecodeType::CHAR1, (bytecode_t)val.data };
	case ValueType::INT:
		bytecodes_t codes;
		number_to_bytecode(val.data, codes);
		return codes;
	}
}

std::optional<ValueType> ExprValue::type_check(Scope const& scope) const
{
	return val.type;
}


ExprVar::ExprVar(std::string const& _name)
	: Expr(ExprType::VALUE, nullptr, nullptr), name(_name), index(-1) {}

bytecodes_t ExprVar::to_bytecode() const
{
	bytecodes_t codes;
	codes.push_back((bytecode_t)BytecodeType::VARIABLE);
	number_to_bytecode(codes, index);

	return codes;
}

std::optional<ValueType> ExprVar::type_check(Scope const& scope) const
{
	if (!scope.vars.contains(name))
		night::error::get().create_minor_error("variable '" + name + "' not defined", loc);
	
	bytecodes_t codes;
	codes.push_back((bytecode_t)BytecodeType::VARIABLE);
	number_to_bytecode(codes, scope.vars.at(name).id);

	return codes;
}



ExprUnary::ExprUnary(ExprUnaryType _type, expr_p const& _val)
	: Expr(ExprType::UNARY, _val, nullptr), unary_type(_type) {}

expr_p& ExprUnary::next()
{
	return lhs;
}

bytecodes_t ExprUnary::to_bytecode() const
{
	switch (unary_type)
	{
	case ExprUnaryType::NEGATIVE:
		return { (bytecode_t)BytecodeType::NEGATIVE };
	case ExprUnaryType::NOT:
		return { (bytecode_t)BytecodeType::NOT };
	default:
		night::unhandled_case(unary_type);
	}
}

int ExprUnary::prec() const
{
	return 100;
}



ExprBinary::ExprBinary(ExprBinaryType _type, expr_p const& _lhs, expr_p const& _rhs)
	: Expr(ExprType::BINARY, _lhs, _rhs), binary_type(_type), guard(false) {}

bytecodes_t ExprBinary::to_bytecode() const
{
	switch (binary_type)
	{
	case ExprBinaryType::ADD:
		return { (bytecode_t)BytecodeType::ADD };
	case ExprBinaryType::SUB:
		return { (bytecode_t)BytecodeType::SUB };
	case ExprBinaryType::MULT:
		return { (bytecode_t)BytecodeType::MULT };
	case ExprBinaryType::DIV:
		return { (bytecode_t)BytecodeType::DIV };
	default:
		night::unhandled_case(binary_type);
	}
}

std::optional<ValueType> ExprBinary::type_check(Scope const& scope) const
{
	auto lhs_type = lhs->type_check(scope);
	if (!lhs_type)
		return std::nullopt;

	auto rhs_type = rhs->type_check(scope);
	if (!rhs_type)
		return std::nullopt;

	if (binary_type == ExprBinaryType::DOT)
	{
		
	}
}

expr_p& ExprBinary::next()
{
	return rhs;
}

int ExprBinary::prec() const
{
	if (guard)
		return 100;

	switch (binary_type)
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

void ExprBinary::set_guard()
{
	// guard is set when this expr contains brackets
	// nothing has higher precedence than brackets, so we guard this expr
	guard = true;
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

std::string const& expr_bin_type_to_string(ExprBinaryType type)
{
	switch (type)
	{
	case ExprBinaryType::ADD:
		return "+";
	case ExprBinaryType::SUB:
		return "-";
	case ExprBinaryType::MULT:
		return "*";
	case ExprBinaryType::DIV:
		return "/";
	}
}

void number_to_bytecode(bytecodes_t& codes, int64_t num)
{
	if (num >= 0)
	{
		uint64_t uint64 = num;

		if (uint64 <= std::numeric_limits<uint8_t>::max())
			codes.push_back((bytecode_t)BytecodeType::U_INT1);
		else if (uint64 <= std::numeric_limits<uint16_t>::max())
			codes.push_back((bytecode_t)BytecodeType::U_INT2);
		else if (uint64 <= std::numeric_limits<uint32_t>::max())
			codes.push_back((bytecode_t)BytecodeType::U_INT4);
		else if (uint64 <= std::numeric_limits<uint64_t>::max())
			codes.push_back((bytecode_t)BytecodeType::U_INT8);
		else {}

		do {
			codes.push_back(uint64 & 0xFF);
		} while (uint64 >>= 8);
	}
	else
	{
		int64_t int64 = num;

		if (int64 <= std::numeric_limits<uint8_t>::max())
			codes.push_back((bytecode_t)BytecodeType::S_INT1);
		else if (int64 <= std::numeric_limits<uint16_t>::max())
			codes.push_back((bytecode_t)BytecodeType::S_INT2);
		else if (int64 <= std::numeric_limits<uint32_t>::max())
			codes.push_back((bytecode_t)BytecodeType::S_INT4);
		else if (int64 <= std::numeric_limits<uint64_t>::max())
			codes.push_back((bytecode_t)BytecodeType::S_INT8);
		else {}

		do {
			codes.push_back(int64 & 0xFF);
		} while (int64 >>= 8);
	}
}