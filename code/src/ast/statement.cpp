#include "ast/statement.hpp"

#include "bytecode.hpp"
#include "interpreter_scope.hpp"
#include "statement_scope.hpp"
#include "type.hpp"
#include "util.hpp"
#include "error.hpp"
#include "debug.hpp"

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

		if (expr_type.has_value() && !is_same_or_primitive(type, expr_type))
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

	if (type != Type::FLOAT && expr_type == Type::FLOAT)
		bytes.push_back(BytecodeType_F2I);
	else if (type == Type::FLOAT && expr_type != Type::FLOAT)
		bytes.push_back(BytecodeType_I2F);

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
	, type(_type, (int)_arr_sizes.size())
	, arr_sizes(_arr_sizes)
	, expr(_expr) {}

void ArrayInitialization::check(StatementScope& scope)
{
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

		if (expr_type.has_value() && !is_same_or_primitive(type, expr_type))
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
			for (int i = (int)arr->elements.size(); i < arr_sizes_numerics[depth]; ++i)
			{
				arr->elements.push_back(std::make_shared<expr::Numeric>(name_loc, type.prim, (int64_t)0));
				arr->type_conversion.push_back(std::nullopt);
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


VariableAssign::VariableAssign(
	std::string const& _name,
	Location const& _name_loc,
	std::string const& _assign_op,
	expr::expr_p const& _expr)
	: name(_name)
	, name_loc(_name_loc)
	, assign_op(_assign_op)
	, expr(_expr)
	, assign_type(std::nullopt)
	, id(std::nullopt) {}

void VariableAssign::check(StatementScope& scope)
{
	assert(expr);

	auto variable = scope.get_variable(name, name_loc);
	if (variable)
		id = variable->id;

	auto expr_type = expr->type_check(scope);
	if (!expr_type.has_value())
		return;

	if (variable && !is_same_or_primitive(variable->type, *expr_type))
		night::error::get().create_minor_error(
			"Variable '" + name + "' of type '" + night::to_str(variable->type) +
			"can not be assigned to type '" + night::to_str(*expr_type) + "'.", name_loc);

	assign_type = expr_type.value();
}

bool VariableAssign::optimize(StatementScope& scope)
{
	expr = expr->optimize(scope);
	return true;
}

bytecodes_t VariableAssign::generate_codes() const
{
	static std::unordered_map<std::string, std::pair<bytecode_t, bytecode_t>> operator_bytes{
		{ "+=", {BytecodeType_ADD_I, BytecodeType_ADD_F} },
		{ "-=", {BytecodeType_SUB_I, BytecodeType_SUB_F} },
		{ "*=", {BytecodeType_MULT_I, BytecodeType_MULT_F} },
		{ "/=", {BytecodeType_DIV_I, BytecodeType_DIV_F} }
	};

	bytecodes_t bytes;

	if (assign_op != "=")
	{
		auto id_codes = int_to_bytes(id.value());
		night::container_concat(bytes, id_codes);

		bytes.push_back(ByteType_LOAD);

		auto expr_codes = expr->generate_codes();
		night::container_concat(bytes, expr_codes);

		assert(operator_bytes.contains(assign_op));
		auto [int_op, float_op] = operator_bytes[assign_op];

		// Special case for strings.
		if (assign_op == "+=" && is_same(*assign_type, Type(Type::CHAR, 1)))
			bytes.push_back(BytecodeType_ADD_S);
		else
			bytes.push_back(assign_type == Type::FLOAT ? float_op : int_op);
	}
	else
	{
		auto expr_codes = expr->generate_codes();
		night::container_concat(bytes, expr_codes);
	}

	auto id_bytes = int_to_bytes(id.value());
	night::container_concat(bytes, id_bytes);

	bytes.push_back(ByteType_STORE);

	return bytes;
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

			if (cond_type.has_value() && cond_type->is_arr())
				night::error::get().create_minor_error(
					"expected type 'bool', 'char', 'int', or float."
					"condition is type '" + night::to_str(*cond_type) + "'", loc);
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
			auto numeric_true = expr::Numeric(loc, Type::BOOL, 1);
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

		if (!is_same_or_primitive(scope.return_type, *expr_type))
			night::error::get().create_minor_error(
				"found return type '" + night::to_str(*expr_type) + "', "
				"expected return type '" + night::to_str(*scope.return_type) + "'", loc);
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


ArrayMethod::ArrayMethod(
	Location const& _loc,
	std::string const& _var_name,
	std::string const& _assign_op,
	std::vector<expr::expr_p> const& _subscripts,
	expr::expr_p const& _assign_expr)
	: name_loc(_loc), var_name(_var_name), assign_op(_assign_op), subscripts(_subscripts), assign_expr(_assign_expr), id(std::nullopt) {}

void ArrayMethod::check(StatementScope& scope)
{
	auto variable = scope.get_variable(var_name, name_loc);
	if (variable)
		id = variable->id;

	for (auto const& subscript : subscripts)
	{
		assert(subscript);

		auto subscript_type = subscript->type_check(scope);
		if (subscript_type.has_value() && subscript_type->is_arr())
			night::error::get().create_minor_error("subscript is type '" + night::to_str(*subscript_type) + "', expected type bool, char, or int'", name_loc);
	}

	if (assign_expr)
		assign_type = assign_expr->type_check(scope);

	if (variable && assign_expr && (variable->type.dim - subscripts.size() != assign_type->dim || variable->type.prim != assign_type->prim))
		night::error::get().create_minor_error(
			"Variable '" + var_name +
			"' can not be assigned to type '" + night::to_str(assign_type.value()) + "'.", name_loc);
}

bool ArrayMethod::optimize(StatementScope& scope)
{
	return true;
}

bytecodes_t ArrayMethod::generate_codes() const
{
	assert(id.has_value());
	assert(assign_expr);

	bytecodes_t codes;

	for (int i = (int)subscripts.size() - 1; i >= 0; --i)
	{
		assert(subscripts[i]);

		auto subscript_codes = subscripts[i]->generate_codes();
		codes.insert(std::end(codes), std::begin(subscript_codes), std::end(subscript_codes));
	}

	if (assign_op != "=")
	{
		for (int i = (int)subscripts.size() - 1; i >= 0; --i)
		{
			assert(subscripts[i]);

			auto subscript_codes = subscripts[i]->generate_codes();
			codes.insert(std::end(codes), std::begin(subscript_codes), std::end(subscript_codes));
		}

		auto num = int_to_bytes<uint64_t>(subscripts.size());
		codes.insert(std::end(codes), std::begin(num), std::end(num));

		auto id_codes = int_to_bytes(id.value());
		codes.insert(std::end(codes), std::begin(id_codes), std::end(id_codes));
		codes.push_back(BytecodeType_LOAD_ELEM);

		auto assign_codes = assign_expr->generate_codes();
		codes.insert(std::end(codes), std::begin(assign_codes), std::end(assign_codes));

		if (assign_op == "+=")
		{
			if (assign_type == Type::FLOAT)
				codes.push_back(BytecodeType_ADD_F);
			else if (is_same(*assign_type, Type(Type::CHAR, 1)))
				codes.push_back(BytecodeType_ADD_S);
			else
				codes.push_back(BytecodeType_ADD_I);
		}
		else if (assign_op == "-=")
		{
			if (assign_type == Type::FLOAT)
				codes.push_back(BytecodeType_SUB_F);
			else
				codes.push_back(BytecodeType_SUB_I);
		}
		else if (assign_op == "*=")
		{
			if (assign_type == Type::FLOAT)
				codes.push_back(BytecodeType_MULT_F);
			else
				codes.push_back(BytecodeType_MULT_I);
		}
		else if (assign_op == "/=")
		{
			if (assign_type == Type::FLOAT)
				codes.push_back(BytecodeType_DIV_F);
			else
				codes.push_back(BytecodeType_DIV_I);
		}
		else
			throw debug::unhandled_case(assign_op);
	}
	else
	{
		auto assign_codes = assign_expr->generate_codes();
		codes.insert(std::end(codes), std::begin(assign_codes), std::end(assign_codes));
	}

	auto index_bytes = int_to_bytes(id.value());
	codes.insert(std::end(codes), std::begin(index_bytes), std::end(index_bytes));

	if (assign_type.has_value() && assign_type->prim == Type::Primitive::CHAR && assign_type->dim == 0)
		codes.push_back(BytecodeType_STORE_INDEX_S);
	else
		codes.push_back(BytecodeType_STORE_INDEX_A);

	return codes;
}


expr::FunctionCall::FunctionCall(
	Token const& _name,
	std::vector<expr::expr_p> const& _arg_exprs,
	std::optional<uint64_t> const& _id)
	: Expression(_name.loc, Expression::single_precedence)
	, name(_name)
	, arg_exprs(_arg_exprs)
	, id(_id)
	, is_expr(true) {}

void expr::FunctionCall::insert_node(
	expr::expr_p node,
	expr::expr_p* prev)
{
	node->insert_node(std::make_shared<FunctionCall>(name, arg_exprs));
	*prev = node;
}

void expr::FunctionCall::check(StatementScope& scope)
{
	is_expr = false;
	this->type_check(scope);
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
					   std::begin(funcs_with_same_name->second.param_types), std::end(funcs_with_same_name->second.param_types),
					   is_same))
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

	if (is_expr && !funcs_with_same_name->second.rtn_type.has_value())
		night::error::get().create_minor_error("function '" + funcs_with_same_name->first + "' can not have a return type of void when used in an expression", name.loc);

	if (night::error::get().has_minor_errors())
		return std::nullopt;

	id = funcs_with_same_name->second.id;
	return funcs_with_same_name->second.rtn_type;
}

bool expr::FunctionCall::optimize(StatementScope& scope)
{
	for (auto& arg : arg_exprs)
		arg = arg->optimize(scope);

	return true;
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