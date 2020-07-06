#pragma once

#include <vector>
#include <string>

#include "Variable.h"
#include "Error.h"
#include "Token.h"

void CheckInt(int start, std::vector<Token>& expr, const std::vector<Variable>& vars, TokenType ss = TokenType::ASSIGNMENT)
{
	/* valid integer expressions:
	 = 5 +          5 + 5          = ( 5          5 ) +
	 = 5 ;          5 + (          = ( (          5 ) )
	 + 5 )          ) + 5          + ( 5          5 ) ;
	 + 5 +          ) + (          + ( (          ) ) +
	 + 5 ;                         ( ( 5          ) ) )
	 ( 5 +                         ( ( (          ) ) ;
	 ( 5 )
	 */

	int openBracket = 0;
	for (std::size_t a = start; a < expr.size() - 1; ++a)
	{
		if (expr[a].type == TokenType::OPEN_BRACKET)
			openBracket += 1;
		else if (expr[a].type == TokenType::CLOSE_BRACKET)
			openBracket -= 1;

		// checks value
		if (expr[a].type == TokenType::INT_VALUE)
		{
			if ((expr[a - 1].type != ss ||
					expr[a + 1].type < TokenType::PLUS || expr[a + 1].type > TokenType::MOD) &&
				(expr[a - 1].type != ss ||
					expr[a + 1].type != TokenType::SEMICOLON)
				&&
				(expr[a - 1].type < TokenType::PLUS || expr[a - 1].type > TokenType::MOD ||
					expr[a + 1].type != TokenType::CLOSE_BRACKET) &&
				(expr[a - 1].type < TokenType::PLUS || expr[a - 1].type > TokenType::MOD ||
					expr[a + 1].type < TokenType::PLUS || expr[a + 1].type > TokenType::MOD) &&
				(expr[a - 1].type < TokenType::PLUS || expr[a - 1].type > TokenType::MOD ||
					expr[a + 1].type != TokenType::SEMICOLON)
				&&
				(expr[a - 1].type != TokenType::OPEN_BRACKET ||
					expr[a + 1].type < TokenType::PLUS || expr[a + 1].type > TokenType::MOD) &&
				(expr[a - 1].type != TokenType::OPEN_BRACKET ||
					expr[a + 1].type != TokenType::CLOSE_BRACKET))
			{
				error("1invalid integer expression");
			}
		}
		// checks operator
		else if (expr[a].type >= TokenType::PLUS && expr[a].type <= TokenType::MOD)
		{
			if ((expr[a - 1].type != TokenType::INT_VALUE ||
					expr[a + 1].type != TokenType::INT_VALUE) &&
				(expr[a - 1].type != TokenType::INT_VALUE ||
					expr[a + 1].type != TokenType::OPEN_BRACKET)
				&&
				(expr[a - 1].type != TokenType::CLOSE_BRACKET ||
					expr[a + 1].type != TokenType::INT_VALUE) &&
				(expr[a - 1].type != TokenType::CLOSE_BRACKET ||
					expr[a + 1].type != TokenType::OPEN_BRACKET))
			{
				error("2invalid integer expression");
			}
		}
		// checks open bracket
		else if (expr[a].type == TokenType::OPEN_BRACKET)
		{
			if ((expr[a - 1].type != ss ||
					expr[a + 1].type != TokenType::INT_VALUE) &&
				(expr[a - 1].type != ss ||
					expr[a + 1].type != TokenType::OPEN_BRACKET)
				&&
				(expr[a].type < TokenType::PLUS || expr[a].type > TokenType::MOD ||
					expr[a + 1].type != TokenType::INT_VALUE) &&
				(expr[a].type < TokenType::PLUS || expr[a].type > TokenType::MOD ||
					expr[a + 1].type != TokenType::OPEN_BRACKET)
				&&
				(expr[a - 1].type != TokenType::OPEN_BRACKET ||
					expr[a + 1].type != TokenType::INT_VALUE) &&
				(expr[a - 1].type != TokenType::OPEN_BRACKET ||
					expr[a + 1].type != TokenType::OPEN_BRACKET))
			{
				error("3invalid integer expression");
			}
		}
		// checks close bracket
		else if (expr[a].type == TokenType::CLOSE_BRACKET)
		{
			if ((expr[a - 1].type != TokenType::INT_VALUE ||
					expr[a + 1].type < TokenType::PLUS || expr[a + 1].type > TokenType::MOD) &&
				(expr[a - 1].type != TokenType::INT_VALUE ||
					expr[a + 1].type != TokenType::CLOSE_BRACKET) &&
				(expr[a - 1].type != TokenType::INT_VALUE ||
					expr[a + 1].type != TokenType::SEMICOLON)
				&&
				(expr[a - 1].type != TokenType::CLOSE_BRACKET ||
					expr[a + 1].type < TokenType::PLUS || expr[a + 1].type > TokenType::MOD) &&
				(expr[a - 1].type != TokenType::CLOSE_BRACKET ||
					expr[a + 1].type != TokenType::CLOSE_BRACKET) &&
				(expr[a - 1].type != TokenType::CLOSE_BRACKET ||
					expr[a + 1].type != TokenType::SEMICOLON))
			{
				error("4invalid integer expression");
			}
		}
		else
		{
			error("5invalid integer expression");
		}
	}

	if (openBracket > 0)
		error("closing bracket not found");
	else if (openBracket < 0)
		error("opening bracket not found");
}

