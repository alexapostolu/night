#include "ast/ast.hpp"
#include "bytecode.hpp"
#include "interpreter_scope.hpp"
#include "parser_scope.hpp"
#include "utils.hpp"
#include "error.hpp"
#include "debug.hpp"

#include <limits>
#include <vector>
#include <memory>
#include <assert.h>
#include <ranges>

AST::AST(Location const& _loc)
	: loc(_loc) {}

VariableInit::VariableInit(
	Location const& _loc,
	std::string const& _name,
	ValueType const& _type,
	std::vector<std::optional<expr::expr_p>> const& _arr_sizes,
	expr::expr_p const& _expr)
	: AST(_loc), name(_name), type(_type), arr_sizes(_arr_sizes), expr(_expr), id(std::nullopt)
{
	type.dim = arr_sizes.size();
}

void VariableInit::check(ParserScope& scope)
{
	assert(expr);

	auto _id = scope.create_variable(name, type, loc);
	if (_id.has_value())
		id = _id;

	// check expression

	expr_type = expr->type_check(scope);

	if (expr_type.has_value() && !compare_relative_vt(type, *expr_type))
		night::error::get().create_minor_error(
			"variable '" + name + "' of type '" + night::to_str(type) +
			"' can not be initialized with expression of type '" + night::to_str(*expr_type) + "'", loc);
}

bytecodes_t VariableInit::generate_codes() const
{
	assert(expr);
	assert(id.has_value());

	if (!arr_sizes.empty() && *arr_sizes[0])
	{
		bytecodes_t codes;
		
		if (type.type == ValueType::INT)
		{
			codes.push_back((bytecode_t)BytecodeType::S_INT1);
			codes.push_back(0);
		}
		else if (type.type == ValueType::FLOAT)
		{
			codes.push_back((bytecode_t)BytecodeType::FLOAT4);
			codes.push_back(0);
		}
		else
		{
			throw debug::unhandled_case(type.type);
		}

		for (int i = arr_sizes.size() - 1; i >= 0; --i)
		{
			auto size_codes = (*arr_sizes[i])->generate_codes();
			codes.insert(std::end(codes), std::begin(size_codes), std::end(size_codes));

			codes.push_back((bytecode_t)BytecodeType::ALLOCATE);
		}

		codes.push_back((bytecode_t)BytecodeType::STORE);
		codes.push_back(*id);

		return codes;
	}
	else
	{
		bytecodes_t codes = expr->generate_codes();

		if (expr_type == ValueType::FLOAT && type == ValueType::INT)
			codes.push_back((bytecode_t)BytecodeType::F2I);

		codes.push_back((bytecode_t)BytecodeType::STORE);
		codes.push_back(*id);
		
		return codes;
	}
}


VariableAssign::VariableAssign(
	Location const& _loc,
	std::string const& _var_name,
	std::string const& _assign_op,
	expr::expr_p const& _expr)
	: AST(_loc), var_name(_var_name), assign_op(_assign_op), expr(_expr), assign_type(std::nullopt), id(std::nullopt) {}

void VariableAssign::check(ParserScope& scope)
{
	assert(expr);

	if (!scope.vars.contains(var_name))
		night::error::get().create_minor_error("variable '" + var_name + "' is undefined", loc);

	auto expr_type = expr->type_check(scope);

	if (!expr_type.has_value() || night::error::get().has_minor_errors())
		return;

	id = scope.vars[var_name].id;

	if (!compare_relative_vt(scope.vars[var_name].type, *expr_type))
		night::error::get().create_minor_error(
			"variable '" + var_name + "' of type '" + night::to_str(scope.vars[var_name].type) +
			"can not be assigned to type '" + night::to_str(*expr_type) + "'", loc);

	assign_type = *expr_type;

}

