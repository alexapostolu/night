#include "ast/expression.hpp"
#include "bytecode.hpp"
#include "parser_scope.hpp"
#include "parser.hpp"
#include "error.hpp"
#include "debug.hpp"

#include <optional>
#include <memory>
#include <iostream>
#include <assert.h>

expr::Expression::Expression(
	expr::ExpressionType _type, Location const& _loc)
	: guard(false), type(_type), loc(_loc) {}

bool expr::Expression::is_operator() const { return type == ExpressionType::BINARY_OP || type == ExpressionType::UNARY_OP; };
bool expr::Expression::is_value() const { return type == ExpressionType::BRACKET || type == ExpressionType::UNARY_OP || type == ExpressionType::BINARY_OP; };

int expr::Expression::subscript_prec = 0;
int expr::Expression::unary_op_prec  = 10;
int expr::Expression::bin_op_prec    = 100;
int expr::Expression::single_prec	 = 1000;



//expr::SubscriptOp::SubscriptOp(
//	Location const& _loc,
//	expr::expr_p const& _arr,
//	expr::expr_p const& _index)
//	: Expression(ExpressionType::UNARY_OP, _loc), arr(_arr), index(_index) {}
//
//void expr::SubscriptOp::insert_node(
//	expr::expr_p const& node,
//	expr::expr_p* prev)
//{
//	if (!arr)
//	{
//		arr = node;
//	}
//	else if (node->precedence() > precedence())
//	{
//		expr->insert_node(node, &expr);
//	}
//	else
//	{
//		node->insert_node(std::make_shared<UnaryOp>(loc, type, expr));
//		*prev = node;
//	}
//}
//
//std::optional<value_t> expr::SubscriptOp::type_check(ParserScope const& scope)
//{
//	auto type = expr->type_check(scope);
//	if (type.has_value() && is_object_t(*type))
//		night::error::get().create_minor_error("expression under operator ! has type '" + night::to_str(*type) + "', expected primitive type", loc);
//
//	return type;
//}
//
//bytecodes_t expr::SubscriptOp::generate_codes() const
//{
//	bytecodes_t codes = expr->generate_codes();
//
//	return codes;
//}
//
//int expr::SubscriptOp::precedence() const
//{
//	if (guard)
//		return single_prec;
//
//	return subscript_prec;
//}


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

std::optional<value_t> expr::UnaryOp::type_check(ParserScope const& scope)
{
	auto type = expr->type_check(scope);
	if (type.has_value() && is_object_t(*type))
		night::error::get().create_minor_error("expression under operator ! has type '" + night::to_str(*type) + "', expected primitive type", loc);

	return type;
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
	else if (_type == "<")
		type = BinaryOpType::LESSER;
	else if (_type == ">")
		type = BinaryOpType::GREATER;
	/*else if (_type == ".")
		type = BinaryOpType::DOT;*/
	else if (_type == "[")
		type = BinaryOpType::SUBSCRIPT;
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

std::optional<value_t> expr::BinaryOp::type_check(ParserScope const& scope)
{
	assert(lhs);
	auto lhs_type = lhs->type_check(scope);
	if (lhs_type.has_value() && is_object_t(*lhs_type))
		night::error::get().create_minor_error("variable '' can not be used with operator +", loc);

	assert(rhs);
	auto rhs_type = rhs->type_check(scope);
	if (rhs_type.has_value() && is_object_t(*rhs_type))
		night::error::get().create_minor_error("variable '' can not be used with operator +", loc);

	if (!lhs_type.has_value() || !rhs_type.has_value())
		return std::nullopt;

	if (type == BinaryOpType::SUBSCRIPT && *lhs_type == (value_t)ValueType::INT && *rhs_type == (value_t)ValueType::STRING)
		return (value_t)ValueType::STRING;
	if (type == BinaryOpType::SUBSCRIPT && *lhs_type == (value_t)ValueType::INT && *rhs_type == (value_t)ValueType::ARRAY)
		return (value_t)ValueType::STRING;
	if (*lhs_type == (value_t)ValueType::STRING && *lhs_type == *rhs_type)
		return (value_t)ValueType::STRING;
	if (*lhs_type == (value_t)ValueType::INT || *rhs_type == (value_t)ValueType::INT)
		return (value_t)ValueType::INT;
	if (*lhs_type == (value_t)ValueType::CHAR || *rhs_type == (value_t)ValueType::CHAR)
		return (value_t)ValueType::CHAR;
	return (value_t)ValueType::BOOL;
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
	case BinaryOpType::LESSER:
		codes.push_back((bytecode_t)BytecodeType::LESSER);
		break;
	case BinaryOpType::GREATER:
		codes.push_back((bytecode_t)BytecodeType::GREATER);
		break;
	case BinaryOpType::SUBSCRIPT:
		codes.push_back((bytecode_t)BytecodeType::SUBSCRIPT);
		break;
	default:
		throw debug::unhandled_case((int)type);
	}

	return codes;
}

