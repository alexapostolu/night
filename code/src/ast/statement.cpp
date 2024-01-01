#include "ast/statement.hpp"

#include "bytecode.hpp"
#include "interpreter_scope.hpp"
#include "parser_scope.hpp"
#include "scope_check.hpp"
#include "value_type.hpp"
#include "error.hpp"
#include "debug.hpp"

#include <limits>
#include <vector>
#include <memory>
#include <assert.h>


Statement::Statement(Location const& _loc)
	: loc(_loc) {}


VariableInit::VariableInit(
	Location const& _loc,
	std::string const& _name,
	std::string const& _type,
	expr::expr_p const& _expr)
	: Statement(_loc), name(_name), type(_type), expr(_expr) {}

void VariableInit::check(ParserScope& scope)
{
	var_type = ValueType(type, 0);
	id = scope.create_variable(name, var_type, loc);

	// Return if there are minor errors.

	if (night::error::get().has_minor_errors())
		return;

	// Compare variable type with expression type.

	if (expr)
	{
		expr_type = expr->type_check(scope);

		if (expr_type.has_value() && !is_same_or_primitive(var_type, expr_type))
			night::create_minor_error(
				"variable '" + name + "' of type '" + night::to_str(type) +
				"' can not be initialized with expression of type '" + night::to_str(*expr_type) + "'", loc);
	}
}

bool VariableInit::optimize(ParserScope& scope)
{
	/* Process
	 *   1. If variable is never used, return false, else,
	 *   2. Optimize expression if it exists.
	 *
	 * Note: Array sizes have already been optimized in check(), so there is no
	 * need to optimize them here.
	 */

	if (!scope.is_var_used(name))
		return false;

	if (expr)
		expr = expr->optimize(scope);

	return true;
}

bytecodes_t VariableInit::generate_codes() const
{
	assert(id.has_value());

	// Populate expression array

	bytecodes_t codes;

	if (expr)
		codes = expr->generate_codes();

	if (var_type == ValueType::BOOL && expr_type == ValueType::FLOAT)
		codes.push_back((bytecode_t)BytecodeType::F2B);
	else if (var_type == ValueType::FLOAT && expr_type != ValueType::FLOAT)
		codes.push_back((bytecode_t)BytecodeType::I2F);
	else if (var_type != ValueType::FLOAT && expr_type == ValueType::FLOAT)
		codes.push_back((bytecode_t)BytecodeType::F2I);

	auto id_codes = int_to_bytecodes(*id);
	codes.insert(std::end(codes), std::begin(id_codes), std::end(id_codes));

	codes.push_back((bytecode_t)BytecodeType::STORE);

	return codes;
}


ArrayInitialization::ArrayInitialization(
	Location const& _loc,
	std::string const& _name,
	std::string const& _type,
	std::vector<expr::expr_p> const& _arr_sizes,
	expr::expr_p const& _expr)
	: Statement(_loc), name(_name), type(_type), arr_sizes(_arr_sizes)
	, expr(_expr) {}

void ArrayInitialization::check(ParserScope& scope)
{
	// Type check variable sizes.

	for (auto& arr_size : arr_sizes)
	{
		if (!arr_size)
			continue;

		// Type check by expecting primitive types.

		auto arr_size_type = arr_size->type_check(scope);

		if (arr_size_type.has_value() && arr_size_type->is_arr())
			night::create_minor_error(
				"found type '" + night::to_str(*arr_size_type) + "' in array size, "
				"expected primitive type", loc);
	}

	// Deduce type of variable.

	var_type = ValueType(type, arr_sizes.size());
	id = scope.create_variable(name, var_type, loc);

	// Return if there are minor errors.

	if (night::error::get().has_minor_errors())
		return;

	// Compare variable type with expression type.

	if (expr)
	{
		expr_type = expr->type_check(scope);

		if (expr_type.has_value() && !is_same_or_primitive(var_type, expr_type))
			night::create_minor_error(
				"variable '" + name + "' of type '" + night::to_str(type) +
				"' can not be initialized with expression of type '" + night::to_str(*expr_type) + "'", loc);
	}
}

