#pragma once

#include <iostream>
#include <vector>

#include "Token.h"
#include "Variable.h"

bool CheckStr(std::vector<Token>& expr, std::vector<Variable>& vars)
{
	// turn variables into their values
	for (std::size_t a = 3; a < expr.size() - 1; ++a)
	{
		if (expr[a].type == TokenType::VARIABLE)
		{
			bool definedVariable = false;
			for (std::size_t b = 0; b < vars.size(); ++b)
			{
				if (expr[a].token == vars[b].name && vars[b].type == "str")
				{
					expr[a].type = TokenType::STRING_VALUE;
					expr[a].token = vars[b].value;

					definedVariable = true;
					break;
				}
			}

			if (!definedVariable)
			{
				std::cout << "Error - undefined variable '" << expr[a].token << "'\n";
				exit(0);
			}
		}
	}

	/* valid string expressions:

	 = str +          str + str            = ( str          str ) +
	 = str ;          str + (              = ( (            str ) )
	 + str )          )   + str            + ( str          str ) ;
	 + str +          )   + (              + ( (            )   ) +
	 + str ;                               ( ( str          )   ) )
	 ( str +                               ( ( (            )   ) ;
	 ( str )

	 */

	for (std::size_t a = 3; a < expr.size() - 1; ++a)
	{
		// checks value
		if (expr[a].type == TokenType::STRING_VALUE)
		{
			if ((expr[a - 1].type != TokenType::ASSIGNMENT ||
					expr[a + 1].type != TokenType::PLUS) &&
				(expr[a - 1].type != TokenType::ASSIGNMENT ||
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
				return false;
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
				return false;
			}
		}
		// check open bracket
		else if (expr[a].type == TokenType::OPEN_BRACKET)
		{
			if ((expr[a - 1].type != TokenType::ASSIGNMENT ||
				expr[a + 1].type != TokenType::STRING_VALUE) &&
				(expr[a - 1].type != TokenType::ASSIGNMENT ||
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
				return false;
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
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	return true;
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
