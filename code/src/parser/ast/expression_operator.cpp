#include "parser/ast/expression_operator.hpp"
#include "parser/ast/expression.hpp"
#include "common/util.hpp"
#include "common/debug.hpp"

#include <assert.h>

std::unordered_map<std::string, expr::UnaryOpType> const expr::UnaryOp::operators{
	{ "-", UnaryOpType::NEGATIVE },
	{ "!", UnaryOpType::NOT }
};

expr::UnaryOp::UnaryOp(
	Location const& _loc,
	std::string const& _operator)
	: Expression(_loc, Expression::unary_precedence)
	, operator_type(operators.at(_operator))
	, expr(nullptr)
	, expr_type(std::nullopt) {}

expr::UnaryOp::UnaryOp(
	UnaryOp const& other)
	: Expression(other.loc, other.precedence_)
	, operator_type(other.operator_type)
	, expr(other.expr)
	, expr_type(other.expr_type) {}

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

	expr_type = expr->type_check(scope);
	if (!expr_type.has_value())
		return std::nullopt;

	if (!expr_type->is_prim())
	{
		night::error::get().create_minor_error(
			"The operator " + operator_type_to_str() + " can only be used on a primitive type.\n"
			"The expression is type " + night::to_str(expr_type.value()) + ".", loc);
		return std::nullopt;
	}

	switch (operator_type)
	{
	case UnaryOpType::NEGATIVE: {
		if (!expr_type->is_int() && expr_type != Primitive::FLOAT)
		{
			night::error::get().create_minor_error(
				"The negative operator can only be used on integral or floating point expressions.\n"
				"The expression is type " + night::to_str(expr_type.value()) + ".", loc);
			return std::nullopt;
		}

		return expr_type;
	}

	case UnaryOpType::NOT:
		if (expr_type != Primitive::BOOL)
		{
			night::error::get().create_minor_error(
				"The not operator can only be used on boolean expressions.\n"
				"The expression is type " + night::to_str(expr_type.value()) + ".", loc);
			return std::nullopt;
		}

		return Type(Primitive::BOOL, 0);

	default:
		assert(false);
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

	switch (operator_type)
	{
	case UnaryOpType::NEGATIVE:
		std::visit([](auto&& arg) { arg = -arg; }, numeric->val);
		break;

	case UnaryOpType::NOT:
		std::visit([](auto&& arg) { arg = !arg; }, numeric->val);
		break;

	default:
		throw debug::unhandled_case((unsigned)operator_type);
	}

	return numeric;
}

bytecodes_t expr::UnaryOp::generate_codes() const
{
	assert(expr);

	bytecodes_t codes;

	// Generate expression bytes.
	night::container_concat(codes, expr->generate_codes());

	// Generate operator bytes.
	codes.push_back(generate_operator_byte());

	return codes;
}

bytecode_t expr::UnaryOp::generate_operator_byte() const
{
	/*
	 * Read the comments inside the expr::BinaryOp::compute_operator_byte().
	 */
	struct OperatorByte {
		bytecode_t int_, float_;
	};

	static std::unordered_map<UnaryOpType, OperatorByte> const operator_bytes{
		{ UnaryOpType::NEGATIVE, { BytecodeType_NEGATIVE_I,	BytecodeType_NEGATIVE_F	} },
		{ UnaryOpType::NOT,		 { BytecodeType_NOT_I,		BytecodeType_NOT_F		} }
	};

	assert(expr_type.has_value());

	return expr_type == Primitive::FLOAT
		? operator_bytes.at(operator_type).float_
		: operator_bytes.at(operator_type).int_;
}

std::string expr::UnaryOp::operator_type_to_str() const
{
	switch (operator_type)
	{
	case UnaryOpType::NEGATIVE: return "negative";
	case UnaryOpType::NOT: return "not";
	default: throw debug::unhandled_case((unsigned)operator_type);
	}
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
		_loc,
		Expression::binary_precedence + std::get<int>(operators.at(_operator))
	)
	, operator_type(std::get<BinaryOpType>(operators.at(_operator)))
	, lhs(nullptr), rhs(nullptr)
	, lhs_type(std::nullopt), rhs_type(std::nullopt) {}

expr::BinaryOp::BinaryOp(
	BinaryOp const& other)
	: Expression(other.loc, other.precedence_)
	, operator_type(other.operator_type)
	, lhs(other.lhs), rhs(other.rhs)
	, lhs_type(other.lhs_type), rhs_type(other.rhs_type) {}

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
		assert(false);
		return std::nullopt;
	}
}

