#include "ast/ast.hpp"
#include "bytecode.hpp"
#include "interpreter_scope.hpp"
#include "error.hpp"

#include <limits>
#include <vector>
#include <assert.h>

AST::AST(Location const& _loc)
	: loc(_loc) {}

VariableInit::VariableInit(
	Location const& _loc,
	bytecode_t _id,
	std::shared_ptr<expr::Expression> const& _expr)
	: AST(_loc), id(_id), expr(_expr) {}

bytecodes_t VariableInit::generate_codes() const
{
	bytecodes_t codes = expr->generate_codes();

	codes.push_back((bytecode_t)BytecodeType::STORE);
	codes.push_back(id);

	return codes;
}


VariableAssign::VariableAssign(
	Location const& _loc,
	bytecode_t _id,
	std::shared_ptr<expr::Expression> const& _expr,
	std::string const& _assign_op)
	: AST(_loc), id(_id), expr(_expr), assign_op(_assign_op) {}

bytecodes_t VariableAssign::generate_codes() const
{
	bytecodes_t codes = expr->generate_codes();

	if (assign_op != "=")
	{
		codes.push_back((bytecode_t)BytecodeType::LOAD);
		codes.push_back(id);

		if      (assign_op == "+=")	codes.push_back((bytecode_t)BytecodeType::ADD);
		else if (assign_op == "-=") codes.push_back((bytecode_t)BytecodeType::SUB);
		else if (assign_op == "*=") codes.push_back((bytecode_t)BytecodeType::MULT);
		else if (assign_op == "/=") codes.push_back((bytecode_t)BytecodeType::DIV);
		else night::throw_unhandled_case(assign_op);
	}

	codes.push_back((bytecode_t)BytecodeType::STORE);
	codes.push_back(id);

	return codes;
}


Conditional::Conditional(
	Location const& _loc,
	std::vector<
	std::pair<std::shared_ptr<expr::Expression>, AST_Block>
	> const& _conditionals)
	: AST(_loc), conditionals(_conditionals) {}

bytecodes_t Conditional::generate_codes() const
{
	bytecodes_t codes;
	std::vector<int> jumps;

	for (std::size_t i = 0; i < conditionals.size(); ++i)
	{
		auto cond_codes = conditionals[i].first->generate_codes();
		codes.insert(std::end(codes), std::begin(cond_codes), std::end(cond_codes));

		codes.push_back((bytecode_t)BytecodeType::JUMP_IF_FALSE);
		int offset_index = codes.size();

		int stmt_size = 0;
		for (auto const& stmt : conditionals[i].second)
		{
			auto stmt_codes = stmt->generate_codes();
			codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));

			stmt_size += stmt_codes.size();
		}

		// insert offset after JUMP_IF_FALSE
		codes.insert(std::begin(codes) + offset_index, stmt_size + 2);

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
	std::shared_ptr<expr::Expression> const& _cond,
	AST_Block const& _block)
	: AST(_loc), cond_expr(_cond), block(_block) {}

bytecodes_t While::generate_codes() const
{
	auto codes = cond_expr->generate_codes();

	auto codes_s = codes.size() - 2;
	codes.push_back((bytecode_t)BytecodeType::JUMP_IF_FALSE);

	for (auto const& stmt : block)
	{
		auto stmt_codes = stmt->generate_codes();
		codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));
	}

	bytecode_create_int(codes, -codes.size() - 1, codes_s);
	codes.push_back((bytecode_t)BytecodeType::JUMP);

	return codes;
}


For::For(
	Location const& _loc,
	VariableInit const& _var_init,
	std::shared_ptr<expr::Expression> const& _cond_expr,
	VariableAssign const& _var_assign,
	AST_Block const& _block)
	: AST(_loc), var_init(_var_init), cond_expr(_cond_expr), var_assign(_var_assign), block(_block) {}

bytecodes_t For::generate_codes() const
{
	auto codes = var_init.generate_codes();

	auto codes_cond = cond_expr->generate_codes();
	codes.insert(std::end(codes), std::begin(codes_cond), std::begin(codes_cond));

	auto codes_s = codes.size() - 2;
	codes.push_back((bytecode_t)BytecodeType::JUMP_IF_FALSE);

	for (auto const& stmt : block)
	{
		auto stmt_codes = stmt->generate_codes();
		codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));
	}

	auto codes_incr = var_assign.generate_codes();
	codes.insert(std::end(codes), std::begin(codes_incr), std::end(codes_incr));

	bytecode_create_int(codes, -codes.size() - 1, codes_s);
	codes.push_back((bytecode_t)BytecodeType::JUMP);

	return codes;
}


Function::Function(
	Location const& _loc,
	std::string const& _func_name,
	std::vector<bytecode_t> const& _param_ids,
	AST_Block const& _block)
	: AST(_loc), func_name(_func_name), param_ids(_param_ids), block(_block) {}

bytecodes_t Function::generate_codes() const
{
	InterpreterScope::funcs[func_name] = {};

	for (auto const& param_id : param_ids)
		InterpreterScope::funcs[func_name].params.push_back(param_id);

	for (auto const& stmt : block)
	{
		auto stmt_codes = stmt->generate_codes();
		InterpreterScope::funcs[func_name].codes.insert(
			std::end(InterpreterScope::funcs[func_name].codes),
			std::begin(stmt_codes), std::end(stmt_codes));
	}

	return {};
}


Return::Return(
	Location const& _loc,
	std::shared_ptr<expr::Expression> _expr)
	: AST(_loc), expr(_expr) {}

bytecodes_t Return::generate_codes() const
{
	auto codes = expr->generate_codes();

	codes.push_back((bytecode_t)BytecodeType::RETURN);

	return codes;
}


FunctionCall::FunctionCall(
	Location const& _loc,
	std::string const& _func_name,
	std::vector<std::shared_ptr<expr::Expression>> const& _params,
	std::vector<bytecode_t> const& param_ids)
	: AST(_loc), func_name(_func_name)
{
	assert(param_ids.size() == _params.size());

	for (std::size_t i = 0; i < _params.size(); ++i)
		params.push_back(VariableInit(_loc, param_ids[i], _params[i]));
}

bytecodes_t FunctionCall::generate_codes() const
{
	bytecodes_t codes;
	codes.push_back(params->);
}