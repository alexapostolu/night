#pragma once

#include <vector>
#include <string>

#include "Variable.h"
#include "Error.h"
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
		else
			error("Did you forget to add an operator?");
	}
	catch (...)
	{
		error("invalid integer expression");
	}

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

	if (expr.size() > 1)
		error("invalid integer expression");

	return expr[0];
}
