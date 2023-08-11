#include "ast/ast.hpp"
#include "bytecode.hpp"
#include "interpreter_scope.hpp"
#include "utils.hpp"
#include "error.hpp"
#include "debug.hpp"

#include <limits>
#include <vector>
#include <memory>
#include <assert.h>

AST::AST(Location const& _loc)
	: loc(_loc) {}

VariableInit::VariableInit(
	Location const& _loc,
	std::string const& _name,
	std::string const& _type,
	std::shared_ptr<expr::Expression> const& _expr)
	: AST(_loc), name(_name), type(token_var_type_to_val_type(_type)), expr(_expr) {}

void VariableInit::check(ParserScope& scope)
{
	// check variable name

	auto [msg, _id] = scope.create_variable(name, type);
	id = _id;

	if (!msg.empty())
		night::error::get().create_minor_error(msg, loc);

	// check expression

	auto expr_type = expr->type_check(scope);

	if (expr_type.has_value() && !compare_value_t(type, *expr_type))
		night::error::get().create_minor_error(
			"variable '" + name + "' of type '" + night::to_str(type) +
			"' can not be initialized with expression of type '" + night::to_str(*expr_type) + "'", loc);
}

bytecodes_t VariableInit::generate_codes() const
{
	bytecodes_t codes = expr->generate_codes();

	codes.push_back((bytecode_t)BytecodeType::STORE);
	codes.push_back(id);

	return codes;
}


VariableAssign::VariableAssign(
	Location const& _loc,
	std::string const& _name,
	std::string const& _assign_op,
	expr::expr_p const& _expr)
	: AST(_loc), name(_name), assign_op(_assign_op), expr(_expr) {}

void VariableAssign::check(ParserScope& scope)
{
	// check variable name

	if (!scope.vars.contains(name))
		night::error::get().create_minor_error("variable '" + name + "' is undefined", loc);

	// check expression

	auto expr_type = expr->type_check(scope);

	if (expr_type.has_value() && assign_op != "=" && is_object_t(*expr_type))
		night::error::get().create_minor_error(
			"assignment '" + assign_op + "' can not be used on expression of type '" + night::to_str(*expr_type) + "'", loc);

	if (expr_type.has_value() && compare_value_t(scope.vars.at(name).type, *expr_type))
		night::error::get().create_minor_error(
			"variable '" + name + "' of type '" + night::to_str(scope.vars.at(name).type) +
			"' can not be initialized with expression of type '" + night::to_str(*expr_type) + "'", loc);

	id = scope.vars[name].id;
}

bytecodes_t VariableAssign::generate_codes() const
{
	assert(id.has_value());

	bytecodes_t codes = expr->generate_codes();

	if (assign_op != "=")
	{
		codes.push_back((bytecode_t)BytecodeType::LOAD);
		codes.push_back(*id);

		if      (assign_op == "+=")	codes.push_back((bytecode_t)BytecodeType::ADD);
		else if (assign_op == "-=") codes.push_back((bytecode_t)BytecodeType::SUB);
		else if (assign_op == "*=") codes.push_back((bytecode_t)BytecodeType::MULT);
		else if (assign_op == "/=") codes.push_back((bytecode_t)BytecodeType::DIV);
		else throw debug::unhandled_case(assign_op);
	}

	codes.push_back((bytecode_t)BytecodeType::STORE);
	codes.push_back(*id);

	return codes;
}


Conditional::Conditional(
	Location const& _loc,
	std::vector<std::pair<expr::expr_p, AST_Block>
	> const& _conditionals)
	: AST(_loc), conditionals(_conditionals) {}

void Conditional::check(ParserScope& scope)
{
	for (auto const& [cond, block] : conditionals)
	{
		auto cond_type = cond->type_check(scope);

		if (cond_type.has_value() && is_object_t(*cond_type))
			night::error::get().create_minor_error(
				"expected type 'bool', 'char', or 'int',"
				"condition is type '" + night::to_str(*cond_type) + "'", loc);

		ParserScope block_scope{ scope.vars };
		for (auto& stmt : block)
			stmt->check(block_scope);
	}
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

		// insert offset after JUMP_IF_FALSE
		codes.insert(std::begin(codes) + offset_index, codes.size() - offset_index);

		codes.push_back((bytecode_t)BytecodeType::JUMP);
		jumps.push_back(codes.size());
	}

	// insert offsets after JUMP
	for (int jump : jumps)
		codes.insert(std::begin(codes) + jump, codes.size() - jump);

	return codes;
}


While::While(
	Location const& _loc,
	expr::expr_p const& _cond,
	AST_Block const& _block)
	: AST(_loc), cond_expr(_cond), block(_block) {}

void While::check(ParserScope& scope)
{
	auto cond_type = cond_expr->type_check(scope);

	if (cond_type.has_value() && is_object_t(*cond_type))
		night::error::get().create_minor_error(
			"condition is type '" + night::to_str(*cond_type) + "', "
			"expected type 'bool', 'char', 'int', or 'float'", loc);

	ParserScope block_scope{ scope.vars };
	for (auto& stmt : block)
		stmt->check(block_scope);
}

bytecodes_t While::generate_codes() const
{
	//
	// [0] [1] [2] [3] [4] [NJUMP] [x] 
	auto codes = cond_expr->generate_codes();

	codes.push_back((bytecode_t)BytecodeType::JUMP_IF_FALSE);
	auto offset_index = codes.size();

	for (auto const& stmt : block)
	{
		auto stmt_codes = stmt->generate_codes();
		codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));
	}

	codes.push_back((bytecode_t)BytecodeType::NJUMP);
	codes.push_back(codes.size() + 2);

	// insert offset after JUMP_IF_FALSE
	codes.insert(std::begin(codes) + offset_index, codes.size() - offset_index);

	return codes;
}


