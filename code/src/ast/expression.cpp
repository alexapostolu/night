#include "ast/expression.hpp"

#include "statement_scope.hpp"
#include "scope_check.hpp"
#include "bytecode.hpp"
#include "type.hpp"
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


expr::BinaryOp::BinaryOp(
	Location const& _loc,
	std::string const& _operator)
	: Expression(Location{"", 0, 0})
{
	static std::unordered_map<std::string, std::tuple<int, BinaryOpType>> binary_operator_map{
		{ "&&", std::make_tuple(1, BinaryOpType::AND) },
		{ "||", std::make_tuple(1, BinaryOpType::OR) },
		{ "==", std::make_tuple(2, BinaryOpType::EQUALS) },
		{ "!=", std::make_tuple(2, BinaryOpType::NOT_EQUALS) },
		{ "<", std::make_tuple(3, BinaryOpType::LESSER) },
		{ "<=", std::make_tuple(3, BinaryOpType::LESSER_EQUALS) },
		{ ">", std::make_tuple(3, BinaryOpType::GREATER) },
		{ ">=", std::make_tuple(3, BinaryOpType::GREATER_EQUALS) },
		{ "+", std::make_tuple(4, BinaryOpType::ADD) },
		{ "-", std::make_tuple(4, BinaryOpType::SUB) },
		{ "*", std::make_tuple(5, BinaryOpType::MULT) },
		{ "/", std::make_tuple(5, BinaryOpType::DIV) },
		{ "%", std::make_tuple(5, BinaryOpType::MOD) },
		{ "[", std::make_tuple(6, BinaryOpType::SUBSCRIPT) }
	};

	assert(binary_operator_map.contains(_operator));

	precedence_ = Expression::binary_precedence + std::get<int>(binary_operator_map[_operator]);
	op_type	= std::get<BinaryOpType>(binary_operator_map[_operator]);
}

