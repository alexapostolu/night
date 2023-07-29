#include "ast/expression.hpp"
#include "bytecode.hpp"
#include "parser_scope.hpp"
#include "parser.hpp"
#include "error.hpp"

#include <limits>
#include <optional>
#include <memory>

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
		night::throw_unhandled_case(_type);
}

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
		*prev = node;
		node->insert_node(std::shared_ptr<UnaryOp>(this));
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
		night::throw_unhandled_case((int)type);
	}

	return codes;
}

std::optional<val::value_t> expr::UnaryOp::type_check(ParserScope const& scope) const
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
		night::throw_unhandled_case((int)type);
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
		night::throw_unhandled_case(_type);
}

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
		*prev = node;
		node->insert_node(std::shared_ptr<BinaryOp>(this));
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
	case BinaryOpType::SUB:
		codes.push_back((bytecode_t)BytecodeType::SUB);
	case BinaryOpType::MULT:
		codes.push_back((bytecode_t)BytecodeType::MULT);
	case BinaryOpType::DIV:
		codes.push_back((bytecode_t)BytecodeType::DIV);
	default:
		night::throw_unhandled_case((int)type);
	}

	return codes;
}

std::optional<val::value_t> expr::BinaryOp::type_check(ParserScope const& scope) const
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

		if (*lhs_type == (val::value_t)val::ValueType::S_INT || *rhs_type == (val::value_t)val::ValueType::S_INT)
			return (val::value_t)val::ValueType::S_INT;
		if (*lhs_type == (val::value_t)val::ValueType::U_INT || *rhs_type == (val::value_t)val::ValueType::U_INT)
			return (val::value_t)val::ValueType::U_INT;
		if (*lhs_type == (val::value_t)val::ValueType::CHAR || *rhs_type == (val::value_t)val::ValueType::CHAR)
			return (val::value_t)val::ValueType::CHAR;
		return (val::value_t)val::ValueType::BOOL;
	}
}

int expr::BinaryOp::precedence() const
{
	if (guard)
		return bin_op_prec;

	switch (type)
	{
	case BinaryOpType::ADD:
	case BinaryOpType::SUB:
		return bin_op_prec + 101;
	case BinaryOpType::MULT:
	case BinaryOpType::DIV:
		return bin_op_prec + 102;
	case BinaryOpType::DOT:
		return bin_op_prec + 103;
	default:
		night::throw_unhandled_case((int)type);
	}
}

expr::Value::Value(
	Location const& _loc,
	val::Value const& _val)
	: Expression(ExpressionType::VALUE, _loc), val(_val) {}

void expr::Value::insert_node(
	std::shared_ptr<Expression> const& node,
	std::shared_ptr<Expression>* prev)
{
	(*prev)->insert_node(std::make_shared<expr::Value>(*this));
}

bytecodes_t expr::Value::generate_codes() const
{
	switch ((val::ValueType)val.type)
	{
	case val::ValueType::BOOL:
		return { (bytecode_t)BytecodeType::BOOL, (bool)std::get<uint64_t>(val.data) };
	case val::ValueType::CHAR:
		return { (bytecode_t)BytecodeType::CHAR1, (bytecode_t)std::get<uint64_t>(val.data) };
	case val::ValueType::S_INT:
	{
		bytecodes_t codes;
		int64_t int64 = std::get<int64_t>(val.data);

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
	case val::ValueType::U_INT:
	{
		bytecodes_t codes;
		uint64_t uint64 = std::get<uint64_t>(val.data);

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

		return codes;
	}
	default:
		night::throw_unhandled_case((int)(val.type));
	}
}

std::optional<val::value_t> expr::Value::type_check(ParserScope const& scope) const
{
	return val.type;
}

int expr::Value::precedence() const
{
	return single_prec;
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
	*this = *dynamic_cast<expr::Variable*>(node.get());
}

bytecodes_t expr::Variable::generate_codes() const
{
	return { (bytecode_t)BytecodeType::LOAD, id };
}

std::optional<val::value_t> expr::Variable::type_check(ParserScope const& scope) const
{
	return scope.vars.at(name).type;
}

int expr::Variable::precedence() const
{
	return single_prec;
}