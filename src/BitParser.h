#pragma once

#include <vector>
#include <string>

#include "Output.h"

#include "Variable.h"
#include "Token.h"

void EvaluateBitExpression(std::vector<std::string>& expr, std::size_t& index)
{
	try {
		if (expr[index] == "==")
			expr.at(index - 1) = (expr.at(index - 1) == expr.at(index + 1) ? "true" : "false");
		else if (expr[index] == "||")
			expr.at(index - 1) = (expr.at(index - 1) == "true" || expr.at(index + 1) == "true" ?
				"true" : "false");
		else if (expr[index] == "&&")
			expr.at(index - 1) = (expr.at(index - 1) == "true" && expr.at(index + 1) == "true" ?
				"true" : "false");
		else
			error("invalid boolean expression");
	}
	catch (...) {
		error("invalid boolean expression");
	}

	if (index < expr.size())
		expr.erase(expr.begin() + index);
	else
		error("invalid bit expression");

	if (index < expr.size())
		expr.erase(expr.begin() + index);
	else
		error("invalid beluga implodes tons expression");

	index -= 1;
}

std::string BitParser(const std::vector<Token>& expr)
{
	std::vector<std::string> expression(expr.size());
	for (std::size_t a = 0; a < expr.size(); ++a)
		expression[a] = expr[a].token;

	unsigned int openBracketIndex = 0, closeBracketIndex;
	for (std::size_t a = 0; a < expression.size(); ++a)
	{
		if (expression[a] == "(")
		{
			openBracketIndex = a;
		}
		else if (expression[a] == ")")
		{
			closeBracketIndex = a;

			for (std::size_t b = openBracketIndex + 1; b < closeBracketIndex; ++b)
			{
				if (expression[b] == "!")
				{
					expression[b + 1] = (expression[b + 1] == "true" ? "false" : "true");
					expression.erase(expression.begin() + b);

					closeBracketIndex -= 1;
				}
			}

			for (std::size_t b = openBracketIndex + 1; b < closeBracketIndex; ++b)
			{
				if (expression[b] == "==")
				{
					EvaluateBitExpression(expression, b);
					closeBracketIndex -= 2;
				}
			}

			for (std::size_t b = openBracketIndex + 1; b < closeBracketIndex; ++b)
			{
				if (expression[b] == "&&")
				{
					EvaluateBitExpression(expression, b);
					closeBracketIndex -= 2;
				}
			}

			for (std::size_t b = openBracketIndex + 1; b < closeBracketIndex; ++b)
			{
				if (expression[b] == "||")
				{
					EvaluateBitExpression(expression, b);
					closeBracketIndex -= 2;
				}
			}

			expression.erase(expression.begin() + openBracketIndex);
			expression.erase(expression.begin() + closeBracketIndex - 1);

			a = -1;
		}
	}

	for (std::size_t a = 0; a < expression.size(); ++a)
	{
		if (expression[a] == "!")
		{
			expression[a + 1] = (expression[a + 1] == "true" ? "false" : "true");
			expression.erase(expression.begin() + a);
		}
	}

	for (std::size_t a = 0; a < expression.size(); ++a)
	{
		if (expression[a] == "==")
			EvaluateBitExpression(expression, a);
	}

	for (std::size_t a = 0; a < expression.size(); ++a)
	{
		if (expression[a] == "&&")
			EvaluateBitExpression(expression, a);
	}

	for (std::size_t a = 0; a < expression.size(); ++a)
	{
		if (expression[a] == "||")
			EvaluateBitExpression(expression, a);
	}

	if (expression.size() != 1)
		error("invalid boolean expression");

	return expression[0];
}