bool ArrayInitialization::optimize(ParserScope& scope)
{
	/* Process
	 *   1. If variable is never used, return false, else,
	 *   2. Optimize expression if it exists.
	 * 
	 * Note: Array sizes have already been optimized in check(), so there is no
	 * need to optimize them here.
	 */

	for (auto& arr_size : arr_sizes)
	{
		if (!arr_size)
			continue;

		arr_size = arr_size->optimize(scope);

		if (auto arr_size_numeric = std::dynamic_pointer_cast<expr::Numeric>(arr_size))
		{
			std::visit([&](auto&& arg) { arr_sizes_numerics.push_back((int)arg); }, arr_size_numeric->val);
		}
	}

	if (!scope.is_var_used(name))
		return false;

	if (expr)
		expr = expr->optimize(scope);

	if (is_static)
	{
		if (!expr)
			expr = std::make_shared<expr::Array>(loc, std::vector<expr::expr_p>(), false);
		
		fill_array(expr, 0);
	}

	return true;
}

bytecodes_t ArrayInitialization::generate_codes() const
{
	assert(id.has_value());
	assert(expr);

	bytecodes_t codes = expr->generate_codes();

	auto id_codes = int_to_bytecodes(*id);
	codes.insert(std::end(codes), std::begin(id_codes), std::end(id_codes));

	codes.push_back((bytecode_t)BytecodeType::STORE);

	return codes;
}

void ArrayInitialization::fill_array(expr::expr_p expr, int depth) const
{
	assert(expr);

	if (auto arr = std::dynamic_pointer_cast<expr::Array>(expr))
	{
		if (depth == arr_sizes_numerics.size() - 2)
		{
			for (int i = arr->elements.size(); i < arr_sizes_numerics[depth]; ++i)
				arr->elements.push_back(std::make_shared<expr::Numeric>(loc, ValueType(type).type, (int64_t)0));
		}
		else
		{
			for (int i = arr->elements.size(); i < arr_sizes_numerics[depth]; ++i)
				arr->elements.push_back(std::make_shared<expr::Array>(loc, std::vector<expr::expr_p>(), false));
		}

		arr->elements.resize(arr_sizes_numerics[depth]);

		for (int i = 0; i < arr->elements.size(); ++i)
			fill_array(arr->elements[i], depth + 1);
	}
}


VariableAssign::VariableAssign(
	Location const& _loc,
	std::string const& _var_name,
	std::string const& _assign_op,
	expr::expr_p const& _expr)
	: Statement(_loc), var_name(_var_name), assign_op(_assign_op), expr(_expr)
	, assign_type(std::nullopt), id(std::nullopt) {}

void VariableAssign::check(ParserScope& scope)
{
	assert(expr);

	auto var = scope.get_var(var_name);

	if (!var)
		night::create_minor_error("variable '" + var_name + "' is undefined", loc);

	auto expr_type = expr->type_check(scope);

	if (!expr_type.has_value() || night::error::get().has_minor_errors())
		return;

	assert(var);
	id = var->id;

	if (!is_same_or_primitive(var->type, *expr_type))
		night::create_minor_error(
			"variable '" + var_name + "' of type '" + night::to_str(var->type) +
			"can not be assigned to type '" + night::to_str(*expr_type) + "'", loc);

	assign_type = *expr_type;

}

bool VariableAssign::optimize(ParserScope& scope)
{
	expr = expr->optimize(scope);
	return true;
}

