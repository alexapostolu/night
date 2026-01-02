#pragma once

#include "lexer/lexer.hpp"
#include "parser/ast/statement.hpp"
#include "parser/ast/expression.hpp"
#include "common/token.hpp"
#include "common/util.hpp"

#include <vector>

enum {
	SyntaxOptional = 1
};

struct Syntax
{
	bool is_optional;

	Token t;
	expr::expr_p expr;
	std::vector<Parameter> parameters;
	std::vector<stmt_p> body;
	std::tuple<std::string, int> type;
	VariableInit var_init;

	Syntax(TokenType type, bool is_optional = false);
};

/**
 * @brief Tries to match the lexing with the given expected syntax.
 *
 * @param lexer		Starts at the first token of the syntax.
 * @param syntax	The expected syntax. The data of the syntax will be filled.
 *
 * @throws	Parsing error if no syntax matches.
 */
void parse_syntax(
	Lexer& lexer,
	std::vector<Syntax>& syntax
);
