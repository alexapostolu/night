#include "parser/parser.hpp"
#include "parser/expression_parser.hpp"
#include "parser/statement_parser.hpp"
#include "lexer/lexer.hpp"
#include "common/token.hpp"
#include "parser/ast/statement.hpp"

#include <unordered_set>

Syntax::Syntax(TokenType type, bool _is_optional)
	: t(type), is_optional(_is_optional) {}

/*
 * Parses a function's parameters.
 *
 * Lexer starts at open bracket and ends at closing bracket.
 */
static std::vector<Parameter> parse_function_parameters(Lexer& lexer)
{
	if (lexer.curr().type == TokenType::CLOSE_BRACKET)
		return {};

	std::vector<Parameter> parameters;

	while (true)
	{
		std::string name = lexer.curr_is(TokenType::VARIABLE).str;
		Location name_loc = lexer.curr().loc;

		std::string type_s = lexer.expect(TokenType::TYPE).str;

		// Check if the type is an array.
		int dimensions = 0;
		while (lexer.peek().type == TokenType::OPEN_SQUARE)
		{
			// Parse array size?

			dimensions++;

			lexer.eat();
			lexer.expect(TokenType::CLOSE_SQUARE);
		}

		parameters.emplace_back(name, Type(type_s, dimensions), name_loc);

		if (lexer.peek().type == TokenType::CLOSE_BRACKET)
		{
			lexer.eat();
			break;
		}

		lexer.expect(TokenType::COMMA);
		lexer.eat();
	}

	return parameters;
}

static bool require_curly = false;

static std::vector<stmt_p> parse_body_s(Lexer& lexer)
{
	bool contains_return = false;
	if (require_curly)
	{
		require_curly = false;
		return parse_stmts(lexer, true, &contains_return);
	}
	else
	{
		return parse_stmts(lexer, false, &contains_return);
	}
}

void parse_syntax(Lexer& lexer, std::vector<Syntax>& syntax)
{
	for (int i = 0; i < syntax.size(); ++i)
	{
		if (lexer.curr().type == TokenType::DEF)
			require_curly = true;

		if (syntax[i].t.type == TokenType::EXPR)
		{
			syntax[i].expr = parse_expr_s(lexer, false);	
		}
		else if (syntax[i].t.type == TokenType::PARAMETERS)
		{
			syntax[i].parameters = parse_function_parameters(lexer);
		}
		else if (syntax[i].t.type == TokenType::BODY)
		{
			syntax[i].body = parse_body_s(lexer);
		}
		else if (syntax[i].t.type == TokenType::TYPE)
		{
			syntax[i].type = parse_type(lexer);
		}
		else if (syntax[i].t.type == TokenType::VARIABLE_INIT)
		{
			Token variable_initialization = lexer.curr_is(TokenType::VARIABLE);
			lexer.expect(TokenType::TYPE);

			syntax[i].var_init = parse_variable_initialization(lexer, variable_initialization);
		}
		else
		{
			if (lexer.curr().type != syntax[i].t.type)
			{
				if (syntax[i].is_optional)
				{
					lexer.eat();
					continue;
				}

				throw night::error::get().create_fatal_error(
					"found return statement, expected no return statement in void function", lexer.loc);
			}
			
			syntax[i].t.str = lexer.curr().str;
			syntax[i].t.loc = lexer.curr().loc;

			lexer.eat();
		}
	}
}
