#include "ast/expression.hpp"

#include "statement_scope.hpp"
#include "scope_check.hpp"
#include "bytecode.hpp"
#include "type.hpp"
#include "util.hpp"
#include "error.hpp"
#include "debug.hpp"

#include <optional>
#include <memory>
#include <vector>
#include <limits>
#include <tuple>
#include <variant>
#include <cstring>
#include <unordered_map>
#include <assert.h>

expr::Expression::Expression(
	Location const& _loc, int _precedence_)
	: loc(_loc), precedence_(_precedence_) {}

int expr::Expression::precedence() const
{
	return precedence_;
}

void expr::Expression::set_guard()
{
	precedence_ = single_precedence;
}


expr::UnaryOp::UnaryOp(
	Location const& _loc,
	std::string const& _operator)
	: Expression(_loc, Expression::unary_precedence), expr(nullptr)
{
	if (_operator == "-")	   op_type = UnaryOpType::NEGATIVE;
	else if (_operator == "!") op_type = UnaryOpType::NOT;
	else throw debug::unhandled_case(_operator);
}

expr::UnaryOp::UnaryOp(
	UnaryOp const& other)
	: Expression(other.loc, other.precedence_), op_type(other.op_type)
	, expr(other.expr), op_code(other.op_code) {}

void expr::UnaryOp::insert_node(
	expr::expr_p node,
	expr::expr_p* prev)
{
	if (!expr)
	{
		expr = node;
	}
	else if (node->precedence() < precedence())
	{
		expr->insert_node(node, &expr);
	}
	else
	{
		node->insert_node(std::make_shared<UnaryOp>(*this));
		*prev = node;
	}
}

std::optional<Type> expr::UnaryOp::type_check(
	StatementScope& scope) noexcept
{
	assert(expr);

	auto expr_type = expr->type_check(scope);

	if (!expr_type.has_value())
		return std::nullopt;

	if (expr_type->is_arr())
		night::error::get().create_minor_error(
			"expression under unary operator has type '" + night::to_str(*expr_type) +
			"', expected primitive type", loc);

	switch (op_type)
	{
	case UnaryOpType::NEGATIVE:
		switch (expr_type->prim)
		{
		case Type::BOOL:
		case Type::CHAR:
		case Type::INT:
			op_code = BytecodeType_NEGATIVE_I;
			return Type::INT;

		case Type::FLOAT:
			op_code = BytecodeType_NEGATIVE_F;
			return Type::FLOAT;
		}

		[[fallthrough]];

	case UnaryOpType::NOT:
		switch (expr_type->prim)
		{
		case Type::BOOL:
		case Type::CHAR:
		case Type::INT:
			op_code = BytecodeType_NOT_I;
			return Type::BOOL;

		case Type::FLOAT:
			op_code = BytecodeType_NOT_F;
			return Type::BOOL;
		}

		[[fallthrough]];

	default:
		return std::nullopt;
	}
}

expr::expr_p expr::UnaryOp::optimize(
	StatementScope const& scope)
{
	assert(expr);

	expr = expr->optimize(scope);

	auto numeric = std::dynamic_pointer_cast<Numeric>(expr);

	if (!numeric)
		return std::make_shared<UnaryOp>(*this);

	switch (op_type)
	{
	case UnaryOpType::NEGATIVE:
		std::visit([](auto&& arg) { arg = -arg; }, numeric->val);
		break;

	case UnaryOpType::NOT:
		std::visit([](auto&& arg) { arg = !arg; }, numeric->val);
		break;

	default:
		throw debug::unhandled_case((int)op_type);
	}

	return numeric;
}

bytecodes_t expr::UnaryOp::generate_codes() const
{
	bytecodes_t codes = expr->generate_codes();
	codes.push_back((bytecode_t)*op_code);

	return codes;
}


