#include "front-end/front-end.hpp"
#include "back-end/interpreter.hpp"
#include "back-end/parser.hpp"
#include "back-end/lexer.hpp"
#include "back-end/token.hpp"
#include "error.hpp"
#include "cmakedef.hpp"

#include <iostream>
#include <string>
#include <cstring>
#include <vector>

namespace night
{
	void printHelp()
	{
		std::cout << "Usage: night <file>|<options>\n"
		<< "Options:\n"
		<< "--help     Displays this message and exit\n"
		<< "--version  Displays night's current version\n"
		<< std::endl;
	}
}

void FrontEnd(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cout << "Invalid number of arguments.\n" << std::endl;
		night::printHelp();
		return;
	}

	if (argv[1][0] == '-')
	{
		if (!(std::strcmp(argv[1], "--help") && std::strcmp(argv[1], "-h")))
		{
			night::printHelp();
			return;
		}
		if (!(std::strcmp(argv[1], "--version") && std::strcmp(argv[1], "-v")))
		{
			std::cout << "night v" << night_VERSION_MAJOR
			<< '.'  << night_VERSION_MINOR
			<< '.'  << night_VERSION_PATCH 
			<< '\n' << std::endl;
			return;
		}
		std::cout << "Unknown option: " << argv[1] << '\n' << std::endl;
		night::printHelp();
		return;
	}

	Lexer lexer(argv[1], true);
	Parser parser(lexer);

	Parser::ParserScope global_scope{ nullptr };

	std::vector<Stmt> stmts;

	auto token = lexer.eat(true);
	while (lexer.get_curr().type != TokenType::_EOF)
	{
		stmts.push_back(parser.parse_statement(global_scope));

		if (lexer.get_curr().type == TokenType::EOL)
			lexer.eat(true);
	}

	Interpreter::InterpreterScope interpret_scope{ nullptr };

	Interpreter interpreter;
	interpreter.interpret_statements(interpret_scope, stmts);
}
