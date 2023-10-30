#include "ast/statement.hpp"

#include "bytecode.hpp"
#include "interpreter_scope.hpp"
#include "parser_scope.hpp"
#include "scope_check.hpp"
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
	std::vector<expr::expr_p> const& _arr_sizes,
	expr::expr_p const& _expr)
	: Statement(_loc), name(_name), type(_type), arr_sizes(_arr_sizes), expr(_expr) {}

void VariableInit::check(ParserScope& scope)
{
	// Process
	//   1. Determine the type of the variable.
	//   2. Determine the type of the expression.
	//   3. If there are minor errors, return.
	//   4. Compare variable and expression types.

	id = scope.create_variable(name, type, loc);

	// Determine type of variable.

	array_dim dimensions;
	for (auto const& arr_size : arr_sizes)
	{
		// For each array size:
		//   1. If null, add std::nullopt.
		//   2. Type check, skip if null type, and create error if not a primitive.
		//   3. Optimize, create error if not numeric.
		//   4. If no minor errors, push numeric into 'dimensions'

		// Check null

		if (!arr_size)
		{
			dimensions.push_back(std::nullopt);
			continue;
		}

		// Type check

		auto arr_size_type = arr_size->type_check(scope);

		if (!arr_size_type.has_value())
			continue;

		if (arr_size_type->is_arr())
			night::error::get().create_minor_error(
				"found type '" + night::to_str(*arr_size_type) + "' in array size,"
				"expected primitive type", loc);

		// Optimize

		auto arr_size_optimize = arr_size->optimize(scope);
		auto arr_size_numeric = std::dynamic_pointer_cast<expr::Numeric>(arr_size_optimize);

		if (!arr_size_numeric)
			night::error::get().create_minor_error(
				"array size must evaluate to a numeric constant", loc);

		// If type is ok and evaluates to a numeric constant

		if (!night::error::get().has_minor_errors())
			std::visit([&dimensions](auto&& arg)
				{ dimensions.push_back((std::size_t)arg); },
				arr_size_numeric->val);
	}

	var_type = ValueType(type, dimensions);

	// Determine type of expression.

	if (expr)
		expr_type = expr->type_check(scope);
	else
		expr_type = std::nullopt;

	// Return if there are minor errors.

	if (night::error::get().has_minor_errors() || !expr_type.has_value())
		return;

	assert(var_type.dim.size() == arr_sizes.size());

	// Compare variable type with expression type.

	if (var_type.type != var_type.type || expr_type->dim.size() != expr_type->dim.size())
		night::error::get().create_minor_error(
			"variable '" + name + "' of type '" + night::to_str(type) +
			"' can not be initialized with expression of type '" + night::to_str(*expr_type) + "'", loc);
}

bool VariableInit::optimize(ParserScope& scope)
{
	if (expr)
		expr = expr->optimize(scope);

	return true;
}

bytecodes_t VariableInit::generate_codes() const
{
	assert(id.has_value());

	// Populate expression array

	bytecodes_t codes = populate_array(expr, 0);

	if (var_type == ValueType::BOOL && expr_type == ValueType::FLOAT)
		codes.push_back((bytecode_t)BytecodeType::F2B);
	else if (var_type == ValueType::FLOAT && expr_type != ValueType::FLOAT)
		codes.push_back((bytecode_t)BytecodeType::I2F);
	else if (var_type != ValueType::FLOAT && expr_type == ValueType::FLOAT)
		codes.push_back((bytecode_t)BytecodeType::F2I);

	codes.push_back((bytecode_t)BytecodeType::STORE);
	codes.push_back(*id);

	return codes;
}

bytecodes_t VariableInit::populate_array(expr::expr_p expr, int level) const
{
	if (!expr)
	{
		if (level == var_type.dim.size())
		{
			auto num = std::make_shared<expr::Numeric>(loc, var_type.type, (int64_t)0);
			return num->generate_codes();
		}
		else
		{
			assert(var_type.dim[level].has_value());

			int expected_elements = *var_type.dim[level];

			bytecodes_t codes;

			for (int i = 0; i < expected_elements; ++i)
			{
				auto elem_codes = populate_array(nullptr, level + 1);
				codes.insert(std::end(codes), std::begin(elem_codes), std::end(elem_codes));
			}

			auto size_codes = int_to_bytecodes(expected_elements);
			codes.insert(std::end(codes), std::begin(size_codes), std::end(size_codes));

			codes.push_back((bytecode_t)BytecodeType::ALLOCATE_ARR);
			return codes;
		}
	}


	auto arr = std::dynamic_pointer_cast<expr::Array>(expr);

	if (!arr)
		return expr->generate_codes();

	int actual_elements   = arr->elements.size();
	int expected_elements = actual_elements;
	if (var_type.dim[level].has_value())
		expected_elements = *var_type.dim[level];

	bytecodes_t codes;

	if (actual_elements < expected_elements)
	{
		int i = 0;

		for (; i < actual_elements; ++i)
		{
			auto elem_codes = populate_array(arr->elements[i], level + 1);
			codes.insert(std::end(codes), std::begin(elem_codes), std::end(elem_codes));
		}

		for (; i < expected_elements; ++i)
		{
			auto elem_codes = populate_array(nullptr, level + 1);
			codes.insert(std::end(codes), std::begin(elem_codes), std::end(elem_codes));
		}
	}
	else
	{
		for (int i = 0; i < expected_elements; ++i)
		{
			auto elem_codes = populate_array(arr->elements[i], level + 1);
			codes.insert(std::end(codes), std::begin(elem_codes), std::end(elem_codes));
		}
	}

	auto size_codes = int_to_bytecodes(expected_elements);
	codes.insert(std::end(codes), std::begin(size_codes), std::end(size_codes));

	codes.push_back((bytecode_t)BytecodeType::ALLOCATE_ARR);
	return codes;
}


