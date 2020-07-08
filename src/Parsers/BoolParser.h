#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "Variable.h"
#include "Error.h"
#include "Token.h"

void EvaluateBoolExpression(std::vector<std::string>& expr, std::size_t& index)
{
	try
	{
		if (expr[index] == "|")
		{
			if (expr.at(index - 1) == "true" || expr.at(index + 1) == "true")
				expr.at(index - 1) = "true";
			else
				expr.at(index - 1) = "false";
		}
		else if (expr[index] == "&")
		{
			if (expr.at(index - 1) == "true" && expr.at(index + 1) == "true")
				expr.at(index - 1) = "true";
			else
				expr.at(index - 1) = "false";
		}
		else
		{
			error("invalid boolean expression");
		}
	}
	catch (const std::out_of_range&)
	{
		error("invalid boolean expression");
	}
	catch (...)
	{
		error("MEEP");
	}

	if (expr.size() > index)
		expr.erase(expr.begin() + index);
	else
		error("invalid boolean expression");

	if (expr.size() > index)
		expr.erase(expr.begin() + index);
	else
		error("invalid boolean expression");

	index -= 1;
}

std::string BoolParser(const std::vector<Token>& tokens)
{
	std::vector<std::string> expr(tokens.size());
	for (std::size_t a = 0; a < tokens.size(); ++a)
		expr[a] = tokens[a].token;

	unsigned int openBracketIndex = 0, closeBracketIndex;
	for (std::size_t a = 0; a < expr.size(); ++a)
	{
		if (expr[a] == "(")
		{
			openBracketIndex = a;
		}
		else if (expr[a] == ")")
		{
			closeBracketIndex = a;

			for (std::size_t b = openBracketIndex + 1; b < closeBracketIndex; ++b)
			{
				if (expr[b] == "!")
				{
					expr[b + 1] = (expr[b + 1] == "true" ? "false" : "true");
					expr.erase(expr.begin() + b);

					closeBracketIndex -= 1;
				}
			}

			for (std::size_t b = openBracketIndex + 1; b < closeBracketIndex; ++b)
			{
				if (expr[b] == "&")
				{
					EvaluateBoolExpression(expr, b);
					closeBracketIndex -= 2;
				}
			}

			// so it turns out that 'AND' actually has a higher operator precedence than 'OR'
			// and I did not know that until now...
			for (std::size_t b = openBracketIndex + 1; b < closeBracketIndex; ++b)
			{
				if (expr[b] == "|")
				{
					EvaluateBoolExpression(expr, b);
					closeBracketIndex -= 2;
				}
			}

			expr.erase(expr.begin() + openBracketIndex);
			expr.erase(expr.begin() + closeBracketIndex - 1);

			a = -1;
		}
	}

	for (std::size_t a = 0; a < expr.size(); ++a)
	{
		if (expr[a] == "!")
		{
			expr[a + 1] = (expr[a + 1] == "true" ? "false" : "true");
			expr.erase(expr.begin() + a);
		}
	}

	for (std::size_t a = 0; a < expr.size(); ++a)
	{
		if (expr[a] == "&")
			EvaluateBoolExpression(expr, a);
	}

	for (std::size_t a = 0; a < expr.size(); ++a)
	{
		if (expr[a] == "|")
			EvaluateBoolExpression(expr, a);
	}

	if (expr.size() > 1)
		error("invalid boolean expression");

	return expr[0];
}