std::unordered_map<std::string, std::tuple<int, expr::BinaryOpType>> const expr::BinaryOp::operators{
	{ "&&", std::make_tuple(1, BinaryOpType::AND) },
	{ "||", std::make_tuple(1, BinaryOpType::OR) },
	{ "==", std::make_tuple(2, BinaryOpType::EQUALS) },
	{ "!=", std::make_tuple(2, BinaryOpType::NOT_EQUALS) },
	{ "<",  std::make_tuple(3, BinaryOpType::LESSER) },
	{ "<=", std::make_tuple(3, BinaryOpType::LESSER_EQUALS) },
	{ ">",  std::make_tuple(3, BinaryOpType::GREATER) },
	{ ">=", std::make_tuple(3, BinaryOpType::GREATER_EQUALS) },
	{ "+",  std::make_tuple(4, BinaryOpType::ADD) },
	{ "-",  std::make_tuple(4, BinaryOpType::SUB) },
	{ "*",  std::make_tuple(5, BinaryOpType::MULT) },
	{ "/",  std::make_tuple(5, BinaryOpType::DIV) },
	{ "%",  std::make_tuple(5, BinaryOpType::MOD) },
	{ "[",  std::make_tuple(6, BinaryOpType::SUBSCRIPT) }
};

expr::BinaryOp::BinaryOp(
	Location const& _loc,
	std::string const& _operator)
	: Expression(
		Location{"", 0, 0},
		Expression::binary_precedence + std::get<int>(operators.at(_operator))
	  )
	, operator_type(std::get<BinaryOpType>(operators.at(_operator)))
	, lhs(nullptr), rhs(nullptr)
	, lhs_type(std::nullopt), rhs_type(std::nullopt)
	, cast_lhs(_ByteType_INVALID_), cast_rhs(_ByteType_INVALID_) {}

expr::BinaryOp::BinaryOp(
	BinaryOp const& other)
	: Expression(other.loc, other.precedence_)
	, operator_type(other.operator_type)
	, lhs(other.lhs), rhs(other.rhs)
	, lhs_type(other.lhs_type), rhs_type(other.rhs_type)
	, cast_lhs(other.cast_lhs), cast_rhs(other.cast_rhs) {}

void expr::BinaryOp::insert_node(
	expr::expr_p node,
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
		assert(prev);

		node->insert_node(std::make_shared<BinaryOp>(*this));
		*prev = node;
	}
}

std::optional<Type> expr::BinaryOp::type_check(StatementScope& scope) noexcept
{
	assert(lhs && rhs);

	lhs_type = lhs->type_check(scope);
	rhs_type = rhs->type_check(scope);

	if (!lhs_type.has_value() || !rhs_type.has_value())
		return std::nullopt;

	switch (operator_type)
	{
	case BinaryOpType::ADD:
		if (auto string_concat = type_check_string_concat(); string_concat.has_value())
			return string_concat;

		[[fallthrough]];

	case BinaryOpType::SUB:
	case BinaryOpType::MULT:
	case BinaryOpType::DIV:
		return type_check_arithmetic();

	case BinaryOpType::MOD:
		return type_check_mod();

	case BinaryOpType::LESSER:
	case BinaryOpType::GREATER:
	case BinaryOpType::LESSER_EQUALS:
	case BinaryOpType::GREATER_EQUALS:
	case BinaryOpType::EQUALS:
	case BinaryOpType::NOT_EQUALS:
		return type_check_comparision();

	case BinaryOpType::AND:
	case BinaryOpType::OR:
		return type_check_boolean();

	case BinaryOpType::SUBSCRIPT:
		return type_check_subscript();

	default:
		throw std::runtime_error("Unhandled case in " + std::string(std::source_location::current().function_name()));
	}
}

std::optional<Type> expr::BinaryOp::type_check_string_concat()
{
	assert(lhs_type.has_value() && rhs_type.has_value());

	return lhs_type->is_str() && rhs_type->is_str()
		? std::optional(Type(Type::CHAR, 1))
		: std::nullopt;
}

