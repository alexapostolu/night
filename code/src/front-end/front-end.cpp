#include "../../include/front-end/front-end.hpp"
#include "../../include/back-end/interpreter.hpp"
#include "../../include/back-end/parser.hpp"
#include "../../include/back-end/lexer.hpp"
#include "../../include/back-end/token.hpp"
#include "../../include/error.hpp"

#include <iostream>
#include <string>
#include <vector>

void FrontEnd(int argc, char** argv)
{
	if (argc != 2)
		throw NIGHT_PREPROCESS_ERROR("invalid command line arguments");

	if (std::string(argv[1]) == "--help")
	{
		std::cout << "--version\t\tdisplays current version\n"
				  << "<file>   \t\truns <file>\n";
		return;
	}
	if (std::string(argv[1]) == "--version")
	{
		std::cout << "night v0.0.0";
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