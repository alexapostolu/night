#include "expression.hpp"
#include "bytecode.hpp"
#include "parser_scope.hpp"
#include "parser.hpp"
#include "error.hpp"

#include <optional>
#include <memory>

Expression::Expression(
	ExpressionType _type,
	Location const& _loc,
	std::shared_ptr<Expression> const& _lhs = nullptr,
	std::shared_ptr<Expression> const& _rhs = nullptr)
	: type(_type), loc(_loc), lhs(_lhs), rhs(_rhs) {}

bool Expression::is_operator() const { return type == ExpressionType::BINARY_OP || type == ExpressionType::UNARY_OP; };
bool Expression::is_value() const { return type == ExpressionType::BRACKET || type == ExpressionType::UNARY_OP || type == ExpressionType::BINARY_OP; };


Bracket::Bracket(
	Location const& _loc,
	std::shared_ptr<Expression> const& _expr)
	: Expression(ExpressionType::BRACKET, _loc), expr(_expr) {}

void Bracket::insert_node(std::shared_ptr<Expression> const& node)
{
	expr->insert_node(node);
}

bytecodes_t Bracket::generate_codes(ParserScope const& scope) const
{
	return expr->generate_codes(scope);
}

std::optional<value_t> Bracket::type_check(ParserScope const& scope) const
{
	return expr->type_check(scope);
}


BinaryOp::BinaryOp(
	Location const& _loc,
	BinaryOpType _type,
	std::shared_ptr<Expression> const& _lhs,
	std::shared_ptr<Expression> const& _rhs)
	: Expression(ExpressionType::BINARY_OP, _loc, _lhs, _rhs), type(_type) {}

void BinaryOp::insert_node(
	std::shared_ptr<Expression>& prev,
	std::shared_ptr<Expression> const& node)
{
	if (!lhs)
		lhs = node;
	else if (!rhs)
		rhs = node;
	else
		rhs->insert_node(this->rhs, node);
}

bytecodes_t BinaryOp::generate_codes(ParserScope const& scope) const
{
	bytecodes_t codes;
	
	auto codes_lhs = lhs->generate_codes(scope);
	codes.insert(std::end(codes), std::begin(codes_lhs), std::end(codes_lhs));

	auto codes_rhs = rhs->generate_codes(scope);
	codes.insert(std::end(codes), std::begin(codes_rhs), std::end(codes_rhs));

	switch (type)
	{
	case BinaryOpType::ADD:
		codes.push_back((bytecode_t)BytecodeType::ADD);
	case BinaryOpType::SUB:
		codes.push_back((bytecode_t)BytecodeType::SUB);
	case BinaryOpType::MULT:
		codes.push_back((bytecode_t)BytecodeType::MULT);
	case BinaryOpType::DIV:
		codes.push_back((bytecode_t)BytecodeType::DIV);
	default:
		night::unhandled_case(type);
	}

	return codes;
}

std::optional<value_t> BinaryOp::type_check(ParserScope const& scope) const
{
	if (type == BinaryOpType::DOT)
	{
		auto lhs_type = lhs->type_check(scope);
		if (lhs_type.has_value() && is_object_t(*lhs_type))
			night::error::get().create_minor_error("variable '' can not be used with operator +", loc);

		auto rhs_type = rhs->type_check(scope);
		if (rhs_type.has_value() && is_object_t(*rhs_type))
			night::error::get().create_minor_error("variable '' can not be used with operator +", loc);
	}
	else
	{
		auto lhs_type = lhs->type_check(scope);
		if (lhs_type.has_value() && is_object_t(*lhs_type))
			night::error::get().create_minor_error("variable '' can not be used with operator +", loc);

		auto rhs_type = rhs->type_check(scope);
		if (rhs_type.has_value() && is_object_t(*rhs_type))
			night::error::get().create_minor_error("variable '' can not be used with operator +", loc);

		if (*lhs_type == (value_t)ValueType::S_INT || *rhs_type == (value_t)ValueType::S_INT)
			return (value_t)ValueType::S_INT;
		if (*lhs_type == (value_t)ValueType::U_INT || *rhs_type == (value_t)ValueType::U_INT)
			return (value_t)ValueType::U_INT;
		if (*lhs_type == (value_t)ValueType::CHAR || *rhs_type == (value_t)ValueType::CHAR)
			return (value_t)ValueType::CHAR;
		return (value_t)ValueType::BOOL;
	}
}


UnaryOp::UnaryOp(
	Location const& _loc,
	UnaryOpType _type,
	std::shared_ptr<Expression> const& _expr = nullptr)
	: Expression(ExpressionType::UNARY_OP, _loc), type(_type), expr(_expr) {}

void UnaryOp::insert_node(
	std::shared_ptr<Expression>& prev,
	std::shared_ptr<Expression> const& node)
{
	if (!expr)
		expr = node;
	else if (node->precedence() > precedence())
		expr->insert_node(expr, node);
	else
	{
		prev = node;
		node->insert_node(prev, std::make_shared<Expression>(this));
	}
}

bytecodes_t UnaryOp::generate_codes(ParserScope const& scope) const
{
	bytecodes_t codes = expr->generate_codes(scope);

	switch (type)
	{
	case UnaryOpType::NEGATIVE:
		codes.push_back((bytecode_t)BytecodeType::NEGATIVE);
		break;
	case UnaryOpType::NOT:
		codes.push_back((bytecode_t)BytecodeType::NOT);
		break;
	default:
		night::unhandled_case(type);
	}

	return codes;
}

std::optional<value_t> UnaryOp::type_check(ParserScope const& scope) const
{
	auto type = expr->type_check(scope);
	if (type.has_value() && is_object_t(*type))
		night::error::get().create_minor_error("expression under operator ! has type '" + val_type_to_str(*type) + "', expected primitive type", loc);

	return type;
}











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