std::optional<Type> expr::BinaryOp::type_check_arithmetic()
{
	assert(lhs_type.has_value() && rhs_type.has_value());

	if (lhs_type->is_prim() && rhs_type->is_prim())
	{
		if (lhs_type == Type::FLOAT)
		{
			if (rhs_type != Type::FLOAT)
				cast_rhs = BytecodeType_I2F;

			return Type::FLOAT;
		}

		if (rhs_type == Type::FLOAT)
		{
			if (lhs_type != Type::FLOAT)
				cast_lhs = BytecodeType_I2F;

			return Type::FLOAT;
		}

		return Type::INT;
	}

	if (!lhs_type->is_prim() && rhs_type->is_prim())
		night::error::get().create_minor_error(
			"The left hand expression is a " + night::to_str(lhs_type.value()) + " type.\n"
			"The " + operator_type_to_str() + " operator can only be used on two primitive types.", loc);
	else if (lhs_type->is_prim() && !rhs_type->is_prim())
		night::error::get().create_minor_error(
			"The right hand expression is a " + night::to_str(lhs_type.value()) + " type.\n"
			"The " + operator_type_to_str() + " operator can only be used on two primitive types.", loc);
	else
		night::error::get().create_minor_error(
			"The left hand expression is a " + night::to_str(lhs_type.value()) + " type and "
			"the right hand expression is a " + night::to_str(rhs_type.value()) + " type.\n"
			"The " + operator_type_to_str() + " operator can only be used on two integer types.", loc);

	return std::nullopt;
}

std::optional<Type> expr::BinaryOp::type_check_mod() const
{
	assert(lhs_type.has_value() && rhs_type.has_value());

	if (lhs_type == Type::INT && rhs_type == Type::INT)
		return Type::INT;

	if (lhs_type != Type::INT && rhs_type == Type::INT)
		night::error::get().create_minor_error(
			"The left hand expression is a " + night::to_str(lhs_type.value()) + " type.\n"
			"The modulus operator can only be used on two integer types.", loc);
	else if (lhs_type == Type::INT && rhs_type != Type::INT)
		night::error::get().create_minor_error(
			"The right hand expression is a " + night::to_str(lhs_type.value()) + " type.\n"
			"The modulus operator can only be used on two integer types.", loc);
	else
		night::error::get().create_minor_error(
			"The left hand expression is a " + night::to_str(lhs_type.value()) + " type and "
			"the right hand expression is a " + night::to_str(rhs_type.value()) + " type.\n"
			"The modulus operator can only be used on two integer types.", loc);

	return std::nullopt;
}

std::optional<Type> expr::BinaryOp::type_check_comparision()
{
	assert(lhs_type.has_value() && rhs_type.has_value());

	if (lhs_type->is_str() && rhs_type->is_str())
		return Type::BOOL;

	if (!lhs_type->is_arr() && !rhs_type->is_arr())
	{
		if (lhs_type == Type::FLOAT && rhs_type != Type::FLOAT)
			cast_rhs = BytecodeType_I2F;
		else if (rhs_type == Type::FLOAT && lhs_type != Type::FLOAT)
			cast_lhs = BytecodeType_I2F;

		return Type::BOOL;
	}

	if (lhs_type->is_arr() && !rhs_type->is_arr())
		night::error::get().create_minor_error(
			"The left hand expression is an array type.\n"
			"The " + operator_type_to_str() + " operator can only be used on two non-array types.", loc);
	else if (!lhs_type->is_arr() && rhs_type->is_arr())
		night::error::get().create_minor_error(
			"The right hand expression is a " + night::to_str(lhs_type.value()) + " type.\n"
			"The " + operator_type_to_str() + " operator can only be used on two non-array types.", loc);
	else
		night::error::get().create_minor_error(
			"The left and right hand expressions are both array types.\n"
			"The " + operator_type_to_str() + " operator can only be used on two non-array types.", loc);

	return std::nullopt;
}

