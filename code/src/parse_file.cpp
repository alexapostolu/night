#include "parse_file.hpp"
#include "interpreter.hpp"
#include "parser.hpp"
#include "lexer.hpp"
#include "token.hpp"

#include <string>
#include <vector>

void parse_file(std::string_view file_name)
{
	Lexer lexer(file_name, true);
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
