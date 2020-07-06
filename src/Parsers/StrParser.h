#pragma once

#include <vector>

#include "Variable.h"
#include "Error.h"
#include "Token.h"

void CheckStr(int check, std::vector<Token>& expr, std::vector<Variable>& vars, TokenType ss = TokenType::ASSIGNMENT)
{
	/* valid string expressions:
	 = str +          str + str            = ( str          str ) +
	 = str ;          str + (              = ( (            str ) )
	 + str )          )   + str            + ( str          str ) ;
	 + str +          )   + (              + ( (            )   ) +
	 + str ;                               ( ( str          )   ) )
	 ( str +                               ( ( (            )   ) ;
	 ( str )
	 */

	int openBracket = 0;
	for (std::size_t a = check; a < expr.size() - 1; ++a)
	{
		if (expr[a].type == TokenType::OPEN_BRACKET)
			openBracket += 1;
		else if (expr[a].type == TokenType::CLOSE_BRACKET)
			openBracket -= 1;

		// checks value
		if (expr[a].type == TokenType::STRING_VALUE)
		{
			if ((expr[a - 1].type != ss ||
					expr[a + 1].type != TokenType::PLUS) &&
				(expr[a - 1].type != ss ||
					expr[a + 1].type != TokenType::SEMICOLON)
				&&
				(expr[a - 1].type != TokenType::PLUS ||
					expr[a + 1].type != TokenType::CLOSE_BRACKET) &&
				(expr[a - 1].type != TokenType::PLUS ||
					expr[a + 1].type != TokenType::PLUS) &&
				(expr[a - 1].type != TokenType::PLUS ||
					expr[a + 1].type != TokenType::SEMICOLON)
				&&
				(expr[a - 1].type != TokenType::OPEN_BRACKET ||
					expr[a + 1].type != TokenType::PLUS) &&
				(expr[a - 1].type != TokenType::OPEN_BRACKET ||
					expr[a + 1].type != TokenType::CLOSE_BRACKET))
			{
				error("invalid string expression");
			}
		}
		// checks PLUS operator
		else if (expr[a].type == TokenType::PLUS)
		{
			if ((expr[a - 1].type != TokenType::STRING_VALUE ||
					expr[a + 1].type != TokenType::STRING_VALUE) &&
				(expr[a - 1].type != TokenType::STRING_VALUE ||
					expr[a + 1].type != TokenType::OPEN_BRACKET)
				&&
				(expr[a - 1].type != TokenType::CLOSE_BRACKET ||
					expr[a + 1].type != TokenType::STRING_VALUE) &&
				(expr[a - 1].type != TokenType::CLOSE_BRACKET ||
					expr[a + 1].type != TokenType::OPEN_BRACKET))
			{
				error("invalid string expression");
			}
		}
		// check open bracket
		else if (expr[a].type == TokenType::OPEN_BRACKET)
		{
			if ((expr[a - 1].type != ss ||
					expr[a + 1].type != TokenType::STRING_VALUE) &&
				(expr[a - 1].type != ss ||
					expr[a + 1].type != TokenType::OPEN_BRACKET)
				&&
				(expr[a].type != TokenType::PLUS ||
					expr[a + 1].type != TokenType::STRING_VALUE) &&
				(expr[a].type != TokenType::PLUS ||
					expr[a + 1].type != TokenType::OPEN_BRACKET)
				&&
				(expr[a - 1].type != TokenType::OPEN_BRACKET ||
					expr[a + 1].type != TokenType::STRING_VALUE) &&
				(expr[a - 1].type != TokenType::OPEN_BRACKET ||
					expr[a + 1].type != TokenType::OPEN_BRACKET))
			{
				error("invalid string expression");
			}
		}
		// checks close bracket
		else if (expr[a].type == TokenType::CLOSE_BRACKET)
		{
			if ((expr[a - 1].type != TokenType::STRING_VALUE ||
					expr[a + 1].type != TokenType::PLUS) &&
				(expr[a - 1].type != TokenType::STRING_VALUE ||
					expr[a + 1].type != TokenType::CLOSE_BRACKET) &&
				(expr[a - 1].type != TokenType::STRING_VALUE ||
					expr[a + 1].type != TokenType::SEMICOLON)
				&&
				(expr[a - 1].type != TokenType::CLOSE_BRACKET ||
					expr[a + 1].type != TokenType::PLUS) &&
				(expr[a - 1].type != TokenType::CLOSE_BRACKET ||
					expr[a + 1].type != TokenType::CLOSE_BRACKET) &&
				(expr[a - 1].type != TokenType::CLOSE_BRACKET ||
					expr[a + 1].type != TokenType::SEMICOLON))
			{
				error("invalid string expression");
			}
		}
		else
		{
			error("invalid string expression");
		}
	}

	if (openBracket > 0)
		error("closing bracket not found");
	else if (openBracket < 0)
		error("opening bracket not found");
}

void EvaluateStrExpression(std::vector<std::string>& expr, std::size_t& index)
{
	expr[index - 1] = expr[index - 1] + expr[index + 1];

	expr.erase(expr.begin() + index);
	expr.erase(expr.begin() + index);

	index -= 1;
}

std::string StrParser(const std::vector<Token>& tokens)
{
	std::vector<std::string> expr(tokens.size());
	for (std::size_t a = 0; a < tokens.size(); ++a)
		expr[a] = tokens[a].token;

	int openBracketIndex = 0, closeBrakcetIndex;
	for (std::size_t a = 0; a < expr.size(); ++a)
	{
		if (expr[a] == "(")
		{
			openBracketIndex = a;
		}
		else if (expr[a] == ")")
		{
			closeBrakcetIndex = a;

			for (std::size_t b = openBracketIndex + 1; b < closeBrakcetIndex; ++b)
			{
				if (expr[b] == "+")
				{
					EvaluateStrExpression(expr, b);
					closeBrakcetIndex -= 2;
				}
			}

			expr.erase(expr.begin() + openBracketIndex);
			expr.erase(expr.begin() + closeBrakcetIndex - 1);

			a = -1;
		}
	}

	for (std::size_t a = 0; a < expr.size(); ++a)
	{
		if (expr[a] == "+")
			EvaluateStrExpression(expr, a);
	}

	return expr[0];
}