std::optional<Type> expr::BinaryOp::type_check_boolean()
{
	assert(lhs_type.has_value() && rhs_type.has_value());

	if (!lhs_type->is_arr() && !rhs_type->is_arr())
	{
		if (lhs_type == Type::FLOAT)
		{
			if (rhs_type != Type::FLOAT)
				cast_rhs = BytecodeType_I2F;
		}
		else if (rhs_type != Type::FLOAT)
		{
			if (lhs_type != Type::FLOAT)
				cast_lhs = BytecodeType_I2F;
		}

		return Type::BOOL;
	}

	if (!lhs_type->is_prim() && rhs_type->is_prim())
		night::error::get().create_minor_error(
			"The left hand expression is a " + night::to_str(lhs_type.value()) + " type.\n"
			"The " + operator_type_to_str() + " operator can only be used on two primitive types.", loc);
	else if (lhs_type->is_prim() && !rhs_type->is_prim())
		night::error::get().create_minor_error(
			"The right hand expression is a " + night::to_str(lhs_type.value()) + " type.\n"
			"The " + operator_type_to_str() + " operator can only be used on two primitive types.", loc);
	else
		night::error::get().create_minor_error(
			"The left hand expression is a " + night::to_str(lhs_type.value()) + " type and "
			"the right hand expression is a " + night::to_str(rhs_type.value()) + " type.\n"
			"The " + operator_type_to_str() + " operator can only be used on two primitive types.", loc);

	return std::nullopt;
}

std::optional<Type> expr::BinaryOp::type_check_subscript() const
{
	assert(lhs_type.has_value() && rhs_type.has_value());

	if (lhs_type == Type::INT)
	{
		// Check if it is string first before checking if it is array because
		// array encompasses strings.
		if (rhs_type->is_str())
			return Type::CHAR;
		else if (rhs_type->is_arr())
			return Type(rhs_type->prim, rhs_type->dim - 1);
	}

	if (lhs_type != Type::INT)
		night::error::get().create_minor_error(
			"The left hand expression is a " + night::to_str(lhs_type.value()) + " type.\n"
			"The subscript operator's index can only be an integer type.", loc);
	else if (!rhs_type->is_str() && !rhs_type->is_arr())
		night::error::get().create_minor_error(
			"The right hand expression is a " + night::to_str(rhs_type.value()) + " type.\n"
			"The subscript operator can only be used on an array or string type.", loc);

	return std::nullopt;
}

#define BinaryOpEvaluateNumeric(op, is_result_bool)	{																							 \
	if (lhs_num->type == Type::FLOAT || rhs_num->type == Type::FLOAT)																			 \
		return std::make_shared<Numeric>(																										 \
			loc, (is_result_bool) ? Type::BOOL : Type::FLOAT,																					 \
			std::visit([](auto&& arg1, auto&& arg2) { return double(arg1 op arg2); }, lhs_num->val, rhs_num->val)								 \
		);																																		 \
																																				 \
	return std::make_shared<Numeric>(																											 \
		loc, (is_result_bool) ? Type::BOOL : Type::INT,																							 \
		int64_t(std::get<int64_t>(lhs_num->val) op std::get<int64_t>(rhs_num->val))																 \
	);																																			 \
}

