#include "parser.hpp"
#include "lexer.hpp"
#include "scope.hpp"
#include "bytecode.hpp"

#include <iostream>

bytecodes_t parse_stmts(Lexer& lexer, Scope& scope)
{
	// testing!!
	Token tok;
	do {
		tok = lexer.eat();
		std::cout << (int)tok.type << ": " << tok.val << " :\n";
	} while (tok.type != TokenType::END_OF_FILE);

	bytecodes_t bytecodes;

	while (true)
	{
		bytecodes_t bytecode;
		auto tok = lexer.eat();

		switch (tok.type)
		{
		case TokenType::VARIABLE:
			bytecode = parse_stmt_var(lexer, scope);
			break;

		case TokenType::IF:
			bytecode = parse_stmt_if(lexer, scope);
			break;

		case TokenType::ELIF:
		case TokenType::ELSE:
			throw NIGHT_COMPILE_ERROR(
				lexer.get_curr().data + " statement does not precede an if or elif statement",
				lexer.get_curr().data + " statements must come after an if or an elif statement");

		case TokenType::FOR:
			bytecode = parse_stmt_for(lexer, scope);
			break;

		case TokenType::WHILE:
			bytecode = parse_stmt_while(lexer, scope);
			break;

		case TokenType::RETURN:
			bytecode = parse_stmt_rtn(lexer, scope);
			break;

		case TokenType::END_OF_FILE:
			return bytecodes;

		default:
			throw NIGHT_COMPILE_ERROR(
				"unknown syntax",
				"no clue what you did here sorry :/");
		}

		bytecodes.insert(std::end(bytecodes), std::begin(bytecode), std::begin(bytecode));
	}
}

bytecodes_t parse_stmt_var(Lexer& lexer, Scope& scope)
{
	bytecodes_t codes;
	
	codes.push_back(std::make_shared<Bytecode>(BytecodeType::VARIABLE, lexer.curr().val));

	if (lexer.eat().type == TokenType::END_OF_FILE) {
		throw Error;
	}
	if (lexer.curr().type != TokenType::BOOL_TYPE) {
		throw Error;
	}

	codes.push_back(std::make_shared<Bytecode>(BytecodeType::VALUE));

	if (lexer.curr().type == TokenType::ASSIGN)
	{

	}
	
	if (lexer.eat().type != TokenType::SEMICOLON) {
		throw Error;
	}

	codes.push_back(std::make_shared<Bytecode>(BytecodeType::VALUE));
	
	return codes;
}

bytecodes_t parse_stmt_if(Lexer& lexer, Scope& scope)
{

}