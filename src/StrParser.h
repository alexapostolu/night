#pragma once

#include <vector>

#include "Output.h"
 
#include "Variable.h"
#include "Token.h"

void EvaluateStrExpression(std::vector<Token>& expr, std::size_t& index)
{
	try
	{
		if (expr.at(index - 1).type == TokenType::STR_VALUE && expr.at(index + 1).type ==
			TokenType::STR_VALUE)
			expr.at(index - 1).token = expr.at(index - 1).token + expr.at(index + 1).token;
		else
			error("invalid string expression");
	}
	catch (...)
	{
		error("invalid string expression");
	}

	if (index < expr.size())
		expr.erase(expr.begin() + index);
	else
		error("invalid integer expression");

	if (index < expr.size())
		expr.erase(expr.begin() + index);
	else
		error("invalid integer expression");

	index -= 1;
}

std::string StrParser(std::vector<Token>& expr)
{
	unsigned int openBracketIndex = 0, closeBrakcetIndex;
	for (std::size_t a = 0; a < expr.size(); ++a)
	{
		if (expr[a].token == "(")
		{
			openBracketIndex = a;
		}
		else if (expr[a].token == ")")
		{
			closeBrakcetIndex = a;

			for (std::size_t b = openBracketIndex + 1; b < closeBrakcetIndex; ++b)
			{
				if (expr[b].token == "+")
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
		if (expr[a].token == "+")
			EvaluateStrExpression(expr, a);
	}

	if (expr.size() > 1)
		error("invalid string expression");

	return expr[0].token;
}