expr::expr_p expr::BinaryOp::optimize(StatementScope const& scope)
{
	assert(lhs && rhs);

	lhs = lhs->optimize(scope);
	rhs = rhs->optimize(scope);

	// Optimize string concatenation.
	if (auto [lhs_str, rhs_str] = is_string_concatenation(); lhs_str && rhs_str)
	{
		std::vector<expr_p> new_str;
		night::container_concat(new_str, lhs_str->elements);
		night::container_concat(new_str, rhs_str->elements);

		return std::make_shared<Array>(loc, new_str, true);
	}

	auto lhs_num = std::dynamic_pointer_cast<Numeric>(lhs);
	auto rhs_num = std::dynamic_pointer_cast<Numeric>(rhs);

	// Both left and right hand side are not Numerics, so binary expression can
	// not be optimized.
	if (!lhs_num || !rhs_num)
		return std::make_shared<BinaryOp>(*this);

	switch (operator_type)
	{
	case BinaryOpType::ADD:  BinaryOpEvaluateNumeric(+, false);
	case BinaryOpType::SUB:  BinaryOpEvaluateNumeric(-, false);
	case BinaryOpType::MULT: BinaryOpEvaluateNumeric(*, false);
	case BinaryOpType::DIV:  BinaryOpEvaluateNumeric(/, false);

	// Separate case for modulus.
	case BinaryOpType::MOD:
		return std::make_shared<Numeric>(loc, Type::INT,
			std::visit([](auto&& arg1, auto&& arg2) {
				return (int64_t)arg1 % (int64_t)arg2;
			}, lhs_num->val, rhs_num->val)
		);

	case BinaryOpType::LESSER:		   BinaryOpEvaluateNumeric(<, true);
	case BinaryOpType::GREATER:		   BinaryOpEvaluateNumeric(>, true);
	case BinaryOpType::LESSER_EQUALS:  BinaryOpEvaluateNumeric(<=, true);
	case BinaryOpType::GREATER_EQUALS: BinaryOpEvaluateNumeric(>=, true);
	case BinaryOpType::EQUALS:		   BinaryOpEvaluateNumeric(==, true);
	case BinaryOpType::NOT_EQUALS:	   BinaryOpEvaluateNumeric(!=, true);
	case BinaryOpType::AND:			   BinaryOpEvaluateNumeric(&&, true);
	case BinaryOpType::OR:			   BinaryOpEvaluateNumeric(||, true);

	// No optimization can be done for Subscript operator.
	default:
		break;
	}

	return std::make_shared<BinaryOp>(*this);
}

bytecodes_t expr::BinaryOp::generate_codes() const
{
	assert(lhs && rhs);

	bytecodes_t bytes;

	// Generate left hand side bytes.
	night::container_concat(bytes, lhs->generate_codes());
	if (cast_lhs != _ByteType_INVALID_)
		bytes.push_back(cast_lhs);
	
	// Generate right hand side bytes.
	night::container_concat(bytes, rhs->generate_codes());
	if (cast_rhs != _ByteType_INVALID_)
		bytes.push_back(cast_rhs);

	// Generate operator byte.
	bytecode_t operator_byte = compute_operator_byte();
	bytes.push_back(operator_byte);

	return bytes;
}

std::pair<std::shared_ptr<expr::Array>, std::shared_ptr<expr::Array>> expr::BinaryOp::is_string_concatenation() const
{
	/*
	 * String concatenation requires three conditions,
	 *   1) The operator is ADD
	 *   2) Both left and right hand expressions are Arrays
	 *   3) Both left and right hand Arrays are Strings
	 */

	if (operator_type == BinaryOpType::ADD)
	{
		auto lhs_arr = std::dynamic_pointer_cast<Array>(lhs);
		auto rhs_arr = std::dynamic_pointer_cast<Array>(rhs);

		// If both left and right hand expressions are strings, return them.
		if (lhs_arr && lhs_arr->is_str() && rhs_arr && rhs_arr->is_str())
			return std::make_pair(lhs_arr, rhs_arr);
	}

	return std::make_pair(nullptr, nullptr);
}

