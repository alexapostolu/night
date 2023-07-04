#include "ast.hpp"
#include "bytecode.hpp"
#include "parser_scope.hpp"
#include "interpreter_scope.hpp"
#include "error.hpp"

#include <limits>

AST::AST(Location const& _loc)
	: loc(_loc) {}

VariableInit::VariableInit(
	Location const& _loc,
	std::string const& _name,
	std::shared_ptr<Expression> const& _expr)
	: AST(_loc), name(_name), expr(_expr) {}

bytecodes_t VariableInit::generate_codes(ParserScope const& scope) const
{
	bytecodes_t codes = expr->generate_codes(scope);

	codes.push_back(scope.vars.at(name).id);
	codes.push_back((bytecode_t)BytecodeType::ASSIGN);

	return codes;
}


VariableAssign::VariableAssign(
	Location const& _loc,
	std::string const& _name,
	std::shared_ptr<Expression> const& _expr,
	std::string const& _assign_op)
	: AST(_loc), name(_name), expr(_expr), assign_op(_assign_op) {}

bytecodes_t VariableAssign::generate_codes(ParserScope const& scope) const
{
	bytecodes_t codes = expr->generate_codes(scope);

	if (assign_op != "=")
	{
		codes.push_back(scope.vars.at(name).id);

		if      (assign_op == "+=")	codes.push_back((bytecode_t)BytecodeType::ADD);
		else if (assign_op == "-=") codes.push_back((bytecode_t)BytecodeType::SUB);
		else if (assign_op == "*=") codes.push_back((bytecode_t)BytecodeType::MULT);
		else if (assign_op == "/=") codes.push_back((bytecode_t)BytecodeType::DIV);
		else throw night::unhandled_case(assign_op);
	}

	codes.push_back(scope.vars.at(name).id);
	codes.push_back((bytecode_t)BytecodeType::ASSIGN);

	return codes;
}


If::If(Location const& _loc, std::shared_ptr<Expression> const& _cond_expr, AST_Block const& _block)
	: AST(_loc), cond_expr(_cond_expr), block(_block) {}

bytecodes_t If::generate_codes(ParserScope const& scope) const
{
	bytecodes_t codes;
	
	auto cond_codes = cond_expr->generate_codes(scope);
	codes.insert(std::end(codes), std::begin(cond_codes), std::end(cond_codes));

	codes.push_back((bytecode_t)BytecodeType::JUMP_IF_FALSE);
	codes.push_back((bytecode_t)block.size());

	for (auto stmt : block)
	{
		auto stmt_codes = stmt->generate_codes(scope);
		codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));
	}

	codes.push_back((bytecode_t)BytecodeType::END_IF);

	return codes;
}


While::While(
	Location const& _loc,
	std::shared_ptr<Expression> const& _cond,
	AST_Block const& _block)
	: AST(_loc), cond_expr(_cond), block(_block) {}

bytecodes_t While::generate_codes(ParserScope const& scope) const
{
	auto codes = cond_expr->generate_codes(scope);

	auto codes_s = codes.size() - 2;
	codes.push_back((bytecode_t)BytecodeType::JUMP_IF_FALSE);

	for (auto const& stmt : block)
	{
		auto stmt_codes = stmt->generate_codes(scope);
		codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));
	}

	bytecode_create_int(codes, -codes.size() - 1, codes_s);
	codes.push_back((bytecode_t)BytecodeType::JUMP);

	return codes;
}


For::For(
	Location const& _loc,
	VariableInit const& _var_init,
	std::shared_ptr<Expression> const& _cond_expr,
	VariableAssign const& _var_assign,
	AST_Block const& _block)
	: AST(_loc), var_init(_var_init), cond_expr(_cond_expr), var_assign(_var_assign), block(_block) {}

bytecodes_t For::generate_codes(ParserScope const& scope) const
{
	auto codes = var_init.generate_codes(scope);

	auto codes_cond = cond_expr->generate_codes(scope);
	codes.insert(std::end(codes), std::begin(codes_cond), std::begin(codes_cond));

	auto codes_s = codes.size() - 2;
	codes.push_back((bytecode_t)BytecodeType::JUMP_IF_FALSE);

	for (auto const& stmt : block)
	{
		auto stmt_codes = stmt->generate_codes(scope);
		codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));
	}

	auto codes_incr = var_assign.generate_codes(scope);
	codes.insert(std::end(codes), std::begin(codes_incr), std::end(codes_incr));

	bytecode_create_int(codes, -codes.size() - 1, codes_s);
	codes.push_back((bytecode_t)BytecodeType::JUMP);

	return codes;
}


Function::Function(
	Location const& _loc,
	std::string const& _func_name,
	std::vector<std::string> const& _param_names,
	AST_Block const& _block)
	: AST(_loc), func_name(_func_name), param_names(_param_names), block(_block) {}

bytecodes_t Function::generate_codes(ParserScope const& scope) const
{
	InterpreterScope::funcs[func_name] = {};

	for (auto const& param_name : param_names)
		InterpreterScope::funcs[func_name].params.push_back(scope.vars.at(param_name).id);

	for (auto const& stmt : block)
	{
		auto stmt_codes = stmt->generate_codes(scope);
		InterpreterScope::funcs[func_name].codes.insert(
			std::end(InterpreterScope::funcs[func_name].codes),
			std::begin(stmt_codes), std::end(stmt_codes));
	}

	return {};
}


Return::Return(
	Location const& _loc,
	std::shared_ptr<Expression> _expr)
	: AST(_loc), expr(_expr) {}

bytecodes_t Return::generate_codes(ParserScope const& scope) const
{
	auto codes = expr->generate_codes(scope);

	codes.push_back((bytecode_t)BytecodeType::RETURN);

	return codes;
}