bytecodes_t VariableAssign::generate_codes() const
{
	bytecodes_t codes;

	if (assign_op != "=")
	{
		codes.push_back((bytecode_t)BytecodeType::LOAD);
		codes.push_back(*id);

		auto expr_codes = expr->generate_codes();
		codes.insert(std::end(codes), std::begin(expr_codes), std::end(expr_codes));

		if (assign_op == "+=")
		{
			if (is_same(*assign_type, ValueType(ValueType::CHAR, true)))
				codes.push_back((bytecode_t)BytecodeType::ADD_S);
			else if (assign_type == ValueType::FLOAT)
				codes.push_back((bytecode_t)BytecodeType::ADD_F);
			else
				codes.push_back((bytecode_t)BytecodeType::ADD_I);
		}
		else if (assign_op == "-=")
		{
			if (assign_type == ValueType::FLOAT)
				codes.push_back((bytecode_t)BytecodeType::SUB_F);
			else
				codes.push_back((bytecode_t)BytecodeType::SUB_I);
		}
		else if (assign_op == "*=")
		{
			if (assign_type == ValueType::FLOAT)
				codes.push_back((bytecode_t)BytecodeType::MULT_F);
			else
				codes.push_back((bytecode_t)BytecodeType::MULT_I);
		}
		else if (assign_op == "/=")
		{
			if (assign_type == ValueType::FLOAT)
				codes.push_back((bytecode_t)BytecodeType::DIV_F);
			else
				codes.push_back((bytecode_t)BytecodeType::DIV_I);
		}
		else
			throw debug::unhandled_case(assign_op);
	}
	else
	{
		auto expr_codes = expr->generate_codes();
		codes.insert(std::end(codes), std::begin(expr_codes), std::end(expr_codes));
	}

	auto id_codes = int_to_bytecodes(*id);
	codes.insert(std::end(codes), std::begin(id_codes), std::end(id_codes));

	codes.push_back((bytecode_t)BytecodeType::STORE);

	return codes;
}


Conditional::Conditional(
	Location const& _loc,
	conditional_container const& _conditionals)
	: Statement(_loc), conditionals(_conditionals) {}

void Conditional::check(ParserScope& scope)
{
	for (auto const& [cond, block] : conditionals)
	{
		auto cond_type = cond->type_check(scope);

		if (cond_type.has_value() && cond_type->is_arr())
			night::create_minor_error(
				"expected type 'bool', 'char', 'int', or float."
				"condition is type '" + night::to_str(*cond_type) + "'", loc);

		ParserScope conditional_scope(scope);
		for (auto& stmt : block)
			stmt->check(conditional_scope);
	}
}

bool Conditional::optimize(ParserScope& scope)
{
	std::vector<int> remove_stmts_index;

	for (int i = 0; i < conditionals.size(); ++i)
	{
		auto& [condition, body] = conditionals[i];

		condition = condition->optimize(scope);

		auto condition_lit = std::dynamic_pointer_cast<expr::Numeric>(condition);

		if (condition_lit && !condition_lit->is_true())
		{
			remove_stmts_index.push_back(i);
			ParserScope::inside_false_conditional = true;
		}

		for (auto& stmt : body)
			stmt->optimize(scope);

		ParserScope::inside_false_conditional = false;
	}

	for (int i : remove_stmts_index)
		conditionals.erase(std::begin(conditionals) + i);

	return conditionals.size();
}

bytecodes_t Conditional::generate_codes() const
{
	bytecodes_t codes;
	std::vector<int> jumps;

	for (auto const& [cond_expr, stmts] : conditionals)
	{
		// condition
		auto cond_codes = cond_expr->generate_codes();
		codes.insert(std::end(codes), std::begin(cond_codes), std::end(cond_codes));

		codes.push_back((bytecode_t)BytecodeType::JUMP_IF_FALSE);
		int offset_index = codes.size();

		// statements
		for (auto const& stmt : stmts)
		{
			auto stmt_codes = stmt->generate_codes();
			codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));
		}

		// insert offset before JUMP_IF_FALSE
		auto jif_codes = int_to_bytecodes(codes.size() - offset_index + 2);
		codes.insert(std::begin(codes) + offset_index - 1, std::begin(jif_codes), std::end(jif_codes));


		codes.push_back((bytecode_t)BytecodeType::JUMP);
		jumps.push_back(codes.size() + jumps.size());
	}

	// insert offsets after JUMP
	int final_size = codes.size() + jumps.size();
	for (int i = 0; i < jumps.size(); ++i)
		codes.insert(std::begin(codes) + jumps[i], final_size - jumps[i] - 1);

	return codes;
}