bytecode_t expr::BinaryOp::compute_operator_byte() const
{
	/*
	 * Most operators have multiple types depending on the type of expressions
	 * they are dealing with. For example, operator ADD has three types, one for
	 * integers, floats and strings, corresponding to ADD_I, ADD_F and ADD_S
	 * respectively.
	 * 
	 * This struct stores the three types for each operator, with _INVALID_ as a
	 * placeholder for types an operator should not have, such as the string
	 * type for the SUB operator.
	 * 
	 * Array subscripts are handled in a separate case.
	 */
	struct OperatorByte {
		bytecode_t int_, float_, str_;
	};

	static std::unordered_map<BinaryOpType, OperatorByte> operator_bytes{
		{ BinaryOpType::ADD,			{ BytecodeType_ADD_I,			 BytecodeType_ADD_F,			BytecodeType_ADD_S			  } },
		{ BinaryOpType::SUB,			{ BytecodeType_SUB_I,			 BytecodeType_SUB_F,			_ByteType_INVALID_			  } },
		{ BinaryOpType::MULT,			{ BytecodeType_MULT_I,			 BytecodeType_MULT_F,			_ByteType_INVALID_			  } },
		{ BinaryOpType::DIV,			{ BytecodeType_DIV_I,			 BytecodeType_DIV_F,			_ByteType_INVALID_			  } },
		{ BinaryOpType::MOD,			{ ByteType_MOD,					 ByteType_MOD,					_ByteType_INVALID_			  } },
		{ BinaryOpType::LESSER,			{ BytecodeType_LESSER_I,		 BytecodeType_LESSER_F,		    BytecodeType_LESSER_S		  } },
		{ BinaryOpType::GREATER,		{ BytecodeType_GREATER_I,		 BytecodeType_GREATER_F,		BytecodeType_GREATER_S		  } },
		{ BinaryOpType::LESSER_EQUALS,	{ BytecodeType_LESSER_EQUALS_I,	 BytecodeType_LESSER_EQUALS_F,  BytecodeType_LESSER_EQUALS_S  } },
		{ BinaryOpType::GREATER_EQUALS,	{ BytecodeType_GREATER_EQUALS_I, BytecodeType_GREATER_EQUALS_F, BytecodeType_GREATER_EQUALS_S } },
		{ BinaryOpType::EQUALS,			{ BytecodeType_EQUALS_I,	  	 BytecodeType_EQUALS_F,		    BytecodeType_EQUALS_S		  } },
		{ BinaryOpType::NOT_EQUALS,		{ BytecodeType_NOT_EQUALS_I, 	 BytecodeType_NOT_EQUALS_F,	    BytecodeType_NOT_EQUALS_S	  } },
		{ BinaryOpType::AND,			{ BytecodeType_AND,				 BytecodeType_AND,			    _ByteType_INVALID_			  } },
		{ BinaryOpType::OR,				{ BytecodeType_OR,				 BytecodeType_OR,			    _ByteType_INVALID_			  } },
		{ BinaryOpType::SUBSCRIPT,		{ _ByteType_INVALID_,			 _ByteType_INVALID_,			BytecodeType_INDEX_S		  } }
	};

	assert(lhs_type.has_value() && rhs_type.has_value());

	if (lhs_type->is_str() || rhs_type->is_str())
		return operator_bytes[operator_type].str_;
	
	// Separate case for array subscripts.
	if (lhs_type->is_arr() || rhs_type->is_arr())
		return BytecodeType_INDEX_A;

	// Note that this is not the same as,
	//   lhs_type == Type::INT || rhs_type == Type::INT
	return lhs_type == Type::FLOAT || rhs_type == Type::FLOAT
		? operator_bytes[operator_type].float_
		: operator_bytes[operator_type].int_;
}

std::string expr::BinaryOp::operator_type_to_str() const
{
	switch (operator_type)
	{
	case BinaryOpType::ADD: return "addition";
	case BinaryOpType::SUB: return "subtraction";
	case BinaryOpType::MULT: return "multiplication";
	case BinaryOpType::DIV: return "division";
	case BinaryOpType::MOD: return "modulus";
	case BinaryOpType::LESSER: return "lesser";
	case BinaryOpType::GREATER: return "greater";
	case BinaryOpType::LESSER_EQUALS: return "lesser or equals";
	case BinaryOpType::GREATER_EQUALS: return "greater or equals";
	case BinaryOpType::EQUALS: return "equals";
	case BinaryOpType::NOT_EQUALS: return "not equals";
	case BinaryOpType::AND: return "and";
	case BinaryOpType::OR: return "or";
	case BinaryOpType::SUBSCRIPT: return "subscript";
	default: return "UNKNOWN";
	}
}


