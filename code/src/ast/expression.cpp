#include "ast/expression.hpp"
#include "ast/ast.hpp"
#include "bytecode.hpp"
#include "parser_scope.hpp"
#include "parser.hpp"
#include "error.hpp"
#include "debug.hpp"

#include <algorithm>
#include <optional>
#include <memory>
#include <vector>
#include <iostream>
#include <assert.h>

expr::Expression::Expression(
	expr::ExpressionType _type, Location const& _loc)
	: guard(false), type(_type), loc(_loc) {}

bool expr::Expression::is_operator() const { return type == ExpressionType::BINARY_OP || type == ExpressionType::UNARY_OP; };
bool expr::Expression::is_value() const { return type == ExpressionType::BRACKET || type == ExpressionType::UNARY_OP || type == ExpressionType::BINARY_OP; };

int expr::Expression::bin_op_prec    = 10;
int expr::Expression::unary_op_prec  = 100;
int expr::Expression::single_prec	 = 1000;


expr::UnaryOp::UnaryOp(
	Location const& _loc,
	std::string const& _type,
	expr::expr_p const& _expr)
	: Expression(ExpressionType::UNARY_OP, _loc), expr(_expr)
{
	if		(_type == "-") type = UnaryOpType::NEGATIVE;
	else if (_type == "!") type = UnaryOpType::NOT;
	else throw debug::unhandled_case(_type);
}

expr::UnaryOp::UnaryOp(
	Location const& _loc,
	UnaryOpType _type,
	expr::expr_p const& _expr)
	: Expression(ExpressionType::UNARY_OP, _loc), type(_type), expr(_expr) {}

void expr::UnaryOp::insert_node(
	expr::expr_p const& node,
	expr::expr_p* prev)
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

std::optional<ValueType> expr::UnaryOp::type_check(ParserScope const& scope)
{
	assert(expr);

	auto expr_type = expr->type_check(scope);

	if (!expr_type)
		return std::nullopt;

	switch (type)
	{
	case UnaryOpType::NEGATIVE:
		if (!expr_type->dim)
		{
			switch (expr_type->type)
			{
			case ValueType::BOOL:
			case ValueType::CHAR:
			case ValueType::INT:
				return op_code = ValueType::INT;
			case ValueType::FLOAT:
				return op_code = ValueType::FLOAT;
			}
		}

		break;
		
	case UnaryOpType::NOT:
		if (!expr_type->dim)
		{
			switch (expr_type->type)
			{
			case ValueType::BOOL:
			case ValueType::CHAR:
			case ValueType::INT:
				op_code = ValueType::INT;
				return ValueType::BOOL;
			case ValueType::FLOAT:
				op_code = ValueType::FLOAT;
				return ValueType::BOOL;
			}
		}

		break;

	default:
		throw debug::unhandled_case((int)type);
	}

	night::error::get().create_minor_error("expression under unary operator has type '" + night::to_str(*expr_type) + "', expected primitive type", loc);
	return std::nullopt;
}

