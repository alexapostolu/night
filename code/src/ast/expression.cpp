#include "ast/expression.hpp"
#include "bytecode.hpp"
#include "parser_scope.hpp"
#include "parser.hpp"
#include "error.hpp"
#include "debug.hpp"

#include <optional>
#include <memory>
#include <iostream>

expr::Expression::Expression(
	expr::ExpressionType _type, Location const& _loc)
	: guard(false), type(_type), loc(_loc) {}

bool expr::Expression::is_operator() const { return type == ExpressionType::BINARY_OP || type == ExpressionType::UNARY_OP; };
bool expr::Expression::is_value() const { return type == ExpressionType::BRACKET || type == ExpressionType::UNARY_OP || type == ExpressionType::BINARY_OP; };

int expr::Expression::unary_op_prec = 0;
int expr::Expression::bin_op_prec   = 100;
int expr::Expression::single_prec	= 1000;



expr::UnaryOp::UnaryOp(
	Location const& _loc,
	std::string const& _type,
	std::shared_ptr<Expression> const& _expr)
	: Expression(ExpressionType::UNARY_OP, _loc), expr(_expr)
{
	if (_type == "-")
		type = UnaryOpType::NEGATIVE;
	else if (_type == "!")
		type = UnaryOpType::NOT;
	else
		throw debug::unhandled_case(_type);
}

expr::UnaryOp::UnaryOp(
	Location const& _loc,
	UnaryOpType _type,
	std::shared_ptr<Expression> const& _expr)
	: Expression(ExpressionType::UNARY_OP, _loc), type(_type), expr(_expr) {}

void expr::UnaryOp::insert_node(
	std::shared_ptr<Expression> const& node,
	std::shared_ptr<Expression>* prev)
{
	if (!expr)
	{
		expr = node;
	}
	else if (node->precedence() > precedence())
	{
		expr->insert_node(node, &expr);
	}
	else
	{
		node->insert_node(std::make_shared<UnaryOp>(loc, type, expr));
		*prev = node;
	}
}

bytecodes_t expr::UnaryOp::generate_codes() const
{
	bytecodes_t codes = expr->generate_codes();

	switch (type)
	{
	case UnaryOpType::NEGATIVE:
		codes.push_back((bytecode_t)BytecodeType::NEGATIVE);
		break;
	case UnaryOpType::NOT:
		codes.push_back((bytecode_t)BytecodeType::NOT);
		break;
	default:
		throw debug::unhandled_case((int)type);
	}

	return codes;
}

std::optional<value_t> expr::UnaryOp::type_check(ParserScope const& scope) const
{
	auto type = expr->type_check(scope);
	if (type.has_value() && is_object_t(*type))
		night::error::get().create_minor_error("expression under operator ! has type '" + night::to_str(*type) + "', expected primitive type", loc);

	return type;
}

int expr::UnaryOp::precedence() const
{
	if (guard)
		return single_prec;

	switch (type)
	{
	case UnaryOpType::NOT:
	case UnaryOpType::NEGATIVE:
		return unary_op_prec + 1;
	default:
		throw debug::unhandled_case((int)type);
	}
}


expr::BinaryOp::BinaryOp(
	Location const& _loc,
	std::string const& _type,
	std::shared_ptr<Expression> const& _lhs,
	std::shared_ptr<Expression> const& _rhs)
	: Expression(ExpressionType::BINARY_OP, _loc), lhs(_lhs), rhs(_rhs)
{
	if (_type == "+")
		type = BinaryOpType::ADD;
	else if (_type == "-")
		type = BinaryOpType::SUB;
	else if (_type == "*")
		type = BinaryOpType::MULT;
	else if (_type == "/")
		type = BinaryOpType::DIV;
	else if (_type == ".")
		type = BinaryOpType::DOT;
	else
		throw debug::unhandled_case(_type);
}

expr::BinaryOp::BinaryOp(
	Location const& _loc,
	BinaryOpType _type,
	std::shared_ptr<Expression> const& _lhs,
	std::shared_ptr<Expression> const& _rhs)
	: Expression(ExpressionType::BINARY_OP, _loc), type(_type), lhs(_lhs), rhs(_rhs) {}

void expr::BinaryOp::insert_node(
	std::shared_ptr<Expression> const& node,
	std::shared_ptr<Expression>* prev)
{
	if (!lhs)
	{
		lhs = node;
	}
	else if (!rhs)
	{
		rhs = node;
	}
	else if (node->precedence() > precedence())
	{
		rhs->insert_node(node, &rhs);
	}
	else
	{
		node->insert_node(std::make_shared<BinaryOp>(loc, type, lhs, rhs));
		*prev = node;
	}
}