std::optional<Type> expr::BinaryOp::type_check_string_concat()
{
	assert(lhs_type.has_value() && rhs_type.has_value());

	return lhs_type->is_str() && rhs_type->is_str()
		? std::optional(Type(Primitive::CHAR, 1))
		: std::nullopt;
}

std::optional<Type> expr::BinaryOp::type_check_arithmetic()
{
	assert(lhs_type.has_value() && rhs_type.has_value());

	if (lhs_type != rhs_type || lhs_type->is_arr() || rhs_type->is_arr())
	{
		night::error::get().create_minor_error(
			"The left hand expression is a " + night::to_str(lhs_type.value()) + " type and "
			"the right hand expression is a " + night::to_str(rhs_type.value()) + " type.\n"
			"The " + operator_type_to_str() + " operator can only be used on the same primitive types or two strings.", loc);

		return std::nullopt;
	}

	return lhs_type;
}

std::optional<Type> expr::BinaryOp::type_check_mod() const
{
	assert(lhs_type.has_value() && rhs_type.has_value());

	if (!lhs_type->is_int() || !rhs_type->is_int() || lhs_type != rhs_type) {
		night::error::get().create_minor_error(
			"The left hand expression is a " + night::to_str(lhs_type.value()) + " type and "
			"the right hand expression is a " + night::to_str(rhs_type.value()) + " type.\n"
			"The modulus operator can only be used on two same integer types.", loc);
		return std::nullopt;
	}

	return rhs_type;
}

std::optional<Type> expr::BinaryOp::type_check_comparision()
{
	assert(lhs_type.has_value() && rhs_type.has_value());

	if (lhs_type->is_str() && rhs_type->is_str())
		return Primitive::BOOL;

	if (lhs_type != rhs_type || lhs_type->is_arr() || rhs_type->is_arr())
	{
		night::error::get().create_minor_error(
			"The left hand expression is a " + night::to_str(lhs_type.value()) + " type and "
			"the right hand expression is a " + night::to_str(rhs_type.value()) + " type.\n"
			"The " + operator_type_to_str() + " operator can only be used on the same primitive or string types.", loc);

		return std::nullopt;
	}

	return Primitive::BOOL;
}

std::optional<Type> expr::BinaryOp::type_check_boolean()
{
	assert(lhs_type.has_value() && rhs_type.has_value());

	if (lhs_type != rhs_type || lhs_type->is_arr() || rhs_type->is_arr())
	{
		night::error::get().create_minor_error(
			"The left hand expression is a " + night::to_str(lhs_type.value()) + " type and "
			"the right hand expression is a " + night::to_str(rhs_type.value()) + " type.\n"
			"The " + operator_type_to_str() + " operator can only be used on the same primitive types.", loc);

		return std::nullopt;
	}

	return Primitive::BOOL;
}

std::optional<Type> expr::BinaryOp::type_check_subscript() const
{
	assert(lhs_type.has_value() && rhs_type.has_value());

	bool warnings = false;

	if (!lhs_type->is_int())
	{
		night::error::get().create_minor_error(
			"The left hand expression is a " + night::to_str(lhs_type.value()) + " type.\n"
			"The subscript operator's index can only be an integer type.", loc);
		warnings = true;
	}
	if (!rhs_type->is_arr())
	{
		night::error::get().create_minor_error(
			"The right hand expression is a " + night::to_str(rhs_type.value()) + " type.\n"
			"The subscript operator can only be used on an array or string type.", loc);
		warnings = true;
	}

	if (warnings)
		return std::nullopt;
	
	return Type(rhs_type->get_prim(), rhs_type->get_dim() - 1);
}

