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
#include <set>
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


expr::Array::Array(Location const& _loc, std::vector<expr_p> const& _elements, bool _is_str_)
	: Expression(_loc, Expression::single_precedence)
	, elements(_elements)
	, is_str_(_is_str_) {}

void expr::Array::insert_node(expr_p node, expr_p* prev)
{
	assert(prev);

	node->insert_node(std::make_shared<Array>(*this));
	*prev = node;
}

static bool cmp(Type const& t1, Type const& t2)
{
	if (t1.prim != t2.prim)
		return t1.prim < t2.prim;

	return t1.dim < t2.dim;
};

static std::string array_types_to_str(std::set<Type, decltype(&cmp)> const& types)
{
	std::string types_s;

	for (auto it = std::cbegin(types); it != std::cend(types); ++it)
	{
		types_s += night::to_str(*it);
		if (std::next(it) != std::end(types))
			types_s += ", ";
	}

	return types_s;
}

std::optional<Type> expr::Array::type_check(StatementScope& scope) noexcept
{
	std::set<Type, decltype(&cmp)> types;

	for (auto const& element : elements)
	{
		assert(element);

		auto type = element->type_check(scope);

		if (type.has_value())
			types.insert(type.value());
	}

	if (types.empty())
	{
		return std::nullopt;
	}
	else if (types.size() != 1)
	{
		night::error::get().create_warning(
			"Array elements have different types " + array_types_to_str(types), loc);

		return std::nullopt;
	}
	else
	{
		Type array_type = *std::begin(types);
		++array_type.dim;

		return array_type;
	}
}

expr::expr_p expr::Array::optimize(StatementScope const& scope)
{
	for (auto& element : elements)
		element = element->optimize(scope);

	return std::make_shared<Array>(*this);
}

bytecodes_t expr::Array::generate_codes() const
{
	bytecodes_t bytes;

	for (auto i = 0; i < elements.size(); ++i)
	{
		auto elem_codes = elements[i]->generate_codes();
		night::container_concat(bytes, elem_codes);
	}

	auto size_bytes = int_to_bytes<uint64_t>(elements.size());
	night::container_concat(bytes, size_bytes);

	bytes.push_back(is_str() ? BytecodeType_ALLOCATE_STR : BytecodeType_ALLOCATE_ARR);

	return bytes;
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
