#include "parser/ast/statement.hpp"
#include "parser/statement_scope.hpp"

#include "interpreter/interpreter_scope.hpp"

#include "common/bytecode.hpp"
#include "common/type.hpp"
#include "common/util.hpp"
#include "common/error.hpp"
#include "common/debug.hpp"

#include <limits>
#include <iterator>
#include <ranges>
#include <vector>
#include <memory>
#include <assert.h>

VariableInit::VariableInit(
	std::string const& _name,
	Location const& _name_loc,
	std::string const& _type,
	expr::expr_p const& _expr)
	: name(_name)
	, name_loc(_name_loc)
	, type(_type, 0)
	, expr(_expr)
	, id(std::nullopt)
	, expr_type(std::nullopt) {}

void VariableInit::check(StatementScope& scope)
{
	id = scope.create_variable(name, name_loc, type);
	if (!id.has_value())
		return;

	if (expr)
	{
		expr_type = expr->type_check(scope);

		if (expr_type.has_value() && type != expr_type)
			night::error::get().create_minor_error(
				"Variable '" + name + "' of type '" + night::to_str(type) +
				"' can not be initialized with expression of type '" + night::to_str(*expr_type) + "'.", name_loc);
	}
}

bool VariableInit::optimize(StatementScope& scope)
{
	if (expr)
		expr = expr->optimize(scope);

	return true;
}

bytecodes_t VariableInit::generate_codes() const
{
	assert(id.has_value());

	bytecodes_t bytes;

	if (expr)
		bytes = expr->generate_codes();
	else
		bytes = int_to_bytes<int64_t>(0);

	bytecodes_t id_bytes = int_to_bytes(id.value());
	night::container_concat(bytes, id_bytes);

	bytes.push_back(ByteType_STORE);

	return bytes;
}


ArrayInitialization::ArrayInitialization(
	std::string const& _name,
	Location const& _name_location,
	std::string const& _type,
	std::vector<expr::expr_p> const& _arr_sizes,
	expr::expr_p const& _expr)
	: name(_name)
	, name_loc(_name_location)
	, type(_type, static_cast<int>(_arr_sizes.size()), TypeCategory::Addressable)
	, arr_sizes(_arr_sizes)
	, expr(_expr) {}

void ArrayInitialization::check(StatementScope& scope)
{
	// Type check array sizes.
	for (expr::expr_p& arr_size : arr_sizes)
	{
		if (!arr_size)
			continue;

		std::optional<Type> arr_size_type = arr_size->type_check(scope);

		if (arr_size_type.has_value() && !arr_size_type->is_prim())
			night::error::get().create_minor_error(
				"Found type '" + night::to_str(*arr_size_type) + "' in array size.\n"
				"Expected primitive type.", name_loc);
	}

	id = scope.create_variable(name, name_loc, type);
	if (!id.has_value())
		return;

	if (expr)
	{
		expr_type = expr->type_check(scope);

		if (expr_type.has_value() && type != expr_type)
			night::error::get().create_minor_error(
				"Variable '" + name + "' of type '" + night::to_str(type) +
				"' can not be initialized with expression of type '" + night::to_str(*expr_type) + "'", name_loc);
	}
}

bool ArrayInitialization::optimize(StatementScope& scope)
{
	for (expr::expr_p& arr_size : arr_sizes)
	{
		if (!arr_size)
		{
			arr_sizes_numerics.push_back(-1);
			continue;
		}

		arr_size = arr_size->optimize(scope);

		if (auto arr_size_numeric = std::dynamic_pointer_cast<expr::Numeric>(arr_size))
			std::visit([&](auto&& arg) { arr_sizes_numerics.push_back((int)arg); }, arr_size_numeric->val);
	}

	if (expr)
		expr = expr->optimize(scope);
	else
		expr = std::make_shared<expr::Array>(name_loc, std::vector<expr::expr_p>(), false);
	
	fill_array(type, expr, 0);

	return true;
}

bytecodes_t ArrayInitialization::generate_codes() const
{
	assert(id.has_value());
	assert(expr);

	bytecodes_t bytes = expr->generate_codes();

	auto id_bytes = int_to_bytes(id.value());
	night::container_concat(bytes, id_bytes);

	bytes.push_back(ByteType_STORE);

	return bytes;
}

