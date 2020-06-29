#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "Token.h"
#include "Variable.h"

bool CheckInt(std::vector<Token>& expr, const std::vector<Variable>& vars)
{
	// turn variables into their values
	for (std::size_t a = 3; a < expr.size() - 1; ++a)
	{
		if (expr[a].type == TokenType::VARIABLE)
		{
			bool definedVariable = false;
			for (std::size_t b = 0; b < vars.size(); ++b)
			{
				if (expr[a].token == vars[b].name && vars[b].type == "int")
				{
					expr[a].type = TokenType::INT_VALUE;
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

	/* valid integer expressions:

	 = 5 +          5 + 5          = ( 5          5 ) +
	 = 5 ;          5 + (          = ( (          5 ) )
	 + 5 )          ) + 5          + ( 5          5 ) ;
	 + 5 +          ) + (          + ( (          ) ) +
	 + 5 ;                         ( ( 5          ) ) )
	 ( 5 +                         ( ( (          ) ) ;
	 ( 5 )

	 */

	unsigned int openBracket = 0;
	for (std::size_t a = 3; a < expr.size() - 1; ++a)
	{
		if (expr[a].type == TokenType::OPEN_BRACKET)
			openBracket += 1;
		else if (expr[a].type == TokenType::CLOSE_BRACKET)
			openBracket -= 1;

		// checks value
		if (expr[a].type == TokenType::INT_VALUE)
		{
			if ((expr[a - 1].type != TokenType::ASSIGNMENT ||
				expr[a + 1].type < TokenType::PLUS || expr[a + 1].type > TokenType::MOD) &&
				(expr[a - 1].type != TokenType::ASSIGNMENT ||
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
				return false;
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
				return false;
			}
		}
		// checks open bracket
		else if (expr[a].type == TokenType::OPEN_BRACKET)
		{
			if ((expr[a - 1].type != TokenType::ASSIGNMENT ||
					expr[a + 1].type != TokenType::INT_VALUE) &&
				(expr[a - 1].type != TokenType::ASSIGNMENT ||
					expr[a + 1].type != TokenType::OPEN_BRACKET)
				&&
				(expr[a].type < TokenType::PLUS && expr[a].type > TokenType::MOD ||
					expr[a + 1].type != TokenType::INT_VALUE) &&
				(expr[a].type < TokenType::PLUS && expr[a].type > TokenType::MOD ||
					expr[a + 1].type != TokenType::OPEN_BRACKET)
				&&
				(expr[a - 1].type != TokenType::OPEN_BRACKET ||
					expr[a + 1].type != TokenType::INT_VALUE) &&
				(expr[a - 1].type != TokenType::OPEN_BRACKET ||
					expr[a + 1].type != TokenType::OPEN_BRACKET))
			{
				return false;
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
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	if (openBracket > 0)
	{
		std::cout << "Error - missing closing bracket\n";
		exit(0);
	}
	else if (openBracket < 0)
	{
		std::cout << "Error - missing opening bracket\n";
		exit(0);
	}

	return true;
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
	else
		expr[index - 1] = std::to_string(stoi(expr[index - 1]) % stoi(expr[index + 1]));

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