bytecodes_t expr::UnaryOp::generate_codes() const
{
	bytecodes_t codes = expr->generate_codes();

	switch (type)
	{
	case UnaryOpType::NEGATIVE:
		if (op_code == ValueType::INT)
			codes.push_back((bytecode_t)BytecodeType::NEGATIVE_I);
		if (op_code == ValueType::FLOAT)
			codes.push_back((bytecode_t)BytecodeType::NEGATIVE_F);

		break;

	case UnaryOpType::NOT:
		if (op_code == ValueType::INT)
			codes.push_back((bytecode_t)BytecodeType::NOT_I);
		if (op_code == ValueType::FLOAT)
			codes.push_back((bytecode_t)BytecodeType::NOT_F);

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
	expr::expr_p const& _lhs,
	expr::expr_p const& _rhs)
	: Expression(ExpressionType::BINARY_OP, _loc), lhs(_lhs), rhs(_rhs)
{
	if (_type == "+")		type = BinaryOpType::ADD;
	else if (_type == "-")  type = BinaryOpType::SUB;
	else if (_type == "*")  type = BinaryOpType::MULT;
	else if (_type == "/")  type = BinaryOpType::DIV;
	else if (_type == "<")  type = BinaryOpType::LESSER;
	else if (_type == ">")  type = BinaryOpType::GREATER;
	else if (_type == "<=") type = BinaryOpType::LESSER_EQUALS;
	else if (_type == ">=") type = BinaryOpType::GREATER_EQUALS;
	else if (_type == "==") type = BinaryOpType::EQUALS;
	else if (_type == "!=") type = BinaryOpType::NOT_EQUALS;
	else if (_type == "&&") type = BinaryOpType::AND;
	else if (_type == "||") type = BinaryOpType::OR;
	else if (_type == "[")  type = BinaryOpType::SUBSCRIPT;
	else throw debug::unhandled_case(_type);
}

expr::BinaryOp::BinaryOp(
	Location const& _loc,
	BinaryOpType _type,
	expr::expr_p const& _lhs,
	expr::expr_p const& _rhs)
	: Expression(ExpressionType::BINARY_OP, _loc), type(_type), lhs(_lhs), rhs(_rhs) {}

void expr::BinaryOp::insert_node(
	expr::expr_p const& node,
	expr::expr_p* prev)
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

std::optional<ValueType> expr::BinaryOp::type_check(ParserScope const& scope)
{
	assert(lhs);
	assert(rhs);

	auto lhs_type = lhs->type_check(scope);
	auto rhs_type = rhs->type_check(scope);

	if (!lhs_type.has_value() || !rhs_type.has_value())
		return std::nullopt;

	switch (type)
	{
	case BinaryOpType::ADD:
		if (!lhs_type->dim && !rhs_type->dim)
		{
			if (lhs_type == ValueType::STR && rhs_type == ValueType::STR)
				return op_code = ValueType::STR;
		}

	case BinaryOpType::SUB:
	case BinaryOpType::MULT:
	case BinaryOpType::DIV:
		if (lhs_type->is_prim() && rhs_type->is_prim())
		{
			if (lhs_type == ValueType::FLOAT)
			{
				if (rhs_type != ValueType::FLOAT)
					cast_rhs = BytecodeType::I2F;

				return op_code = ValueType::FLOAT;
			}

			if (rhs_type == ValueType::FLOAT)
			{
				if (lhs_type != ValueType::FLOAT)
					cast_rhs = BytecodeType::I2F;

				return op_code = ValueType::FLOAT;
			}

			return op_code = ValueType::INT;
		}

		break;

	case BinaryOpType::LESSER:
	case BinaryOpType::GREATER:
	case BinaryOpType::LESSER_EQUALS:
	case BinaryOpType::GREATER_EQUALS:
	case BinaryOpType::EQUALS:
	case BinaryOpType::NOT_EQUALS:
		if (!lhs_type->dim && !rhs_type->dim)
		{
			if (lhs_type == ValueType::STR && rhs_type == ValueType::STR)
			{
				op_code = ValueType::STR;
				return ValueType::BOOL;
			}
		}

		if (lhs_type->is_prim() && rhs_type->is_prim())
		{
			if (rhs_type == ValueType::FLOAT)
			{
				if (lhs_type != ValueType::FLOAT)
					cast_lhs = BytecodeType::I2F;

				op_code = ValueType::FLOAT;
				return ValueType::BOOL;
			}
			
			if (lhs_type == ValueType::FLOAT && rhs_type != ValueType::FLOAT)
			{
				if (lhs_type != ValueType::FLOAT)
					cast_lhs = BytecodeType::I2F;

				op_code = ValueType::FLOAT;
				return ValueType::BOOL;
			}

			op_code = ValueType::INT;
			return ValueType::BOOL;
		}

		break;

	case BinaryOpType::AND:
	case BinaryOpType::OR:
		if (lhs_type->is_prim() && rhs_type->is_prim())
		{
			if (rhs_type == ValueType::FLOAT)
			{
				if (lhs_type != ValueType::FLOAT)
					cast_lhs = BytecodeType::I2F;

				return ValueType::BOOL;
			}

			if (lhs_type == ValueType::FLOAT && rhs_type != ValueType::FLOAT)
			{
				if (lhs_type != ValueType::FLOAT)
					cast_lhs = BytecodeType::I2F;

				return ValueType::BOOL;
			}

			return ValueType::BOOL;
		}

		break;

	case BinaryOpType::SUBSCRIPT:
		if (lhs_type == ValueType::INT)
		{
			if (rhs_type == ValueType::STR)
				return ValueType::CHAR;

			if (rhs_type->dim)
				return ValueType(rhs_type->type, rhs_type->dim - 1);
		}

		break;

	default:
		throw debug::unhandled_case((int)type);
	}

	night::error::get().create_minor_error("type mismatch between '" + night::to_str(*lhs_type) + "' and '" + night::to_str(*rhs_type) + "'", loc);
	return std::nullopt;
}

bytecodes_t expr::BinaryOp::generate_codes() const
{
	bytecodes_t codes;

	auto codes_lhs = lhs->generate_codes();
	codes.insert(std::end(codes), std::begin(codes_lhs), std::end(codes_lhs));

	if (cast_lhs.has_value())
		codes.push_back((bytecode_t)*cast_lhs);
	
	auto codes_rhs = rhs->generate_codes();
	codes.insert(std::end(codes), std::begin(codes_rhs), std::end(codes_rhs));

	if (cast_rhs.has_value())
		codes.push_back((bytecode_t)*cast_rhs);

	switch (type)
	{
	case BinaryOpType::ADD:
		if (op_code == ValueType::INT)
			codes.push_back((bytecode_t)BytecodeType::ADD_I);
		else if (op_code == ValueType::FLOAT)
			codes.push_back((bytecode_t)BytecodeType::ADD_F);
		else if (op_code == ValueType::STR)
			codes.push_back((bytecode_t)BytecodeType::ADD_S);

		break;

	case BinaryOpType::SUB:
		if (op_code == ValueType::INT)
			codes.push_back((bytecode_t)BytecodeType::SUB_I);
		else if (op_code == ValueType::FLOAT)
			codes.push_back((bytecode_t)BytecodeType::SUB_F);

		break;

	case BinaryOpType::MULT:
		if (op_code == ValueType::INT)
			codes.push_back((bytecode_t)BytecodeType::MULT_I);
		else if (op_code == ValueType::FLOAT)
			codes.push_back((bytecode_t)BytecodeType::MULT_F);

		break;

	case BinaryOpType::DIV:
		if (op_code == ValueType::INT)
			codes.push_back((bytecode_t)BytecodeType::DIV_I);
		else if (op_code == ValueType::FLOAT)
			codes.push_back((bytecode_t)BytecodeType::DIV_F);

		break;

	case BinaryOpType::LESSER:
		if (op_code == ValueType::INT)
			codes.push_back((bytecode_t)BytecodeType::LESSER_I);
		else if (op_code == ValueType::FLOAT)
			codes.push_back((bytecode_t)BytecodeType::LESSER_F);
		else if (op_code == ValueType::STR)
			codes.push_back((bytecode_t)BytecodeType::LESSER_S);

		break;

	case BinaryOpType::GREATER:
		if (op_code == ValueType::INT)
			codes.push_back((bytecode_t)BytecodeType::GREATER_I);
		else if (op_code == ValueType::FLOAT)
			codes.push_back((bytecode_t)BytecodeType::GREATER_F);
		else if (op_code == ValueType::STR)
			codes.push_back((bytecode_t)BytecodeType::GREATER_S);

		break;

	case BinaryOpType::LESSER_EQUALS:
		if (op_code == ValueType::INT)
			codes.push_back((bytecode_t)BytecodeType::LESSER_EQUALS_I);
		else if (op_code == ValueType::FLOAT)
			codes.push_back((bytecode_t)BytecodeType::LESSER_EQUALS_F);
		else if (op_code == ValueType::STR)
			codes.push_back((bytecode_t)BytecodeType::LESSER_EQUALS_S);

		break;
	case BinaryOpType::GREATER_EQUALS:
		if (op_code == ValueType::INT)
			codes.push_back((bytecode_t)BytecodeType::GREATER_EQUALS_I);
		else if (op_code == ValueType::FLOAT)
			codes.push_back((bytecode_t)BytecodeType::GREATER_EQUALS_F);
		else if (op_code == ValueType::STR)
			codes.push_back((bytecode_t)BytecodeType::GREATER_EQUALS_S);
		break;
	case BinaryOpType::EQUALS:
		if (op_code == ValueType::INT)
			codes.push_back((bytecode_t)BytecodeType::EQUALS_I);
		else if (op_code == ValueType::FLOAT)
			codes.push_back((bytecode_t)BytecodeType::EQUALS_F);
		else if (op_code == ValueType::STR)
			codes.push_back((bytecode_t)BytecodeType::EQUALS_S);
		break;
	case BinaryOpType::NOT_EQUALS:
		if (op_code == ValueType::INT)
			codes.push_back((bytecode_t)BytecodeType::NOT_EQUALS_I);
		else if (op_code == ValueType::FLOAT)
			codes.push_back((bytecode_t)BytecodeType::NOT_EQUALS_F);
		else if (op_code == ValueType::STR)
			codes.push_back((bytecode_t)BytecodeType::NOT_EQUALS_S);
		break;
	case BinaryOpType::AND:
		codes.push_back((bytecode_t)BytecodeType::AND);
		break;
	case BinaryOpType::OR:
		codes.push_back((bytecode_t)BytecodeType::OR);
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
	case BinaryOpType::AND:
	case BinaryOpType::OR:
		return bin_op_prec + 1;
	case BinaryOpType::LESSER:
	case BinaryOpType::GREATER:
	case BinaryOpType::LESSER_EQUALS:
	case BinaryOpType::GREATER_EQUALS:
		return bin_op_prec + 2;
	case BinaryOpType::ADD:
	case BinaryOpType::SUB:
		return bin_op_prec + 3;
	case BinaryOpType::MULT:
	case BinaryOpType::DIV:
		return bin_op_prec + 4;
	case BinaryOpType::SUBSCRIPT:
		return bin_op_prec + 5;
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

std::optional<ValueType> expr::Array::type_check(ParserScope const& scope)
{
	std::optional<ValueType> arr_type;
	for (auto const& elem : arr)
	{
		assert(elem);

		auto elem_type = elem->type_check(scope);

		if (arr_type.has_value() && elem_type.has_value() && !compare_relative_vt(*arr_type, *elem_type))
			night::error::get().create_minor_error("all values of an array must be the same", loc);
		else if (!arr_type.has_value())
			arr_type = elem_type;
	}

	if (arr_type.has_value())
		arr_type->dim += 1;

	return arr_type;
}

bytecodes_t expr::Array::generate_codes() const
{
	bytecodes_t codes;

	for (auto const& elem : arr)
	{
		auto elem_codes = elem->generate_codes();
		codes.insert(std::begin(codes), std::begin(elem_codes), std::end(elem_codes));
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
	expr::expr_p const& node,
	expr::expr_p* prev)
{
	node->insert_node(std::make_shared<expr::Variable>(loc, name));
	*prev = node;
}

std::optional<ValueType> expr::Variable::type_check(ParserScope const& scope)
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
	ValueType::PrimType _type,
	std::string const& _val)
	: Expression(ExpressionType::VALUE, _loc), type(_type), val(_val) {}

void expr::Value::insert_node(
	expr::expr_p const& node,
	expr::expr_p* prev)
{
	node->insert_node(std::make_shared<expr::Value>(loc, type, val));
	*prev = node;
}

std::optional<ValueType> expr::Value::type_check(ParserScope const& scope)
{
	return type;
}

bytecodes_t expr::Value::generate_codes() const
{
	switch (type)
	{
	case ValueType::BOOL:
		assert(val == "true" || val == "false");
		return { (bytecode_t)BytecodeType::S_INT1, val == "true" };

	case ValueType::CHAR:
		assert(val.length() == 1);
		return { (bytecode_t)BytecodeType::S_INT1, (bytecode_t)val[0] };

	case ValueType::INT:
		return Value::int_to_bytecodes(std::stoull(val));

	case ValueType::FLOAT: {
		bytecodes_t codes(sizeof(float));

		float val_f = std::stof(val);
		memcpy(codes.data(), &val_f, sizeof(float));

		codes.insert(std::begin(codes), (bytecode_t)BytecodeType::FLOAT4);
		return codes;
	}
	case ValueType::STR: {
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

bytecodes_t expr::Value::int_to_bytecodes(uint64_t uint64)
{
	bytecodes_t codes;
	int count;

	if (uint64 <= std::numeric_limits<uint8_t>::max())
	{
		codes.push_back((bytecode_t)BytecodeType::S_INT1);
		count = 1;
	}
	else if (uint64 <= std::numeric_limits<uint16_t>::max())
	{
		codes.push_back((bytecode_t)BytecodeType::S_INT2);
		count = 2;
	}
	else if (uint64 <= std::numeric_limits<uint32_t>::max())
	{
		codes.push_back((bytecode_t)BytecodeType::S_INT4);
		count = 4;
	}
	else if (uint64 <= std::numeric_limits<uint64_t>::max())
	{
		codes.push_back((bytecode_t)BytecodeType::S_INT8);
		count = 8;
	}
	else
		throw debug::unhandled_case(uint64);

	while (count--)
	{
		codes.push_back(uint64 & 0xFF);
		uint64 >>= 8;
	}

	return codes;
}