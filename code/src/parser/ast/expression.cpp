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

	bytecodes_t bytes = int_to_bytes(id.value());
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

/*
 * Compare by primitive then by dimension.
 * 
 * Used by std::set's compare in array element type checking.
 */
static bool array_types_cmp(Type const& t1, Type const& t2)
{
	if (t1.get_prim() != t2.get_prim())
		return t1.get_prim() < t2.get_prim();

	return t1.get_dim() < t2.get_dim();
};

/*
 * Turns a std::set of types into a string for error messaging.
 * 
 * Used by Array::type_check() when throwing an error.
 */
template <typename ArrayTypesCmp>
static std::string array_types_to_str(std::set<Type, ArrayTypesCmp> const& types)
{
	std::string types_s;
	// Reserve average string length of a type (4.8) plus number of comma/space characters.
	types_s.reserve((types.size() * 4.8) + ((types.size() - 1) * 2));

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
	/* Inserts the types of all elements to a set. If the set has more than one
	 * element, then at least two elements have different types. */

	std::set<Type, decltype(&array_types_cmp)> types(&array_types_cmp);

	for (auto const& element : elements)
	{
		assert(element);

		auto type = element->type_check(scope);

		if (type.has_value())
			types.insert(type.value());
	}

	if (types.empty())
	{
		/* The type of the array cannot be deduced. Return std::nullopt. */

		return std::nullopt;
	}
	else if (types.size() != 1)
	{
		/* At least two elements have different types. */

		night::error::get().create_warning(
			"Array elements have different types " + array_types_to_str(types), loc);

		return std::nullopt;
	}
	else
	{
		/* All elements are the same type. */

		assert(types.size() == 1);

		Type element_type = *std::begin(types);
		// Return the same primitive with one higher dimension.
		return Type(element_type.get_prim(), element_type.get_dim() + 1);
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

	for (std::size_t i = 0; i < elements.size(); ++i)
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
	Primitive const _type,
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
	Primitive _type,
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


expr::FunctionCall::FunctionCall(
	Token const& _name,
	std::vector<expr::expr_p> const& _arg_exprs,
	std::optional<uint64_t> const& _id)
	: Expression(_name.loc, Expression::single_precedence)
	, name(_name)
	, arg_exprs(_arg_exprs)
	, id(_id) {}

void expr::FunctionCall::insert_node(
	expr::expr_p node,
	expr::expr_p* prev)
{
	node->insert_node(std::make_shared<FunctionCall>(name, arg_exprs));
	*prev = node;
}

std::optional<Type> expr::FunctionCall::type_check(StatementScope& scope) noexcept
{
	// Check argument types

	std::vector<Type> arg_types;
	for (auto& arg_expr : arg_exprs)
	{
		assert(arg_expr);

		auto arg_type = arg_expr->type_check(scope);

		if (arg_type.has_value())
			arg_types.push_back(*arg_type);
	}

	if (night::error::get().has_minor_errors())
		return std::nullopt;

	assert(arg_types.size() == arg_exprs.size());

	// Get all functions with the same name as the function call

	auto [funcs_with_same_name, funcs_with_same_name_end] = StatementScope::functions.equal_range(name.str);

	if (funcs_with_same_name == funcs_with_same_name_end)
	{
		night::error::get().create_minor_error("function call '" + name.str + "' is undefined", name.loc);
		return std::nullopt;
	}

	// match function with ParserScope function based on name and argument types

	for (; funcs_with_same_name != funcs_with_same_name_end; ++funcs_with_same_name)
	{
		if (std::equal(std::begin(arg_types), std::end(arg_types),
			std::begin(funcs_with_same_name->second.param_types), std::end(funcs_with_same_name->second.param_types)))
			break;
	}

	if (funcs_with_same_name == funcs_with_same_name_end)
	{
		std::string s_types;
		for (auto const& type : arg_types)
			s_types += night::to_str(type) + ", ";

		// remove extra comma at the end
		if (s_types.length() >= 2)
			s_types = s_types.substr(0, s_types.size() - 2);

		if (arg_types.empty())
		{
			night::error::get().create_minor_error("function call '" + name.str + "' has no arguments, "
				"and do not match with the parameters in its function definition", name.loc);
		}
		else
		{
			night::error::get().create_minor_error("arguments in function call '" + name.str + "' are of type '" + s_types +
				"', and do not match with the parameters in its function definition", name.loc);
		}
	}

	if (night::error::get().has_minor_errors())
		return std::nullopt;

	id = funcs_with_same_name->second.id;
	return funcs_with_same_name->second.rtn_type;
}

expr::expr_p expr::FunctionCall::optimize(StatementScope const& scope)
{
	for (auto& arg : arg_exprs)
		arg = arg->optimize(scope);

	return std::make_shared<FunctionCall>(name, arg_exprs, id);
}

bytecodes_t expr::FunctionCall::generate_codes() const
{
	assert(id.has_value());

	bytecodes_t codes;

	for (auto const& param : arg_exprs)
	{
		assert(param);

		auto param_codes = param->generate_codes();
		codes.insert(std::end(codes), std::begin(param_codes), std::end(param_codes));
	}

	auto id_bytes = int_to_bytes(id.value());
	codes.insert(std::end(codes), std::begin(id_bytes), std::end(id_bytes));
	codes.push_back(BytecodeType_CALL);

	return codes;
}
