#pragma once

#include <string>
#include <vector>

#include "Squid.h"

#include "Variable.h"
#include "Token.h"

void EvaluateIntExpression(std::vector<Token>& expr, std::size_t& index)
{
	try {
		if (expr.at(index - 1).type != TokenType::INT_VALUE ||
			expr.at(index + 1).type != TokenType::INT_VALUE)
			throw "";

		if (expr[index].type == TokenType::PLUS)
		{
			expr.at(index - 1).token = std::to_string(stoi(expr.at(index - 1).token) +
				stoi(expr.at(index + 1).token));
		}
		else if (expr[index].type == TokenType::MINUS)
		{
			expr.at(index - 1).token = std::to_string(stoi(expr.at(index - 1).token) -
				stoi(expr.at(index + 1).token));
		}
		else if (expr[index].type == TokenType::TIMES)
		{
			expr.at(index - 1).token = std::to_string(stoi(expr.at(index - 1).token) *
				stoi(expr.at(index + 1).token));
		}
		else if (expr[index].type == TokenType::DIVIDE)
		{
			expr.at(index - 1).token = std::to_string(stoi(expr.at(index - 1).token) /
				stoi(expr.at(index + 1).token));
		}
		else if (expr[index].type == TokenType::MOD)
		{
			expr.at(index - 1).token = std::to_string(stoi(expr.at(index - 1).token) %
				stoi(expr.at(index + 1).token));
		}
		else
		{
			throw 0;
		}
	}
	catch (int excNum) {
		throw DYNAMIC_SQUID(excNum);
	}
	catch (...) {
		throw _invalid_int_expr_;
	}

	for (int a = 0; a < 2; ++a)
	{
		if (index < expr.size())
			expr.erase(expr.begin() + index);
		else
			throw _invalid_int_expr_;
	}

	index -= 1;
}

std::string IntParser(std::vector<Token>& expr)
{
	for (std::size_t a = 0; a < expr.size(); ++a)
	{
		if (expr[a].type == TokenType::DEC_VALUE)
		{
			expr[a].type = TokenType::INT_VALUE;
			expr[a].token = std::to_string(stoi(expr[a].token));
		}
		else if (a < expr.size() - 1 && expr[a].type == TokenType::MINUS &&
			(expr[a + 1].type == TokenType::INT_VALUE || expr[a + 1].type == TokenType::DEC_VALUE))
		{
			expr[a + 1].type = TokenType::INT_VALUE;
			expr[a + 1].token = std::to_string(stoi(expr[a + 1].token) * -1);

			expr.erase(expr.begin() + a);
			a -= 1;
		}
	}

	unsigned int openBracketIndex = 0, closeBrakcetIndex;
	for (std::size_t a = 0; a < expr.size(); ++a)
	{
		if (expr[a].type == TokenType::OPEN_BRACKET)
		{
			openBracketIndex = a;
		}
		else if (expr[a].type == TokenType::CLOSE_BRACKET)
		{
			closeBrakcetIndex = a;

			for (std::size_t b = openBracketIndex + 1; b < closeBrakcetIndex; ++b)
			{
				if (expr[b].type == TokenType::TIMES || expr[b].type == TokenType::DIVIDE ||
					expr[b].type == TokenType::MOD)
				{
					EvaluateIntExpression(expr, b);
					closeBrakcetIndex -= 2;
				}
			}

			for (std::size_t b = openBracketIndex + 1; b < closeBrakcetIndex; ++b)
			{
				if (expr[b].type == TokenType::PLUS || expr[b].type == TokenType::MINUS)
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

	for (std::size_t a = 0; a < expr.size() - 1; ++a)
	{
		if (expr[a].type == TokenType::MINUS && expr[a + 1].type == TokenType::INT_VALUE)
		{
			expr[a + 1].token = std::to_string(stoi(expr[a + 1].token) * -1);

			expr.erase(expr.begin() + a);
			a -= 1;
		}
	}

	for (std::size_t a = 0; a < expr.size(); ++a)
	{
		if (expr[a].type == TokenType::TIMES || expr[a].type == TokenType::DIVIDE ||
			expr[a].type == TokenType::MOD)
			EvaluateIntExpression(expr, a);
	}

	for (std::size_t a = 0; a < expr.size(); ++a)
	{
		if (expr[a].type == TokenType::PLUS || expr[a].type == TokenType::MINUS)
			EvaluateIntExpression(expr, a);
	}

	if (expr.size() != 1 || expr[0].type != TokenType::INT_VALUE)
		throw _invalid_int_expr_;

	return expr[0].token;
}