int expr::BinaryOp::precedence() const
{
	if (guard)
		return single_prec;

	switch (type)
	{
	case BinaryOpType::LESSER:
	case BinaryOpType::GREATER:
		return bin_op_prec + 1;
	case BinaryOpType::ADD:
	case BinaryOpType::SUB:
		return bin_op_prec + 2;
	case BinaryOpType::MULT:
	case BinaryOpType::DIV:
		return bin_op_prec + 3;
	case BinaryOpType::SUBSCRIPT:
		return bin_op_prec + 4;
	default:
		throw debug::unhandled_case((int)type);
	}
}



expr::Array::Array(
	Location const& _loc,
	std::vector<expr_p> const& _arr)
	: Expression(ExpressionType::ARRAY, _loc), arr(_arr) {}

void expr::Array::insert_node(
	std::shared_ptr<expr::Expression> const& node,
	std::shared_ptr<expr::Expression>* prev)
{
	node->insert_node(std::make_shared<expr::Array>(loc, arr));
	*prev = node;
}

std::optional<value_t> expr::Array::type_check(ParserScope const& scope)
{
	std::optional<value_t> arr_type(std::nullopt);

	for (auto const& elem : arr)
	{
		auto elem_type = elem->type_check(scope);

		if (arr_type.has_value() && elem_type.has_value() && !compare_value_t(*arr_type, *elem_type))
			night::error::get().create_minor_error("all values of an array must be the same", loc);
		else if (!arr_type.has_value())
			arr_type = elem_type;
	}

	return arr_type;
}

bytecodes_t expr::Array::generate_codes() const
{
	bytecodes_t codes;

	for (auto const& elem : arr)
	{
		auto elem_codes = elem->generate_codes();
		codes.insert(std::end(codes), std::begin(elem_codes), std::end(elem_codes));
	}

	codes.push_back((bytecode_t)BytecodeType::ARR);
	codes.push_back((bytecode_t)arr.size());

	return codes;
}

int expr::Array::precedence() const
{
	return single_prec;
}


expr::Variable::Variable(
	Location const& _loc,
	std::string const& _name)
	: Expression(ExpressionType::VARIABLE, _loc), name(_name), id(std::nullopt) {}

void expr::Variable::insert_node(
	std::shared_ptr<expr::Expression> const& node,
	std::shared_ptr<expr::Expression>* prev)
{
	node->insert_node(std::make_shared<expr::Variable>(loc, name));
	*prev = node;
}

std::optional<value_t> expr::Variable::type_check(ParserScope const& scope)
{
	id = scope.vars.at(name).id;
	return scope.vars.at(name).type;
}

bytecodes_t expr::Variable::generate_codes() const
{
	assert(id.has_value());
	return { (bytecode_t)BytecodeType::LOAD, *id };
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

std::optional<value_t> expr::Value::type_check(ParserScope const& scope)
{
	return type;
}

bytecodes_t expr::Value::generate_codes() const
{
	switch ((ValueType)type)
	{
	case ValueType::BOOL:
		return { (bytecode_t)BytecodeType::BOOL, val == "true" };
	case ValueType::CHAR:
		return { (bytecode_t)BytecodeType::CHAR1, (bytecode_t)std::stoi(val) };
	case ValueType::INT:
	{
		return int_to_bytecodes(std::stoll(val));
	}
	case ValueType::FLOAT: {
		break;
	}
	case ValueType::STRING: {
		bytecodes_t codes{ (bytecode_t)BytecodeType::STR };
		
		auto length_codes = Value::int_to_bytecodes(val.length());
		codes.insert(std::end(codes), std::begin(length_codes), std::end(length_codes));

		for (char c : val)
			codes.push_back((bytecode_t)c);

		return codes;
	}
	default:
		throw debug::unhandled_case((int)(type));
	}
}

int expr::Value::precedence() const
{
	return single_prec;
}

bytecodes_t expr::Value::int_to_bytecodes(int64_t int64)
{
	bytecodes_t codes;

	if (int64 <= std::numeric_limits<uint8_t>::max())
		codes.push_back((bytecode_t)BytecodeType::S_INT1);
	else if (int64 <= std::numeric_limits<uint16_t>::max())
		codes.push_back((bytecode_t)BytecodeType::S_INT2);
	else if (int64 <= std::numeric_limits<uint32_t>::max())
		codes.push_back((bytecode_t)BytecodeType::S_INT4);
	else if (int64 <= std::numeric_limits<uint64_t>::max())
		codes.push_back((bytecode_t)BytecodeType::S_INT8);
	else
		throw debug::unhandled_case(int64);

	do {
		codes.push_back(int64 & 0xFF);
	} while (int64 >>= 8);

	return codes;
}