While::While(
	Location const& _loc,
	expr::expr_p const& _cond,
	std::vector<stmt_p> const& _block)
	: Statement(_loc), cond_expr(_cond), block(_block) {}

void While::check(ParserScope& scope)
{
	auto cond_type = cond_expr->type_check(scope);

	if (cond_type.has_value() && cond_type->is_arr())
		night::create_minor_error(
			"condition is type '" + night::to_str(*cond_type) + "', "
			"expected type 'bool', 'char', 'int', or 'float'", loc);

	ParserScope while_scope(scope);

	for (auto& stmt : block)
		stmt->check(while_scope);
}

bool While::optimize(ParserScope& scope)
{
	cond_expr = cond_expr->optimize(scope);

	for (auto& stmt : block)
		stmt->optimize(scope);

	auto lit = std::dynamic_pointer_cast<expr::Numeric>(cond_expr);
	return !lit || lit->is_true();
}

bytecodes_t While::generate_codes() const
{

// beq t0 t1 end
// loop: addi t0 t0 1
// bne t0 t1 loop
// end

	auto codes = cond_expr->generate_codes();

	codes.push_back((bytecode_t)BytecodeType::JUMP_IF_FALSE);
	auto offset_index = codes.size();

	for (auto const& stmt : block)
	{
		auto stmt_codes = stmt->generate_codes();
		codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));
	}

	auto jif_codes = int_to_bytecodes(codes.size() - offset_index + 2);

	codes.push_back((bytecode_t)BytecodeType::NJUMP);
	codes.push_back(codes.size() + jif_codes.size() + 1);

	// insert offset before JUMP_IF_FALSE
	codes.insert(std::begin(codes) + offset_index - 1, std::begin(jif_codes), std::end(jif_codes));


	return codes;
}


For::For(
	Location const& _loc,
	VariableInit const& _var_init,
	expr::expr_p const& _cond_expr,
	std::vector<stmt_p> const& _block)
	: Statement(_loc), var_init(_var_init), loop(_loc, _cond_expr, _block) {}

void For::check(ParserScope& scope)
{
	local_scope = ParserScope(scope);

	var_init.check(local_scope);
	loop.check(local_scope);
}

bool For::optimize(ParserScope& scope)
{
	var_init.optimize(local_scope);
	loop.optimize(local_scope);

	return true;
}

bytecodes_t For::generate_codes() const
{
	auto codes = var_init.generate_codes();

	auto loop_codes = loop.generate_codes();
	codes.insert(std::end(codes), std::begin(loop_codes), std::end(loop_codes));

	return codes;
}


Function::Function(
	Location const& _loc,
	std::string const& _name,
	std::vector<std::tuple<std::string, std::string, bool>> const& _parameters,
	std::string const& _rtn_type,
	std::vector<stmt_p> const& _block)
	: Statement(_loc), name(_name), block(_block)
{
	for (const auto& param : _parameters)
	{
		param_names.push_back(std::get<0>(param));
		param_types.emplace_back(std::get<1>(param));
	}

	if (_rtn_type == "void")
		rtn_type = std::nullopt;
	else
		rtn_type = ValueType(_rtn_type);
}

void Function::check(ParserScope& global_scope)
{
	ParserScope func_scope(global_scope, rtn_type);

	for (std::size_t i = 0; i < param_names.size(); ++i)
	{
		auto param_id = func_scope.create_variable(param_names[i], param_types[i], loc);

		if (param_id.has_value())
			param_ids.push_back(*param_id);
	}

	try {
		// define the function now in ParserScope so it will be defined in recursive calls
		auto func_it = ParserScope::create_function(name, param_names, param_types, rtn_type);
		id = func_it->second.id;
	}
	catch (std::string const& e) {
		night::create_minor_error(e, loc);
	}

	for (auto& stmt : block)
		stmt->check(func_scope);
}