VariableAssign::VariableAssign(
	Location const& _loc,
	std::string const& _var_name,
	std::string const& _assign_op,
	expr::expr_p const& _expr)
	: Statement(_loc), var_name(_var_name), assign_op(_assign_op), expr(_expr), assign_type(std::nullopt), id(std::nullopt) {}

void VariableAssign::check(ParserScope& scope)
{
	assert(expr);

	auto var = scope.get_var(var_name);

	if (!var)
		night::error::get().create_minor_error("variable '" + var_name + "' is undefined", loc);

	auto expr_type = expr->type_check(scope);

	if (!expr_type.has_value() || night::error::get().has_minor_errors())
		return;

	assert(var);
	id = var->id;

	if (!is_same_or_primitive(var->type, *expr_type))
		night::error::get().create_minor_error(
			"variable '" + var_name + "' of type '" + night::to_str(var->type) +
			"can not be assigned to type '" + night::to_str(*expr_type) + "'", loc);

	assign_type = *expr_type;

}

bool VariableAssign::optimize(ParserScope& scope)
{
	scope.use(var_name);

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
			if (is_same(*assign_type, value_type_str))
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

	codes.push_back((bytecode_t)BytecodeType::STORE);
	codes.push_back(*id);

	return codes;
}


Conditional::Conditional(
	Location const& _loc,
	std::vector<std::pair<expr::expr_p, std::vector<stmt_p>> > const& _conditionals)
	: Statement(_loc), conditionals(_conditionals) {}

void Conditional::check(ParserScope& scope)
{
	for (auto const& [cond, block] : conditionals)
	{
		auto cond_type = cond->type_check(scope);

		if (cond_type.has_value() && cond_type->is_arr())
			night::error::get().create_minor_error(
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
		night::error::get().create_minor_error(
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
	ParserScope for_scope(scope);

	var_init.check(for_scope);
	loop.check(for_scope);
}

bool For::optimize(ParserScope& scope)
{
	var_init.optimize(scope);
	loop.optimize(scope);

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
	std::vector<std::pair<std::string, std::string>> const& _parameters,
	std::string const& _rtn_type,
	std::vector<stmt_p> const& _block)
	: Statement(_loc), name(_name), block(_block)
{
	for (const auto& param : _parameters)
	{
		param_names.push_back(param.first);
		param_types.emplace_back(param.second);
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
		night::error::get().create_minor_error(e, loc);
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
			night::error::get().create_minor_error(
				"found return type '" + night::to_str(*expr_type) + "', "
				"expected void return type", loc);

		if (!is_same_or_primitive(*scope.rtn_type, *expr_type))
			night::error::get().create_minor_error(
				"found return type '" + night::to_str(*expr_type) + "', "
				"expected return type '" + night::to_str(*scope.rtn_type) + "'", loc);
	}
	else
	{
		if (scope.rtn_type.has_value())
			night::error::get().create_minor_error(
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
		night::error::get().create_minor_error("variable '" + var_name + "' is undefined", loc);

	for (auto const& subscript : subscripts)
	{
		assert(subscript);

		auto subscript_type = subscript->type_check(scope);
		if (subscript_type.has_value() && subscript_type->is_arr())
			night::error::get().create_minor_error("subscript is type '" + night::to_str(*subscript_type) + "', expected type bool, char, or int'", loc);
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
			else if (is_same(*assign_type, value_type_str))
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

	codes.push_back((bytecode_t)BytecodeType::STORE_INDEX);
	codes.push_back(*id);

	return codes;
}


expr::FunctionCall::FunctionCall(
	Location const& _loc,
	std::string const& _name,
	std::vector<expr::expr_p> const& _arg_exprs,
	std::optional<bytecode_t> const& _id)
	: Statement(_loc), Expression(_loc, expr::ExpressionType::FUNCTION_CALL), name(_name), arg_exprs(_arg_exprs), id(_id), is_expr(true) {}

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

std::optional<ValueType> expr::FunctionCall::type_check(ParserScope const& scope) noexcept
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

		night::error::get().create_minor_error("arguments in function call '" + name + "' are of type '" + s_types +
			"', and do not match with the parameters in its function definition", Statement::loc);
	}

	if (is_expr && !func->second.rtn_type.has_value())
		night::error::get().create_minor_error("function '" + func->first + "' can not have a return type of void when used in an expression", Statement::loc);

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

int expr::FunctionCall::precedence() const
{
	return single_prec;
}