bytecodes_t VariableAssign::generate_codes() const
{
	bytecodes_t codes;// = expr->generate_codes();

	if (assign_op != "=")
	{
		codes.push_back((bytecode_t)BytecodeType::LOAD);
		codes.push_back(*id);

		auto expr_codes = expr->generate_codes();
		codes.insert(std::end(codes), std::begin(expr_codes), std::end(expr_codes));

		if (assign_op == "+=")
		{
			if (assign_type == ValueType::FLOAT)
				codes.push_back((bytecode_t)BytecodeType::ADD_F);
			else if (assign_type == ValueType::STR)
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
	std::vector<std::pair<expr::expr_p, AST_Block>
	> const& _conditionals)
	: AST(_loc), conditionals(_conditionals) {}

void Conditional::check(ParserScope& scope)
{
	for (auto const& [cond, block] : conditionals)
	{
		auto cond_type = cond->type_check(scope);

		if (cond_type.has_value() && !cond_type->is_prim())
			night::error::get().create_minor_error(
				"expected type 'bool', 'char', or 'int',"
				"condition is type '" + night::to_str(*cond_type) + "'", loc);

		ParserScope conditional_scope(scope);
		for (auto& stmt : block)
			stmt->check(conditional_scope);
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

		// insert offset before JUMP_IF_FALSE
		auto jif_codes = expr::Value::int_to_bytecodes(codes.size() - offset_index + 2);
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
	AST_Block const& _block)
	: AST(_loc), cond_expr(_cond), block(_block) {}

void While::check(ParserScope& scope)
{
	auto cond_type = cond_expr->type_check(scope);

	if (cond_type.has_value() && !cond_type->is_prim())
		night::error::get().create_minor_error(
			"condition is type '" + night::to_str(*cond_type) + "', "
			"expected type 'bool', 'char', 'int', or 'float'", loc);

	ParserScope while_scope(scope);

	for (auto& stmt : block)
		stmt->check(while_scope);
}

bytecodes_t While::generate_codes() const
{
	auto codes = cond_expr->generate_codes();

	codes.push_back((bytecode_t)BytecodeType::JUMP_IF_FALSE);
	auto offset_index = codes.size();

	for (auto const& stmt : block)
	{
		auto stmt_codes = stmt->generate_codes();
		codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));
	}

	auto jif_codes = expr::Value::int_to_bytecodes(codes.size() - offset_index + 2);

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
	AST_Block const& _block)
	: AST(_loc), var_init(_var_init), loop(_loc, _cond_expr, _block) {}

void For::check(ParserScope& scope)
{
	ParserScope for_scope(scope);

	var_init.check(for_scope);
	loop.check(for_scope);
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
	: AST(_loc), name(_name), param_names(_param_names), block(_block)
{
	if (_rtn_type == "void")
		rtn_type = std::nullopt;
	else
		rtn_type = token_var_type_to_val_type(_rtn_type);

	for (auto const& param_type : _param_types)
		param_types.push_back(token_var_type_to_val_type(param_type));
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
	: AST(_loc), expr(_expr) {}

void Return::check(ParserScope& scope)
{
	auto expr_type = expr->type_check(scope);

	try {
		scope.check_return_type(expr_type);
	}
	catch (std::string const& e) {
		night::error::get().create_minor_error(e, loc);
	}
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
	std::vector<expr::expr_p> const& _subscripts,
	expr::expr_p const& _assign_expr)
	: AST(_loc), var_name(_var_name), subscripts(_subscripts), assign_expr(_assign_expr), id(std::nullopt) {}

void ArrayMethod::check(ParserScope& scope)
{
	if (scope.vars[var_name].type.dim != subscripts.size())
		night::error::get().create_minor_error("too many subscripts for variable of dimension '" + std::to_string(scope.vars[var_name].type.dim) + "'", loc);

	for (auto const& subscript : subscripts)
	{
		assert(subscript);

		auto subscript_type = subscript->type_check(scope);
		if (subscript_type.has_value() && !subscript_type->is_prim())
			night::error::get().create_minor_error("subscript is type '" + night::to_str(*subscript_type) + "', expected type bool, char, or int'", loc);
	}

	if (assign_expr)
	{
		auto const& assign_type = assign_expr->type_check(scope);

		if (assign_type.has_value() && !compare_relative_vt(ValueType(scope.vars[var_name].type.type, scope.vars[var_name].type.dim - subscripts.size()), *assign_type))
			night::error::get().create_minor_error("array of type '" + night::to_str(scope.vars[var_name].type) + "' can not be assigned to expression of type '" +
				night::to_str(assign_type->type) + "'", loc);
	}

	id = scope.vars[var_name].id;
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

	codes.push_back((bytecode_t)BytecodeType::SET_INDEX);
	codes.push_back(*id);

	return codes;
}


expr::FunctionCall::FunctionCall(
	Location const& _loc,
	std::string const& _name,
	std::vector<expr::expr_p> const& _arg_exprs)
	: AST(_loc), Expression(expr::ExpressionType::FUNCTION_CALL, _loc), name(_name), arg_exprs(_arg_exprs), id(std::nullopt), is_expr(true) {}

void expr::FunctionCall::insert_node(
	expr::expr_p const& node,
	expr::expr_p* prev)
{
	node->insert_node(std::make_shared<FunctionCall>(AST::loc, name, arg_exprs));
	*prev = node;
}

void expr::FunctionCall::check(ParserScope& scope)
{
	is_expr = false;
	type_check(scope);
}

std::optional<ValueType> expr::FunctionCall::type_check(ParserScope const& scope)
{
	// check argument types

	std::vector<ValueType> arg_types;
	for (auto& arg_expr : arg_exprs)
	{
		assert(arg_expr);

		auto arg_type = arg_expr->type_check(scope);

		if (arg_type.has_value())
			arg_types.push_back(*arg_type);
	}

	if (arg_types.size() != arg_exprs.size())
		return std::nullopt;

	// match function with ParserScope function based on name and argument types

	auto [func, range_end] = ParserScope::funcs.equal_range(name);

	if (func == range_end)
		night::error::get().create_minor_error("function '" + name + "' is undefined", AST::loc);

	for (; func != range_end; ++func)
	{
		if (std::equal(std::begin(arg_types), std::end(arg_types),
					   std::begin(func->second.param_types), std::end(func->second.param_types),
			compare_absolute_vt))
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
			"', and do not match with the parameters in its function definition", AST::loc);
	}

	if (is_expr && !func->second.rtn_type.has_value())
		night::error::get().create_minor_error("function '" + func->first + "' can not have a return type of void when used in an expression", AST::loc);

	id = func->second.id;
	return func->second.rtn_type;
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