bool Function::optimize(ParserScope& scope)
{
	return true;
}

bytecodes_t Function::generate_codes() const
{
	InterpreterScope::funcs[id] = {};

	for (auto const& param_id : param_ids)
		InterpreterScope::funcs[id].param_ids.push_back(param_id);

	for (auto const& stmt : block)
	{
		auto stmt_codes = stmt->generate_codes();
		InterpreterScope::funcs[id].codes.insert(
			std::end(InterpreterScope::funcs[id].codes),
			std::begin(stmt_codes), std::end(stmt_codes));
	}

	return {};
}


Return::Return(
	Location const& _loc,
	expr::expr_p const& _expr)
	: Statement(_loc), expr(_expr) {}

void Return::check(ParserScope& scope)
{
	auto expr_type = expr->type_check(scope);

	if (expr_type.has_value())
	{
		if (!scope.rtn_type.has_value())
			night::create_minor_error(
				"found return type '" + night::to_str(*expr_type) + "', "
				"expected void return type", loc);

		if (!is_same_or_primitive(*scope.rtn_type, *expr_type))
			night::create_minor_error(
				"found return type '" + night::to_str(*expr_type) + "', "
				"expected return type '" + night::to_str(*scope.rtn_type) + "'", loc);
	}
	else
	{
		if (scope.rtn_type.has_value())
			night::create_minor_error(
				"found void return type, expected return type '" +
				night::to_str(*scope.rtn_type) + "'", loc);
	}
}

bool Return::optimize(ParserScope& scope)
{
	expr = expr->optimize(scope);
	return true;
}

bytecodes_t Return::generate_codes() const
{
	auto codes = expr->generate_codes();

	codes.push_back((bytecode_t)BytecodeType::RETURN);

	return codes;
}


ArrayMethod::ArrayMethod(
	Location const& _loc,
	std::string const& _var_name,
	std::string const& _assign_op,
	std::vector<expr::expr_p> const& _subscripts,
	expr::expr_p const& _assign_expr)
	: Statement(_loc), var_name(_var_name), assign_op(_assign_op), subscripts(_subscripts), assign_expr(_assign_expr), id(std::nullopt) {}

void ArrayMethod::check(ParserScope& scope)
{
	auto var = scope.get_var(var_name);

	if (!var)
		night::create_minor_error("variable '" + var_name + "' is undefined", loc);

	for (auto const& subscript : subscripts)
	{
		assert(subscript);

		auto subscript_type = subscript->type_check(scope);
		if (subscript_type.has_value() && subscript_type->is_arr())
			night::create_minor_error("subscript is type '" + night::to_str(*subscript_type) + "', expected type bool, char, or int'", loc);
	}

	if (assign_expr)
	{
		assign_type = assign_expr->type_check(scope);
	}

	id = var->id;
}

bool ArrayMethod::optimize(ParserScope& scope)
{
	return true;
}