expr::BinaryOp::BinaryOp(
	BinaryOp const& other)
	: Expression(other.loc, other.precedence_)
	, op_type(other.op_type)
	, lhs(other.lhs), rhs(other.rhs)
	, cast_lhs(other.cast_lhs), cast_rhs(other.cast_rhs)
	, op_code(other.op_code) {}

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
	// Operations where strings are supported are handled separately in the
	// switch statement.
	std::unordered_map<BinaryOpType, std::tuple<bytecode_t, bytecode_t, std::optional<bytecode_t>>> m{
		{ BinaryOpType::ADD,			std::make_tuple(BytecodeType_ADD_I,			BytecodeType_ADD_F,			BytecodeType_ADD_S) },
		{ BinaryOpType::SUB,			std::make_tuple(BytecodeType_SUB_I,			BytecodeType_SUB_F,			std::nullopt) },
		{ BinaryOpType::MULT,			std::make_tuple(BytecodeType_MULT_I,			BytecodeType_MULT_F,			std::nullopt) },
		{ BinaryOpType::DIV,			std::make_tuple(BytecodeType_DIV_I,			BytecodeType_DIV_F,			std::nullopt) },
		{ BinaryOpType::LESSER,			std::make_tuple(BytecodeType_LESSER_I,			BytecodeType_LESSER_F,			BytecodeType_LESSER_S) },
		{ BinaryOpType::GREATER,		std::make_tuple(BytecodeType_GREATER_I,		BytecodeType_GREATER_F,		BytecodeType_GREATER_S) },
		{ BinaryOpType::LESSER_EQUALS,	std::make_tuple(BytecodeType_LESSER_EQUALS_I,	BytecodeType_LESSER_EQUALS_F,	BytecodeType_LESSER_EQUALS_S) },
		{ BinaryOpType::GREATER_EQUALS,	std::make_tuple(BytecodeType_GREATER_EQUALS_I, BytecodeType_GREATER_EQUALS_F,	BytecodeType_GREATER_EQUALS_S) },
		{ BinaryOpType::EQUALS,			std::make_tuple(BytecodeType_EQUALS_I,			BytecodeType_EQUALS_F,			BytecodeType_EQUALS_S) },
		{ BinaryOpType::NOT_EQUALS,		std::make_tuple(BytecodeType_NOT_EQUALS_I,		BytecodeType_NOT_EQUALS_F,		BytecodeType_NOT_EQUALS_S) }
	};

	auto op_code_int = std::get<0>(m[op_type]);
	auto op_code_float = std::get<1>(m[op_type]);
	auto op_code_str = std::get<2>(m[op_type]);

	assert(lhs);
	assert(rhs);

	auto lhs_type = lhs->type_check(scope);
	auto rhs_type = rhs->type_check(scope);

	if (!lhs_type.has_value() || !rhs_type.has_value())
		return std::nullopt;

	switch (op_type)
	{
	case BinaryOpType::ADD:
		if (lhs_type->is_str())
		{
			if (!rhs_type->is_str())
				break;

			op_code = BytecodeType_ADD_S;
			return Type(Type::CHAR, 1);
		}

		[[fallthrough]];

	case BinaryOpType::SUB:
	case BinaryOpType::MULT:
	case BinaryOpType::DIV:
		if (!lhs_type->is_arr() && !rhs_type->is_arr())
		{
			if (lhs_type == Type::FLOAT)
			{
				if (rhs_type != Type::FLOAT)
					cast_rhs = BytecodeType_I2F;

				op_code = op_code_float;
				return Type::FLOAT;
			}

			if (rhs_type == Type::FLOAT)
			{
				if (lhs_type != Type::FLOAT)
					cast_lhs = BytecodeType_I2F;

				op_code = op_code_float;
				return Type::FLOAT;
			}

			op_code = op_code_int;
			return Type::INT;
		}

		break;

	case BinaryOpType::MOD:
		if (lhs_type == Type::INT && rhs_type == Type::INT)
		{
			op_code = BytecodeType_MOD_I;
			return Type::INT;
		}
		break;

	case BinaryOpType::LESSER:
	case BinaryOpType::GREATER:
	case BinaryOpType::LESSER_EQUALS:
	case BinaryOpType::GREATER_EQUALS:
	case BinaryOpType::EQUALS:
	case BinaryOpType::NOT_EQUALS:
		if (lhs_type->is_str() && rhs_type->is_str())
		{
			assert(op_code_str.has_value());
			op_code = *op_code_str;
			return Type::BOOL;
		}

		if (!lhs_type->is_arr() && !rhs_type->is_arr())
		{
			if (lhs_type == Type::FLOAT)
			{
				if (rhs_type != Type::FLOAT)
					cast_rhs = BytecodeType_I2F;

				op_code = op_code_float;
			}
			else if (rhs_type == Type::FLOAT)
			{
				if (lhs_type != Type::FLOAT)
					cast_lhs = BytecodeType_I2F;

				op_code = op_code_float;
			}
			else
			{
				op_code = op_code_int;
			}

			return Type::BOOL;
		}

		break;

	case BinaryOpType::AND:
	case BinaryOpType::OR:
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

			op_code = op_type == BinaryOpType::AND
						? BytecodeType_AND
						: BytecodeType_OR;
			return Type::BOOL;
		}

		break;

	case BinaryOpType::SUBSCRIPT:
		if (lhs_type == Type::INT)
		{
			if (rhs_type->is_str())
			{
				op_code = BytecodeType_INDEX_S;
				return Type::CHAR;
			}
			else if (rhs_type->is_arr())
			{
				op_code = BytecodeType_INDEX_A;
				return Type(rhs_type->prim, rhs_type->dim - 1);
			}
		}

		// Special error case for subscript.
		night::error::get().create_minor_error("subscript operator used on type '" + night::to_str(*rhs_type) + "', it can only be used on arrays", loc);
		return std::nullopt;

	default:
		return std::nullopt;
		//throw debug::unhandled_case((int)op_type);
	}

	night::error::get().create_minor_error("type mismatch between '" + night::to_str(*lhs_type) + "' and '" + night::to_str(*rhs_type) + "'", loc);
	return std::nullopt;
}