For::For(
	Location const& _loc,
	VariableInit const& _var_init,
	expr::expr_p const& _cond_expr,
	AST_Block const& _block)
	: AST(_loc), var_init(_var_init), loop(_loc, _cond_expr, _block) {}

void For::check(ParserScope& scope)
{
	ParserScope block_scope{ scope.vars };

	var_init.check(block_scope);
	loop.check(block_scope);
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
	std::vector<std::string> const& _param_names,
	std::vector<std::string> const& _param_types,
	std::string const& _rtn_type,
	AST_Block const& _block)
	: AST(_loc), name(_name), param_names(_param_names), rtn_type(token_var_type_to_val_type(_rtn_type)), block(_block)
{
	for (auto const& param_type : _param_types)
		param_types.push_back(token_var_type_to_val_type(param_type));

	// create a function so it's return type can be referenced by `FunctionCall::type_check()`
	auto [err_msg, _func_it] = ParserScope::create_function(name, param_names, param_types, ParserScope::curr_rtn_type);

	if (!err_msg.empty())
		night::error::get().create_minor_error(err_msg, loc);
}

void Function::check(ParserScope& scope)
{
	ParserScope block_scope{ scope.vars };
	ParserScope::curr_rtn_type = rtn_type;

	for (std::size_t i = 0; i < param_names.size(); ++i)
	{
		auto [err_msg, id] = block_scope.create_variable(param_names[i], param_types[i]);

		if (!err_msg.empty())
			night::error::get().create_minor_error(err_msg, loc);

		param_ids.push_back(id);
	}

	for (auto& stmt : block)
		stmt->check(block_scope);
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
	expr::expr_p _expr)
	: AST(_loc), expr(_expr) {}

void Return::check(ParserScope& scope)
{
	if (!expr)
	{
		if (ParserScope::curr_rtn_type.has_value())
			night::error::get().create_minor_error(
				"return statement does nto return a value, yet function expects one of type '" +
				night::to_str(*ParserScope::curr_rtn_type) + "'", loc);
	}
	else
	{
		if (!ParserScope::curr_rtn_type.has_value())
			night::error::get().create_minor_error(
				"return statement found, yet function does not return a value", loc);

		auto expr_type = expr->type_check(scope);

		if (expr_type.has_value() && !compare_value_t(*ParserScope::curr_rtn_type, *expr_type))
			night::error::get().create_minor_error(
				"return is type '" + night::to_str(*expr_type) + "', expected type '" +
				night::to_str(*ParserScope::curr_rtn_type) + "'", loc);
	}
}

bytecodes_t Return::generate_codes() const
{
	auto codes = expr->generate_codes();

	codes.push_back((bytecode_t)BytecodeType::RETURN);

	return codes;
}


FunctionCall::FunctionCall(
	Location const& _loc,
	std::string const& _name,
	std::vector<expr::expr_p> const& _arg_exprs)
	: AST(_loc), Expression(expr::ExpressionType::FUNCTION_CALL, _loc), name(_name), arg_exprs(_arg_exprs) {}

void FunctionCall::insert_node(
	expr::expr_p const& node,
	expr::expr_p* prev)
{
	node->insert_node(std::make_shared<FunctionCall>(AST::loc, name, arg_exprs));
	*prev = node;
}

std::optional<value_t> FunctionCall::type_check(ParserScope const& scope)
{
	std::vector<value_t> arg_types;
	for (auto& arg_expr : arg_exprs)
		arg_types.push_back(*arg_expr->type_check(scope));

	auto [func, range_end] = ParserScope::funcs.equal_range(name);

	if (func == range_end)
		night::error::get().create_minor_error("function '" + name + "' is not defined", Expression::loc);

	for (; func != range_end; ++func)
	{
		if (std::equal(std::begin(arg_types), std::end(arg_types),
			std::begin(func->second.param_types), std::end(func->second.param_types)))
			break;
	}

	if (func == range_end)
		night::error::get().create_minor_error(
			"arguments in function call '" + name +
			"' do not match with the parameters in its function definition", Expression::loc);

	id = func->second.id;
	return func->second.rtn_type;
}

void FunctionCall::check(ParserScope& scope)
{
	std::vector<value_t> arg_types;
	for (auto& arg_expr : arg_exprs)
		arg_types.push_back(*arg_expr->type_check(scope));

	auto [func, range_end] = ParserScope::funcs.equal_range(name);

	if (func == range_end)
		night::error::get().create_minor_error("function '" + name + "' is not defined", Expression::loc);

	for (; func != range_end; ++func)
	{
		if (std::equal(std::begin(arg_types), std::end(arg_types),
			std::begin(func->second.param_types), std::end(func->second.param_types)))
			break;
	}

	if (func == range_end)
		night::error::get().create_minor_error(
			"arguments in function call '" + name +
			"' do not match with the parameters in its function definition", Expression::loc);

	id = func->second.id;
}

bytecodes_t FunctionCall::generate_codes() const
{
	bytecodes_t codes;

	for (auto const& param : arg_exprs)
	{
		auto param_codes = param->generate_codes();
		codes.insert(std::end(codes), std::begin(param_codes), std::end(param_codes));
	}

	codes.push_back((bytecode_t)BytecodeType::CALL);
	codes.push_back(id);

	return codes;
}

int FunctionCall::precedence() const
{
	return single_prec;
}