bytecodes_t ArrayMethod::generate_codes() const
{
	assert(id.has_value());
	assert(assign_expr);

	bytecodes_t codes;

	for (int i = subscripts.size() - 1; i >= 0; --i)
	{
		assert(subscripts[i]);

		auto subscript_codes = subscripts[i]->generate_codes();
		codes.insert(std::end(codes), std::begin(subscript_codes), std::end(subscript_codes));
	}

	auto assign_codes = assign_expr->generate_codes();
	codes.insert(std::end(codes), std::begin(assign_codes), std::end(assign_codes));

	if (assign_op != "=")
	{
		for (int i = subscripts.size() - 1; i >= 0; --i)
		{
			assert(subscripts[i]);

			auto subscript_codes = subscripts[i]->generate_codes();
			codes.insert(std::end(codes), std::begin(subscript_codes), std::end(subscript_codes));
		}


		auto num = int_to_bytecodes(subscripts.size());
		codes.insert(std::end(codes), std::begin(num), std::end(num));

		codes.push_back((bytecode_t)BytecodeType::LOAD_ELEM);
		codes.push_back(*id);

		if (assign_op == "+=")
		{
			if (assign_type == ValueType::FLOAT)
				codes.push_back((bytecode_t)BytecodeType::ADD_F);
			else if (is_same(*assign_type, ValueType(ValueType::CHAR, 1)))
				codes.push_back((bytecode_t)BytecodeType::ADD_S);
			else
				codes.push_back((bytecode_t)BytecodeType::ADD_I);
		}
		else if (assign_op == "-=")
		{
			if (assign_type == ValueType::FLOAT)
				codes.push_back((bytecode_t)BytecodeType::SUB_F);
			else
				codes.push_back((bytecode_t)BytecodeType::SUB_I);
		}
		else if (assign_op == "*=")
		{
			if (assign_type == ValueType::FLOAT)
				codes.push_back((bytecode_t)BytecodeType::MULT_F);
			else
				codes.push_back((bytecode_t)BytecodeType::MULT_I);
		}
		else if (assign_op == "/=")
		{
			if (assign_type == ValueType::FLOAT)
				codes.push_back((bytecode_t)BytecodeType::DIV_F);
			else
				codes.push_back((bytecode_t)BytecodeType::DIV_I);
		}
		else
			throw debug::unhandled_case(assign_op);
	}

	auto index_codes = int_to_bytecodes(*id);
	codes.insert(std::end(codes), std::begin(index_codes), std::end(index_codes));

	codes.push_back((bytecode_t)BytecodeType::STORE_INDEX);

	return codes;
}


expr::FunctionCall::FunctionCall(
	Location const& _loc,
	std::string const& _name,
	std::vector<expr::expr_p> const& _arg_exprs,
	std::optional<bytecode_t> const& _id)
	: Statement(_loc), Expression(_loc, Expression::single_precedence), name(_name), arg_exprs(_arg_exprs), id(_id), is_expr(true) {}

void expr::FunctionCall::insert_node(
	expr::expr_p node,
	expr::expr_p* prev)
{
	node->insert_node(std::make_shared<FunctionCall>(Statement::loc, name, arg_exprs));
	*prev = node;
}

void expr::FunctionCall::check(ParserScope& scope)
{
	is_expr = false;
	this->type_check(scope);
}

std::optional<ValueType> expr::FunctionCall::type_check(ParserScope& scope) noexcept
{
	auto [func, range_end] = ParserScope::funcs.equal_range(name);

	check_function_defined(scope, name, Statement::loc);

	// check argument types

	std::vector<ValueType> arg_types;
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

	// match function with ParserScope function based on name and argument types

	for (; func != range_end; ++func)
	{
		if (std::equal(std::begin(arg_types), std::end(arg_types),
					   std::begin(func->second.param_types), std::end(func->second.param_types),
					   is_same))
			break;
	}

	if (func == range_end)
	{
		std::string s_types;
		for (auto const& type : arg_types)
			s_types += night::to_str(type) + ", ";

		// remove extra comma at the end
		if (s_types.length() >= 2)
			s_types = s_types.substr(0, s_types.size() - 2);

		night::create_minor_error("arguments in function call '" + name + "' are of type '" + s_types +
			"', and do not match with the parameters in its function definition", Statement::loc);
	}

	if (is_expr && !func->second.rtn_type.has_value())
		night::create_minor_error("function '" + func->first + "' can not have a return type of void when used in an expression", Statement::loc);

	id = func->second.id;
	return func->second.rtn_type;
}

bool expr::FunctionCall::optimize(ParserScope& scope)
{
	for (auto& arg : arg_exprs)
		arg = arg->optimize(scope);

	return true;
}

expr::expr_p expr::FunctionCall::optimize(ParserScope const& scope)
{
	for (auto& arg : arg_exprs)
		arg = arg->optimize(scope);

	return std::make_shared<FunctionCall>(Expression::loc, name, arg_exprs, id);
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

	codes.push_back((bytecode_t)BytecodeType::CALL);
	codes.push_back(*id);

	return codes;
}