expr::Variable::Variable(
	Location const& _loc,
	std::string const& _name,
	std::optional<uint64_t> const& _id)
	: Expression(_loc, Expression::single_precedence), name(_name), id(_id) {}

void expr::Variable::insert_node(
	expr::expr_p node,
	expr::expr_p* prev)
{
	assert(prev);

	node->insert_node(std::make_shared<expr::Variable>(loc, name, id));
	*prev = node;
}

std::optional<Type> expr::Variable::type_check(StatementScope& scope) noexcept
{
	if (!check_variable_defined(scope, name, loc))
		return std::nullopt;

	id = scope.get_var(name)->id;
	return scope.get_var(name)->type;
}

expr::expr_p expr::Variable::optimize(StatementScope const& scope)
{
	return std::make_shared<Variable>(loc, name, id);
}

bytecodes_t expr::Variable::generate_codes() const
{
	assert(id.has_value());

	auto bytes = int64_to_bytes(id.value());
	bytes.push_back(ByteType_LOAD);

	return bytes;
}


expr::Array::Array(
	Location const& _loc,
	std::vector<expr_p> const& _elements,
	bool _is_str_,
	std::optional<Type> const& _type_convert,
	std::vector<std::optional<bytecode_t>> const& _type_conversion)
	: Expression(_loc, Expression::single_precedence)
	, elements(_elements), is_str_(_is_str_)
	, type_convert(_type_convert)
	, type_conversion(_type_conversion) {}

void expr::Array::insert_node(
	expr_p node,
	expr_p* prev)
{
	assert(prev);

	node->insert_node(std::make_shared<expr::Array>(loc, elements, is_str_));
	*prev = node;
}

std::optional<Type> expr::Array::type_check(StatementScope& scope) noexcept
{
	// Deduce the array type based on the elements. Create error if elements
	// are different types.

	std::optional<Type> arr_type;
	for (auto const& elem : elements)
	{
		assert(elem);

		auto elem_type = elem->type_check(scope);
		if (!elem_type.has_value())
		{
			type_conversion.push_back(std::nullopt);
			continue;
		}

		if (type_convert == Type::BOOL && elem_type == Type::FLOAT)
			type_conversion.push_back(BytecodeType_F2I);
		else if (type_convert == Type::FLOAT && elem_type != Type::FLOAT)
			type_conversion.push_back(BytecodeType_I2F);
		else if (type_convert != Type::FLOAT && elem_type == Type::FLOAT)
			type_conversion.push_back(BytecodeType_F2I);
		else
			type_conversion.push_back(std::nullopt);

		if (arr_type.has_value())
		{
			if (!arr_type->is_arr() && !elem_type->is_arr())
			{
				if (arr_type == Type::FLOAT || elem_type == Type::FLOAT)
					arr_type = Type::FLOAT;
				else if (arr_type == Type::INT || elem_type == Type::INT)
					arr_type = Type::INT;
				else if (arr_type == Type::CHAR || elem_type == Type::CHAR)
					arr_type = Type::CHAR;
				else if (arr_type == Type::BOOL || elem_type == Type::BOOL)
					arr_type = Type::BOOL;
			}
		}
		else
		{
			arr_type = elem_type;
		}
	}

	if (arr_type.has_value())
		++arr_type->dim;

	return arr_type;
}

expr::expr_p expr::Array::optimize(StatementScope const& scope)
{
	for (auto& element : elements)
		element = element->optimize(scope);

	return std::make_shared<Array>(loc, elements, is_str_, type_convert, type_conversion);
}

