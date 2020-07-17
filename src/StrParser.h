#pragma once

#include <string>
#include <vector>

#include "Squid.h"
 
#include "Variable.h"
#include "Token.h"

void EvaluateStrExpression(std::vector<Token>& expr, std::size_t& index)
{
	try {
		if (expr.at(index - 1).type != TokenType::STR_VALUE ||
			expr.at(index + 1).type != TokenType::STR_VALUE)
			throw "";

		if (expr[index].type == TokenType::PLUS)
			expr.at(index - 1).token = expr.at(index - 1).token + expr.at(index + 1).token;
		else
			throw 0;
	}
	catch (int excNum) {
		throw DYNAMIC_SQUID(excNum);
	}
	catch (...) {
		throw _invalid_str_expr_;
	}

	for (int a = 0; a < 2; ++a)
	{
		if (index < expr.size())
			expr.erase(expr.begin() + index);
		else
			throw _invalid_str_expr_;
	}

	index -= 1;
}

std::string StrParser(std::vector<Token>& expr)
{
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
				if (expr[b].type == TokenType::PLUS)
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
		if (expr[a].type == TokenType::PLUS)
			EvaluateStrExpression(expr, a);
	}

	if (expr.size() != 1 || expr[0].type != TokenType::STR_VALUE)
		throw _invalid_str_expr_;

	return expr[0].token;
}