void ArrayInitialization::fill_array(Type const& type, expr::expr_p expr, int depth) const
{
	assert(expr);
	assert(0 <= depth && depth < arr_sizes_numerics.size());

	if (auto arr = std::dynamic_pointer_cast<expr::Array>(expr))
	{
		if (depth == arr_sizes_numerics.size() - 1)
		{
			// arr_sizes_numerics can be -1 to signal size was not specified, so use
			// signed type int instead of unsigned type std::size_t
			for (int i = (int)arr->elements.size(); i < arr_sizes_numerics[depth]; ++i)
			{
				arr->elements.push_back(std::make_shared<expr::Numeric>(name_loc, type.get_prim(), (int64_t)0));
			}

			return;
		}
		else if (arr_sizes_numerics[depth] != -1)
		{
			for (int i = (int)arr->elements.size(); i < arr_sizes_numerics[depth]; ++i)
				arr->elements.push_back(std::make_shared<expr::Array>(name_loc, std::vector<expr::expr_p>(), false));
		}

		if (arr_sizes_numerics[depth] != -1)
			arr->elements.resize(arr_sizes_numerics[depth]);

		for (int i = 0; i < arr->elements.size(); ++i)
			fill_array(type, arr->elements[i], depth + 1);
	}
}


Conditional::Conditional(
	Location const& _loc,
	conditional_container const& _conditionals)
	: loc(_loc)
	, conditionals(_conditionals) {}

void Conditional::check(StatementScope& scope)
{
	for (auto const& [cond, block] : conditionals)
	{
		if (cond)
		{
			auto cond_type = cond->type_check(scope);

			if (cond_type.has_value() && cond_type != Primitive::BOOL)
				night::error::get().create_minor_error(
					"Condition is type '" + night::to_str(cond_type.value()) + "'.\n"
					"Expected type 'bool'.\n", loc);
		}

		StatementScope conditional_scope(scope);
		for (auto& stmt : block)
			stmt->check(conditional_scope);
	}
}

bool Conditional::optimize(StatementScope& scope)
{
	std::vector<int> remove_stmts_index;

	for (int i = 0; i < conditionals.size(); ++i)
	{
		auto& [condition, body] = conditionals[i];

		if (condition)
		{
			condition = condition->optimize(scope);

			auto condition_lit = std::dynamic_pointer_cast<expr::Numeric>(condition);

			if (condition_lit && !condition_lit->is_true())
			{
				night::error::get().create_warning("False conditional.", loc);

				remove_stmts_index.push_back(i);
			}
		}

		for (auto& stmt : body)
			stmt->optimize(scope);
	}

	for (int i : remove_stmts_index)
		conditionals.erase(std::begin(conditionals) + i);

	return conditionals.size();
}

bytecodes_t Conditional::generate_codes() const
{
	bytecodes_t codes;
	std::vector<int> jump_offsets;

	for (auto const& [cond_expr, stmts] : conditionals)
	{
		// If condition exists add its codes, otherwise treat it as an else statement
		// and add a true constant
		if (cond_expr)
		{
			auto cond_codes = cond_expr->generate_codes();
			codes.insert(std::end(codes), std::begin(cond_codes), std::end(cond_codes));
		}
		else
		{
			auto numeric_true = expr::Numeric(loc, Primitive::BOOL, 1);
			auto cond_codes = numeric_true.generate_codes();
			codes.insert(std::end(codes), std::begin(cond_codes), std::end(cond_codes));
		}

		// The value for JUMP_IF_FALSE is added after we determine the number of codes
		// to jump over
		codes.push_back(BytecodeType_JUMP_IF_FALSE);

		uint64_t jump_if_false_index = codes.size() - 1;

		// Used to determine number of codes to jump over for JUMP_IF_FALSE
		std::size_t stmts_codes_size = 0;

		for (auto const& stmt : stmts)
		{
			auto stmt_codes = stmt->generate_codes();
			codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));

			stmts_codes_size += stmt_codes.size();
		}

		// Insert jump if false value before JUMP_IF_FALSE, adding on 10 space for the
		// 10 codes to represent JUMP (1) and its value (9)
		auto jump_if_false_codes = int_to_bytes<uint64_t>(stmts_codes_size + 10);
		night::container_insert(codes, jump_if_false_codes, jump_if_false_index);

		// The value for JUMP is added last after the number of codes to jump back is
		// determined
		codes.push_back(BytecodeType_JUMP);
		jump_offsets.push_back((int)codes.size() - 1);
	}

	for (int i = (int)jump_offsets.size() - 1; i >= 0; --i)
	{
		auto offset_codes = int_to_bytes<uint64_t>(codes.size() - jump_offsets[i] - 1);
		night::container_insert(codes, offset_codes, jump_offsets[i]);
	}

	return codes;
}