#define BinaryOpEvaluateNumeric(op, is_result_bool)	{																\
	if (lhs_num->type == Primitive::FLOAT)																				\
		return std::make_shared<Numeric>(																			\
			loc, (is_result_bool) ? Primitive::BOOL : Primitive::FLOAT,														\
			std::visit([](auto&& arg1, auto&& arg2) { return double(arg1 op arg2); }, lhs_num->val, rhs_num->val)	\
		);																											\
																													\
	return std::make_shared<Numeric>(																				\
		loc, (is_result_bool) ? Primitive::BOOL : lhs_num->type,															\
		int64_t(std::get<int64_t>(lhs_num->val) op std::get<int64_t>(rhs_num->val))									\
	);																												\
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

	// Optimize subscript operator.
	if (auto [lhs_num, rhs_arr] = is_array_subscript(); lhs_num && rhs_arr)
	{
		int64_t index = std::visit(
			[](auto&& v) { return (int64_t)v; },
			lhs_num->val
		);

		if (index < 0 || index > rhs_arr->elements.size())
			throw night::error::get().create_fatal_error(
				"Index '" + std::to_string(index) + "' is out of range.\n"
				"Array has size '" + std::to_string(rhs_arr->elements.size()) + "'.", { loc.file, loc.line, loc.col - 1 });

		return rhs_arr->elements[index];
	}

	auto lhs_num = std::dynamic_pointer_cast<Numeric>(lhs);
	auto rhs_num = std::dynamic_pointer_cast<Numeric>(rhs);

	// Both left and right hand side are not Numerics, so binary expression can
	// not be optimized.
	if (!lhs_num || !rhs_num)
		return std::make_shared<BinaryOp>(*this);

	// Assertion should be true from type_check()
	assert(lhs_num->type == rhs_num->type);

	switch (operator_type)
	{
	case BinaryOpType::ADD:  BinaryOpEvaluateNumeric(+, false);
	case BinaryOpType::SUB:  BinaryOpEvaluateNumeric(-, false);
	case BinaryOpType::MULT: BinaryOpEvaluateNumeric(*, false);
	case BinaryOpType::DIV:  BinaryOpEvaluateNumeric(/ , false);

	// Separate case for modulus.
	case BinaryOpType::MOD:
		return std::make_shared<Numeric>(loc, lhs_num->type,
			std::visit([](auto&& arg1, auto&& arg2) {
				return (int64_t)arg1 % (int64_t)arg2;
			}, lhs_num->val, rhs_num->val)
		);

	case BinaryOpType::LESSER:		   BinaryOpEvaluateNumeric(< , true);
	case BinaryOpType::GREATER:		   BinaryOpEvaluateNumeric(> , true);
	case BinaryOpType::LESSER_EQUALS:  BinaryOpEvaluateNumeric(<= , true);
	case BinaryOpType::GREATER_EQUALS: BinaryOpEvaluateNumeric(>= , true);
	case BinaryOpType::EQUALS:		   BinaryOpEvaluateNumeric(== , true);
	case BinaryOpType::NOT_EQUALS:	   BinaryOpEvaluateNumeric(!= , true);
	case BinaryOpType::AND:			   BinaryOpEvaluateNumeric(&&, true);
	case BinaryOpType::OR:			   BinaryOpEvaluateNumeric(|| , true);
		
	// Optimization for subscript is handled separately above.
	default:
		break;
	}

	return std::make_shared<BinaryOp>(*this);
}

bytecodes_t expr::BinaryOp::generate_codes() const
{
	assert(lhs && rhs);

	bytecodes_t bytes;

	night::container_concat(bytes, lhs->generate_codes());
	night::container_concat(bytes, rhs->generate_codes());

	bytes.push_back(generate_operator_byte());

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

std::pair<std::shared_ptr<expr::Numeric>, std::shared_ptr<expr::Array>> expr::BinaryOp::is_array_subscript() const
{
	/*
	 * String subscript requires three conditions,
	 *   1) The operator is SUBSCRIPT
	 *   2) Left hand side expression is a Numeric
	 *   3) Right hand side expression is an Array
	 */

	if (operator_type == BinaryOpType::SUBSCRIPT)
	{
		auto lhs_num = std::dynamic_pointer_cast<Numeric>(lhs);
		auto rhs_arr = std::dynamic_pointer_cast<Array>(rhs);

		if (lhs_num && rhs_arr)
			return std::make_pair(lhs_num, rhs_arr);
	}

	return std::make_pair(nullptr, nullptr);
}

bytecode_t expr::BinaryOp::generate_operator_byte() const
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

	static std::unordered_map<BinaryOpType, OperatorByte> const operator_bytes{
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
		return operator_bytes.at(operator_type).str_;

	// Separate case for array subscripts.
	if (lhs_type->is_arr() || rhs_type->is_arr())
		return BytecodeType_INDEX_A;

	// Note that this is not the same as,
	//   lhs_type == Type::INT || rhs_type == Type::INT
	return lhs_type == Primitive::FLOAT || rhs_type == Primitive::FLOAT
		? operator_bytes.at(operator_type).float_
		: operator_bytes.at(operator_type).int_;
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
	default: throw debug::unhandled_case((unsigned)operator_type);
	}
}

expr::BinaryOpType const& expr::BinaryOp::get_type() const
{
	return operator_type;
}

expr::expr_p const& expr::BinaryOp::get_lhs() const
{
	return lhs;
}

expr::expr_p const& expr::BinaryOp::get_rhs() const
{
	return rhs;
}
