#include "ast.hpp"
#include "bytecode.hpp"
#include "parser_scope.hpp"
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


While::While(Location const& _loc, std::shared_ptr<Expression> const& _cond, AST_Block const& _block)
	: AST(_loc), cond_expr(_cond), block(_block) {}

bytecodes_t While::generate_codes(ParserScope const& scope) const
{
	bytecodes_t codes;

	auto cond_codes = cond_expr->generate_codes(scope);
	codes.insert(std::end(codes), std::begin(cond_codes), std::end(cond_codes));

	// FIX THIS TO INCLUDE BYTECODETYPE::JUMP_IF_NOT
	// JUMP
	// SET???? auto set = codes_set(codes);
	//         ...
	//         codes.push_back(set);
	//         codes.push_back((bytecode_t)BytecodeType::JUMP);



	codes.push_back((bytecode_t)BytecodeType::WHILE);
	codes.push_back((bytecode_t)block.size());

	for (auto stmt : block)
	{
		auto stmt_codes = stmt->generate_codes(scope);
		codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));
	}

	codes.

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
	bytecodes_t codes = var_init.generate_codes(scope);

	int codes_s = codes.size();

	auto cond_codes = cond_expr->generate_codes(scope);
	codes.insert(std::end(codes), std::begin(cond_codes), std::end(cond_codes));

	codes.push_back((bytecode_t)BytecodeType::JUMP_IF_FALSE);
	codes.push_back((bytecode_t)block.size());

	for (auto stmt : block)
	{
		auto stmt_codes = stmt->generate_codes(scope);
		codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));
	}

	auto var_assign_codes = var_assign.generate_codes(scope);
	codes.insert(std::end(codes), std::begin(var_assign_codes), std::end(var_assign_codes));

	codes.push_back((bytecode_t)(-codes.size() + codes.size()));
	codes.push_back((bytecode_t)BytecodeType::JUMP);

	return codes;
}