While::While(
	Location const& _loc,
	expr::expr_p const& _cond,
	std::vector<stmt_p> const& _block)
	: loc(_loc)
	, cond_expr(_cond)
	, block(_block) {}

void While::check(StatementScope& scope)
{
	auto cond_type = cond_expr->type_check(scope);

	if (cond_type.has_value() && cond_type->is_arr())
		night::error::get().create_minor_error(
			"condition is type '" + night::to_str(*cond_type) + "', "
			"expected type 'bool', 'char', 'int', or 'float'", loc);

	StatementScope while_scope(scope);

	for (auto& stmt : block)
		stmt->check(while_scope);
}

bool While::optimize(StatementScope& scope)
{
	cond_expr = cond_expr->optimize(scope);

	for (auto& stmt : block)
		stmt->optimize(scope);

	auto lit = std::dynamic_pointer_cast<expr::Numeric>(cond_expr);

	if (lit && !lit->is_true())
		night::error::get().create_warning("False loop.", loc);

	return !lit || lit->is_true();
}

bytecodes_t While::generate_codes() const
{
	auto codes = cond_expr->generate_codes();

	// The value for JUMP_IF_FALSE is added after we determine the number of codes
	// to jump over
	codes.push_back(BytecodeType_JUMP_IF_FALSE);
	auto jump_if_false_index = codes.size() - 1;

	// Used to determine number of codes to jump over for JUMP_IF_FALSE
	uint64_t stmt_codes_size = 0;

	for (auto const& stmt : block)
	{
		auto stmt_codes = stmt->generate_codes();
		codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));

		stmt_codes_size += stmt_codes.size();
	}

	// Insert jump if false value before JUMP_IF_FALSE, adding on 10 space for the
	// 10 codes to represent JUMP_N (1) and its value (9)
	auto jump_if_false_codes = int_to_bytes(stmt_codes_size + 10);
	night::container_insert(codes, jump_if_false_codes, jump_if_false_index);

	// Insert jump negative value and JUMP_N
	// Set jump negative value to be 8 bit
	auto jump_n_bytes = int_to_bytes<uint64_t>(codes.size() + 9);
	codes.insert(std::end(codes), std::begin(jump_n_bytes), std::end(jump_n_bytes));
	codes.push_back(ByteType_JUMP_N);

	return codes;
}


For::For(
	Location const& _loc,
	VariableInit const& _var_init,
	expr::expr_p const& _cond_expr,
	std::vector<stmt_p> const& _block)
	: var_init(_var_init), loop(_loc, _cond_expr, _block) {}

void For::check(StatementScope& scope)
{
	local_scope = StatementScope(scope);

	var_init.check(local_scope);
	loop.check(local_scope);
}

bool For::optimize(StatementScope& scope)
{
	var_init.optimize(local_scope);
	return loop.optimize(local_scope);
}

bytecodes_t For::generate_codes() const
{
	auto codes = var_init.generate_codes();

	auto loop_codes = loop.generate_codes();
	codes.insert(std::end(codes), std::begin(loop_codes), std::end(loop_codes));

	return codes;
}


Parameter::Parameter(
	std::string const& _name,
	Type const& _type,
	Location const& _location)
	: name(_name)
	, type(_type)
	, location(_location) {}

