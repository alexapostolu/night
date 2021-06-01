#include "../../include/front-end/front-end.hpp"
#include "../../include/back-end/interpreter.hpp"
#include "../../include/back-end/parser.hpp"
#include "../../include/back-end/lexer.hpp"
#include "../../include/back-end/token.hpp"
#include "../../include/error.hpp"

#include <string>
#include <vector>

void FrontEnd(int argc, char** argv)
{
	if (argc != 2) {
		throw NIGHT_PREPROCESS_ERROR(
			"invalid command line arguments",
			night::learn_run);
	}

	if (std::string(argv[1]) == "--help")
	{
		std::cout << "--help    displays list of commands\n"
				  << "--version displays current version\n"
				  << "<file>    runs <file>\n";
		return;
	}
	if (std::string(argv[1]) == "--version")
	{
		std::cout << "night v0.0.0";
		return;
	}

	Lexer lexer(argv[1], true);
	Parser parser(lexer);

	Parser::Scope global_scope{ nullptr };

	std::vector<Stmt> stmts;
	while (lexer.peek(true).type != TokenType::_EOF)
		stmts.push_back(parser.parse_statement(global_scope));

	// Optimizer optimizer;

	Interpreter interpreter;

	for (Stmt const& stmt : stmts)
		interpreter.interpret_statement(stmt);
}