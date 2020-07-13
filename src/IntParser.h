#pragma once

#include <vector>
#include <string>

#include "Output.h"

#include "Variable.h"
#include "Token.h"

void EvaluateIntExpression(std::vector<std::string>& expr, std::size_t& index)
{
	try
	{
		if (expr[index] == "+")
			expr.at(index - 1) = std::to_string(stoi(expr.at(index - 1)) + stoi(expr.at(index + 1)));
		else if (expr[index] == "-")
			expr.at(index - 1) = std::to_string(stoi(expr.at(index - 1)) - stoi(expr.at(index + 1)));
		else if (expr[index] == "*")
			expr.at(index - 1) = std::to_string(stoi(expr.at(index - 1)) * stoi(expr.at(index + 1)));
		else if (expr[index] == "/")
			expr.at(index - 1) = std::to_string(stoi(expr.at(index - 1)) / stoi(expr.at(index + 1)));
		else if (expr[index] == "%")
			expr.at(index - 1) = std::to_string(stoi(expr.at(index - 1)) % stoi(expr.at(index + 1)));
	}
	catch (...)
	{
		error("invalid integer expression");
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

std::string IntParser(const std::vector<Token>& expr)
{
	std::vector<std::string> expression(expr.size());
	for (std::size_t a = 0; a < expr.size(); ++a)
		expression[a] = expr[a].token;

	unsigned int openBracketIndex = 0, closeBrakcetIndex;
	for (std::size_t a = 0; a < expression.size(); ++a)
	{
		if (expression[a] == "(")
		{
			openBracketIndex = a;
		}
		else if (expression[a] == ")")
		{
			closeBrakcetIndex = a;

			for (std::size_t b = openBracketIndex + 1; b < closeBrakcetIndex; ++b)
			{
				if (expression[b] == "*")
				{
					EvaluateIntExpression(expression, b);
					closeBrakcetIndex -= 2;
				}
				else if (expression[b] == "/")
				{
					EvaluateIntExpression(expression, b);
					closeBrakcetIndex -= 2;
				}
				else if (expression[b] == "%")
				{
					EvaluateIntExpression(expression, b);
					closeBrakcetIndex -= 2;
				}
			}

			for (std::size_t b = openBracketIndex + 1; b < closeBrakcetIndex; ++b)
			{
				if (expression[b] == "+")
				{
					EvaluateIntExpression(expression, b);
					closeBrakcetIndex -= 2;
				}
				else if (expression[b] == "-")
				{
					EvaluateIntExpression(expression, b);
					closeBrakcetIndex -= 2;
				}
			}

			expression.erase(expression.begin() + openBracketIndex);
			expression.erase(expression.begin() + closeBrakcetIndex - 1);

			a = -1;
		}
	}

	for (std::size_t a = 0; a < expression.size(); ++a)
	{
		if (expression[a] == "*")
			EvaluateIntExpression(expression, a);
		else if (expression[a] == "/")
			EvaluateIntExpression(expression, a);
		else if (expression[a] == "%")
			EvaluateIntExpression(expression, a);
	}

	for (std::size_t a = 0; a < expression.size(); ++a)
	{
		if (expression[a] == "+")
			EvaluateIntExpression(expression, a);
		else if (expression[a] == "-")
			EvaluateIntExpression(expression, a);
	}

	if (expression.size() > 1)
		error("invalid integer expression");

	return expression[0];
}