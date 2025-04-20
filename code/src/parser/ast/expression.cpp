#include "parser/ast/expression.hpp"

#include "parser/statement_scope.hpp"
#include "common/bytecode.hpp"
#include "common/type.hpp"
#include "common/util.hpp"
#include "common/error.hpp"
#include "common/debug.hpp"

#include <optional>
#include <memory>
#include <vector>
#include <tuple>
#include <variant>
#include <string>
#include <unordered_map>
#include <assert.h>

expr::Expression::Expression(
	Location const& _loc,
	int _precedence_)
	: loc(_loc)
	, precedence_(_precedence_) {}

int expr::Expression::precedence() const
{
	return precedence_;
}

void expr::Expression::set_guard()
{
	precedence_ = single_precedence;
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
	auto variable = scope.get_variable(name, loc);
	if (!variable)
		return std::nullopt;

	id = variable->id;

	return variable->type;
}

expr::expr_p expr::Variable::optimize(StatementScope const& scope)
{
	return std::make_shared<Variable>(loc, name, id);
}

bytecodes_t expr::Variable::generate_codes() const
{
	assert(id.has_value());

	auto bytes = int_to_bytes(id.value());
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
	auto size_codes = int_to_bytes<uint64_t>(elements.size());
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
	, type(_type)
	, sizes(_sizes) {}

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

	auto dimension_bytes = int_to_bytes<uint64_t>(sizes.size());
	codes.insert(std::end(codes), std::begin(dimension_bytes), std::end(dimension_bytes));
	
	codes.push_back(BytecodeType_ALLOCATE_ARR_AND_FILL);

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
	assert(node);
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
	if (double const* d = std::get_if<double>(&val))
	{
		bytecodes_t bytes;

		// Check if the double can be converted to a float without loss of
		// decimal precision.
		if (*d == static_cast<double>(static_cast<float>(*d)))
		{
			bytes.push_back(ByteType_FLT4);

			float f = static_cast<float>(*d);
			uint8_t* p = reinterpret_cast<uint8_t*>(&f);

			bytes.insert(std::end(bytes), p, p + sizeof(float));
		}
		else
		{
			bytes.push_back(ByteType_FLT8);

			uint8_t const* p = reinterpret_cast<uint8_t const*>(d);
			bytes.insert(std::end(bytes), p, p + sizeof(double));
		}

		return bytes;
	}
	else
	{
		return int_to_bytes(std::get<int64_t>(val));
	}
}

bool expr::Numeric::is_true() const
{
	return std::visit([](auto&& arg) { return (bool)arg; }, val);
}

std::variant<int64_t, double> const& expr::Numeric::get_val() const
{
	return val;
}