bytecodes_t expr::Array::generate_codes() const
{
	bytecodes_t codes;

	// Generate codes for elements.

	assert(is_str() || type_conversion.size() == elements.size());

	for (auto i = 0; i < elements.size(); ++i)
	{
		auto elem_codes = elements[i]->generate_codes();

		if (!is_str() && type_conversion[i].has_value())
			elem_codes.push_back((bytecode_t)*type_conversion[i]);

		codes.insert(std::begin(codes), std::begin(elem_codes), std::end(elem_codes));
	}

	// Generate codes for size.
	auto size_codes = int64_to_bytes<uint64_t>(elements.size());
	codes.insert(std::end(codes), std::begin(size_codes), std::end(size_codes));

	if (is_str())
		codes.push_back((bytecode_t)BytecodeType_ALLOCATE_STR);
	else
		codes.push_back((bytecode_t)BytecodeType_ALLOCATE_ARR);

	return codes;
}

bool expr::Array::is_str() const
{
	return is_str_;
}


expr::Allocate::Allocate(
	Location const& _loc,
	Type::Primitive const _type,
	std::vector<expr_p> const& _sizes)
	: Expression(_loc, Expression::single_precedence)
	, type(_type), sizes(_sizes) {}

void expr::Allocate::insert_node(
	expr_p node,
	expr_p* prev)
{
	assert(prev);

	node->insert_node(std::make_shared<expr::Allocate>(loc, type, sizes));
	*prev = node;
}

std::optional<Type> expr::Allocate::type_check(StatementScope& scope) noexcept
{
	for (auto& size : sizes)
		size->type_check(scope);

	return Type(type, (int)sizes.size());
}

expr::expr_p expr::Allocate::optimize(StatementScope const& scope)
{
	for (auto& size : sizes)
		size = size->optimize(scope);

	return std::make_shared<Allocate>(loc, type, sizes);
}

bytecodes_t expr::Allocate::generate_codes() const
{
	bytecodes_t codes;

	for (auto const& size : sizes)
	{
		auto size_codes = size->generate_codes();
		codes.insert(std::end(codes), std::begin(size_codes), std::end(size_codes));
	}

	auto dimension_bytes = int64_to_bytes<uint64_t>(sizes.size());
	codes.insert(std::end(codes), std::begin(dimension_bytes), std::end(dimension_bytes));
	
	codes.push_back((bytecode_t)BytecodeType_ALLOCATE_ARR_AND_FILL);

	return codes;
}


expr::Numeric::Numeric(
	Location const& _loc,
	Type::Primitive _type,
	std::variant<int64_t, double> const& _val)
	: Expression(_loc, Expression::single_precedence)
	, type(_type), val(_val) {}

void expr::Numeric::insert_node(
	expr_p node,
	expr_p* prev)
{
	assert(prev);

	node->insert_node(std::make_shared<expr::Numeric>(loc, type, val));
	*prev = node;
}

std::optional<Type> expr::Numeric::type_check(StatementScope& scope) noexcept
{
	return type;
}

expr::expr_p expr::Numeric::optimize(StatementScope const& scope)
{
	return std::make_shared<Numeric>(loc, type, val);
}

bytecodes_t expr::Numeric::generate_codes() const
{
	if (double const* dbl = std::get_if<double>(&val))
	{
		bytecodes_t bytes;

		// Check if the value is within the range of a float
		if (*dbl >= std::numeric_limits<float>::min() && *dbl <= std::numeric_limits<float>::max())
		{
			bytes.push_back(ByteType_FLT4);

			float f = (float)*dbl;
			uint8_t arr[sizeof(float)];
			std::memcpy(arr, &f, sizeof(float));

			for (int i = 0; i < sizeof(float); ++i)
				bytes.push_back(arr[i]);
		}
		else
		{
			bytes.push_back(ByteType_FLT8);

			uint8_t arr[sizeof(double)];
			std::memcpy(arr, dbl, sizeof(double));

			for (int i = 0; i < sizeof(double); ++i)
				bytes.push_back(arr[i]);
		}

		return bytes;
	}
	else
	{
		return int64_to_bytes(std::get<int64_t>(val));
	}
}

bool expr::Numeric::is_true() const
{
	return std::visit([](auto&& arg) { return (bool)arg; }, val);
}