Function::Function(
	std::string const& _name,
	Location const& _name_location,
	std::vector<Parameter> const& _parameters,
	std::string const& _rtn_type,
	int rtn_type_dim,
	std::vector<stmt_p> const& _body)
	: Statement()
	, name(_name)
	, name_location(_name_location)
	, parameters(_parameters)
	, body(_body)
	, id(std::nullopt)
{
	if (_rtn_type == "void")
		rtn_type = std::nullopt;
	else
		rtn_type = Type(_rtn_type, rtn_type_dim);
}

void Function::check(StatementScope& global_scope)
{
	StatementScope func_scope(global_scope, rtn_type);

	for (auto const& parameter : parameters)
	{
		auto param_id = func_scope.create_variable(parameter.name, parameter.location, parameter.type);

		if (param_id.has_value())
			parameter_ids.push_back(param_id.value());
	}

	auto parameter_names = parameters | std::views::transform([](Parameter const& p) { return p.name; });
	auto parameter_types = parameters | std::views::transform([](Parameter const& p) { return p.type; });

	// Create the function now in ParserScope so it will be defined in recursive calls.
	id = StatementScope::create_function(
		name,
		name_location,
		std::vector<std::string>(std::begin(parameter_names), std::end(parameter_names)),
		std::vector<Type>(std::begin(parameter_types), std::end(parameter_types)),
		rtn_type
	);

	for (auto& statement : body)
		statement->check(func_scope);
}

bool Function::optimize(StatementScope& scope)
{
	return true;
}

bytecodes_t Function::generate_codes() const
{
	assert(id.has_value());
	assert(parameter_ids.size() == parameters.size());

	InterpreterScope::funcs[id.value()] = {};

	for (auto const& param_id : parameter_ids)
		InterpreterScope::funcs[id.value()].param_ids.push_back(param_id);

	for (auto const& stmt : body)
	{
		auto stmt_codes = stmt->generate_codes();
		InterpreterScope::funcs[id.value()].codes.insert(
			std::end(InterpreterScope::funcs[id.value()].codes),
			std::begin(stmt_codes), std::end(stmt_codes));
	}

	return {};
}


Return::Return(
	Location const& _loc,
	expr::expr_p const& _expr)
	: loc(_loc)
	, expr(_expr) {}

void Return::check(StatementScope& scope)
{
	auto expr_type = expr->type_check(scope);

	if (expr_type.has_value())
	{
		if (!scope.return_type.has_value()) {
			night::error::get().create_minor_error(
				"found return type '" + night::to_str(*expr_type) + "', "
				"expected void return type", loc);

			return;
		}

		if (scope.return_type != expr_type)
			night::error::get().create_minor_error(
				"Expected return type of " + night::to_str(scope.return_type.value()) + " to match the functions return type.\n"
				"Found return type of " + night::to_str(expr_type.value()) + ".", loc);
	}
	else
	{
		if (scope.return_type.has_value())
			night::error::get().create_minor_error(
				"found void return type, expected return type '" +
				night::to_str(*scope.return_type) + "'", loc);
	}
}

bool Return::optimize(StatementScope& scope)
{
	expr = expr->optimize(scope);
	return true;
}

bytecodes_t Return::generate_codes() const
{
	auto codes = expr->generate_codes();

	codes.push_back(BytecodeType_RETURN);

	return codes;
}


expr::ExpressionStatement::ExpressionStatement(expr::expr_p const& _expr, Location const& loc)
	: Expression(loc, Expression::single_precedence)
	, expr(_expr) {}

void expr::ExpressionStatement::insert_node(expr::expr_p node, expr::expr_p* prev)
{
	node->insert_node(std::make_shared<ExpressionStatement>(expr, loc));
	*prev = node;
}

void expr::ExpressionStatement::check(StatementScope& scope)
{
	this->type_check(scope);
}

std::optional<Type> expr::ExpressionStatement::type_check(StatementScope& scope) noexcept
{
	return expr->type_check(scope);
}

bool expr::ExpressionStatement::optimize(StatementScope& scope)
{
	expr = expr->optimize(scope);
	return true;
}

expr::expr_p expr::ExpressionStatement::optimize(StatementScope const& scope)
{
	return expr->optimize(scope);
}

bytecodes_t expr::ExpressionStatement::generate_codes() const
{
	return expr->generate_codes();
}