bytecodes_t expr::BinaryOp::generate_codes() const
{
	bytecodes_t codes;
	
	auto codes_lhs = lhs->generate_codes();
	codes.insert(std::end(codes), std::begin(codes_lhs), std::end(codes_lhs));

	auto codes_rhs = rhs->generate_codes();
	codes.insert(std::end(codes), std::begin(codes_rhs), std::end(codes_rhs));

	switch (type)
	{
	case BinaryOpType::ADD:
		codes.push_back((bytecode_t)BytecodeType::ADD);
		break;
	case BinaryOpType::SUB:
		codes.push_back((bytecode_t)BytecodeType::SUB);
		break;
	case BinaryOpType::MULT:
		codes.push_back((bytecode_t)BytecodeType::MULT);
		break;
	case BinaryOpType::DIV:
		codes.push_back((bytecode_t)BytecodeType::DIV);
		break;
	default:
		throw debug::unhandled_case((int)type);
	}

	return codes;
}

std::optional<value_t> expr::BinaryOp::type_check(ParserScope const& scope) const
{
	if (type == BinaryOpType::DOT)
	{
		auto lhs_type = lhs->type_check(scope);
		if (lhs_type.has_value() && is_object_t(*lhs_type))
			night::error::get().create_minor_error("variable '' can not be used with operator +", loc);

		auto rhs_type = rhs->type_check(scope);
		if (rhs_type.has_value() && is_object_t(*rhs_type))
			night::error::get().create_minor_error("variable '' can not be used with operator +", loc);

		return {};
	}
	else
	{
		auto lhs_type = lhs->type_check(scope);
		if (lhs_type.has_value() && is_object_t(*lhs_type))
			night::error::get().create_minor_error("variable '' can not be used with operator +", loc);

		auto rhs_type = rhs->type_check(scope);
		if (rhs_type.has_value() && is_object_t(*rhs_type))
			night::error::get().create_minor_error("variable '' can not be used with operator +", loc);

		if (*lhs_type == (value_t)ValueType::INT || *rhs_type == (value_t)ValueType::INT)
			return (value_t)ValueType::INT;
		//if (*lhs_type == (val::value_t)val::ValueType::U_INT || *rhs_type == (val::value_t)val::ValueType::U_INT)
			//return (val::value_t)val::ValueType::U_INT;
		if (*lhs_type == (value_t)ValueType::CHAR || *rhs_type == (value_t)ValueType::CHAR)
			return (value_t)ValueType::CHAR;
		return (value_t)ValueType::BOOL;
	}
}

int expr::BinaryOp::precedence() const
{
	if (guard)
		return single_prec;

	switch (type)
	{
	case BinaryOpType::ADD:
	case BinaryOpType::SUB:
		return bin_op_prec + 1;
	case BinaryOpType::MULT:
	case BinaryOpType::DIV:
		return bin_op_prec + 2;
	case BinaryOpType::DOT:
		return bin_op_prec + 3;
	default:
		throw debug::unhandled_case((int)type);
	}
}


expr::Variable::Variable(
	Location const& _loc,
	std::string const& _name,
	bytecode_t _id)
	: Expression(ExpressionType::VARIABLE, _loc), name(_name), id(_id) {}

void expr::Variable::insert_node(
	std::shared_ptr<expr::Expression> const& node,
	std::shared_ptr<expr::Expression>* prev)
{
	node->insert_node(std::make_shared<expr::Variable>(loc, name, id));
	*prev = node;
}

bytecodes_t expr::Variable::generate_codes() const
{
	return { (bytecode_t)BytecodeType::LOAD, id };
}

std::optional<value_t> expr::Variable::type_check(ParserScope const& scope) const
{
	return scope.vars.at(name).type;
}

int expr::Variable::precedence() const
{
	return single_prec;
}


expr::Value::Value(
	Location const& _loc,
	value_t _type,
	std::string const& _val)
	: Expression(ExpressionType::VALUE, _loc), type(_type), val(_val) {}

void expr::Value::insert_node(
	std::shared_ptr<Expression> const& node,
	std::shared_ptr<Expression>* prev)
{
	node->insert_node(std::make_shared<expr::Value>(loc, type, val));
	*prev = node;
}

bytecodes_t expr::Value::generate_codes() const
{
	switch ((ValueType)type)
	{
	case ValueType::BOOL:
		return { (bytecode_t)BytecodeType::BOOL, std::stod(val) != 0 };
	case ValueType::CHAR:
		return { (bytecode_t)BytecodeType::CHAR1, (bytecode_t)std::stoi(val) };
	case ValueType::INT:
	{
		bytecodes_t codes;
		int64_t int64 = std::stoi(val);

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

		return codes;
	}
	case ValueType::FLOAT: {
		break;
	}
	case ValueType::STRING: {
		break;
	}
	default:
		throw debug::unhandled_case((int)(type));
	}
}

std::optional<value_t> expr::Value::type_check(ParserScope const& scope) const
{
	return type;
}

int expr::Value::precedence() const
{
	return single_prec;
}