expr::expr_p expr::BinaryOp::optimize(StatementScope const& scope)
{
	assert(lhs && rhs);

	lhs = lhs->optimize(scope);
	rhs = rhs->optimize(scope);

	// String concatenation.
	if (op_type == BinaryOpType::ADD)
	{
		auto lhs_arr = std::dynamic_pointer_cast<Array>(lhs);
		auto rhs_arr = std::dynamic_pointer_cast<Array>(rhs);

		if (lhs_arr && lhs_arr->is_str() && rhs_arr && rhs_arr->is_str())
		{
			std::vector<expr_p> new_str;
			new_str.insert(std::end(new_str), std::begin(lhs_arr->elements), std::end(lhs_arr->elements));
			new_str.insert(std::end(new_str), std::begin(rhs_arr->elements), std::end(rhs_arr->elements));

			return std::make_shared<Array>(loc, new_str, true);
		}
	}

	auto lhs_lit = std::dynamic_pointer_cast<Numeric>(lhs);
	auto rhs_lit = std::dynamic_pointer_cast<Numeric>(rhs);

	if (!lhs_lit || !rhs_lit)
		return std::make_shared<BinaryOp>(*this);

	auto op = [&loc = this->loc, &lhs_lit, &rhs_lit](bool is_bool, auto eval) -> std::shared_ptr<Numeric> {
		if (std::holds_alternative<double>(lhs_lit->val) || std::holds_alternative<double>(rhs_lit->val))
			return std::make_shared<Numeric>(
				loc, is_bool ? Type::BOOL : Type::FLOAT,
				eval(std::visit([](auto&& arg) { return (double)arg; }, lhs_lit->val), std::visit([](auto&& arg) { return (double)arg; }, rhs_lit->val))
			);
	
		return std::make_shared<Numeric>(
			loc, is_bool ? Type::BOOL : Type::INT,
			eval(std::visit([](auto&& arg) { return (int64_t)arg; }, lhs_lit->val), std::visit([](auto&& arg) { return (int64_t)arg; }, rhs_lit->val))
		);
	};

	// Modulo and subscript operator are excluded because an exception needs to
	// be thrown instead of an error.
	switch (op_type)
	{
	case BinaryOpType::ADD:  return op(false, [](auto p1, auto p2) { return p1 + p2; });
	case BinaryOpType::SUB:  return op(false, [](auto p1, auto p2) { return p1 - p2; });
	case BinaryOpType::MULT: return op(false, [](auto p1, auto p2) { return p1 * p2; });
	case BinaryOpType::DIV:  return op(false, [](auto p1, auto p2) { return p1 / p2; });
	case BinaryOpType::MOD: {
		return std::make_shared<Numeric>(
			loc, Type::INT,
			std::visit([](auto&& arg) { return (int64_t)arg; }, lhs_lit->val) % std::visit([](auto&& arg) { return (int64_t)arg; }, rhs_lit->val)
		);
	}

	case BinaryOpType::LESSER:		   return op(true, [](auto p1, auto p2) { return (int64_t)(p1 < p2); });
	case BinaryOpType::GREATER:		   return op(true, [](auto p1, auto p2) { return (int64_t)(p1 < p2); });
	case BinaryOpType::LESSER_EQUALS:  return op(true, [](auto p1, auto p2) { return (int64_t)(p1 <= p2); });
	case BinaryOpType::GREATER_EQUALS: return op(true, [](auto p1, auto p2) { return (int64_t)(p1 >= p2); });
	case BinaryOpType::EQUALS:		   return op(true, [](auto p1, auto p2) { return (int64_t)(p1 == p2); });
	case BinaryOpType::NOT_EQUALS:	   return op(true, [](auto p1, auto p2) { return (int64_t)(p1 != p2); });
	case BinaryOpType::AND:			   return op(true, [](auto p1, auto p2) { return (int64_t)(p1 && p2); });
	case BinaryOpType::OR:			   return op(true, [](auto p1, auto p2) { return (int64_t)(p1 || p2); });

	default: break;//debug::unhandled_case((int)op_type);
	}

	return std::make_shared<BinaryOp>(*this);
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

	codes.push_back((bytecode_t)op_code);

	return codes;
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
	: Expression(_loc, Expression::single_precedence), elements(_elements), is_str_(_is_str_), type_convert(_type_convert), type_conversion(_type_conversion) {}

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

	assert(type_conversion.size() == elements.size());

	for (auto i = 0; i < elements.size(); ++i)
	{
		auto elem_codes = elements[i]->generate_codes();

		if (type_conversion[i].has_value())
			elem_codes.push_back((bytecode_t)*type_conversion[i]);

		codes.insert(std::begin(codes), std::begin(elem_codes), std::end(elem_codes));
	}

	// Generate codes for size.
	auto size_codes = int64_to_bytes(elements.size());
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

	auto dimension_bytes = int64_to_bytes(sizes.size());
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