void EvaluateIntExpression(std::vector<std::string>& expr, std::size_t& index)
{
	if (expr[index] == "+")
		expr[index - 1] = std::to_string(stoi(expr[index - 1]) + stoi(expr[index + 1]));
	else if (expr[index] == "-")
		expr[index - 1] = std::to_string(stoi(expr[index - 1]) - stoi(expr[index + 1]));
	else if (expr[index] == "*")
		expr[index - 1] = std::to_string(stoi(expr[index - 1]) * stoi(expr[index + 1]));
	else if (expr[index] == "/")
		expr[index - 1] = std::to_string(stoi(expr[index - 1]) / stoi(expr[index + 1]));
	else if (expr[index] == "%")
		expr[index - 1] = std::to_string(stoi(expr[index - 1]) % stoi(expr[index + 1]));
	else
		error("Did you forget to add an operator?");

	expr.erase(expr.begin() + index);
	expr.erase(expr.begin() + index);

	index -= 1;
}

std::string IntParser(const std::vector<Token>& tokens)
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
				if (expr[b] == "*")
				{
					EvaluateIntExpression(expr, b);
					closeBrakcetIndex -= 2;
				}
				else if (expr[b] == "/")
				{
					EvaluateIntExpression(expr, b);
					closeBrakcetIndex -= 2;
				}
				else if (expr[b] == "%")
				{
					EvaluateIntExpression(expr, b);
					closeBrakcetIndex -= 2;
				}
			}

			for (std::size_t b = openBracketIndex + 1; b < closeBrakcetIndex; ++b)
			{
				if (expr[b] == "+")
				{
					EvaluateIntExpression(expr, b);
					closeBrakcetIndex -= 2;
				}
				else if (expr[b] == "-")
				{
					EvaluateIntExpression(expr, b);
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
		if (expr[a] == "*")
			EvaluateIntExpression(expr, a);
		else if (expr[a] == "/")
			EvaluateIntExpression(expr, a);
		else if (expr[a] == "%")
			EvaluateIntExpression(expr, a);
	}

	for (std::size_t a = 0; a < expr.size(); ++a)
	{
		if (expr[a] == "+")
			EvaluateIntExpression(expr, a);
		else if (expr[a] == "-")
			EvaluateIntExpression(expr, a);
	}

	return expr[0];
}
