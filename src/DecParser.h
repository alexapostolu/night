#pragma once

#include <vector>
#include <string>

#include "Output.h"

#include "Variable.h"
#include "Token.h"

void EvaluateDecExpression(std::vector<std::string>& expr, std::size_t& index)
{
	try
	{
		if (expr[index] == "+")
			expr.at(index - 1) = std::to_string(stof(expr.at(index - 1)) + stof(expr.at(index + 1)));
		else if (expr[index] == "-")
			expr.at(index - 1) = std::to_string(stof(expr.at(index - 1)) - stof(expr.at(index + 1)));
		else if (expr[index] == "*")
			expr.at(index - 1) = std::to_string(stof(expr.at(index - 1)) * stof(expr.at(index + 1)));
		else if (expr[index] == "/")
			expr.at(index - 1) = std::to_string(stof(expr.at(index - 1)) / stof(expr.at(index + 1)));
	}
	catch (...)
	{
		error("invalid decimal expression");
	}

	if (index < expr.size())
		expr.erase(expr.begin() + index);
	else
		error("invalid decimal expression");

	if (index < expr.size())
		expr.erase(expr.begin() + index);
	else
		error("invalid decimal expression");

	index -= 1;
}

std::string DecParser(const std::vector<Token>& tokens)
{
	std::vector<std::string> expr(tokens.size());
	for (std::size_t a = 0; a < tokens.size(); ++a)
		expr[a] = tokens[a].token;

	unsigned int openBracketIndex = 0, closeBrakcetIndex;
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
					EvaluateDecExpression(expr, b);
					closeBrakcetIndex -= 2;
				}
				else if (expr[b] == "/")
				{
					EvaluateDecExpression(expr, b);
					closeBrakcetIndex -= 2;
				}
			}

			for (std::size_t b = openBracketIndex + 1; b < closeBrakcetIndex; ++b)
			{
				if (expr[b] == "+")
				{
					EvaluateDecExpression(expr, b);
					closeBrakcetIndex -= 2;
				}
				else if (expr[b] == "-")
				{
					EvaluateDecExpression(expr, b);
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
			EvaluateDecExpression(expr, a);
		else if (expr[a] == "/")
			EvaluateDecExpression(expr, a);
	}

	for (std::size_t a = 0; a < expr.size(); ++a)
	{
		if (expr[a] == "+")
			EvaluateDecExpression(expr, a);
		else if (expr[a] == "-")
			EvaluateDecExpression(expr, a);
	}

	if (expr.size() > 1)
		error("invalid decimal expression");

	return expr[0];
}