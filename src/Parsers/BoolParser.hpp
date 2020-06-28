#pragma once

#include <string>
#include <vector>

#include "Token.hpp"

void EvaluateBool(std::vector<std::string>& expression, std::size_t& index, int set, char operation);

std::string BoolParser(const std::vector<Token>& tokens)
{
	std::vector<std::string> expression(tokens.size());
	for (std::size_t a = 0; a < tokens.size(); ++a)
		expression[a] = tokens[a].token;

	int openBracketIndex = 0, closeBrakcetIndex;
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
				if (expression[b] == "&&")
				{
					EvaluateBool(expression, b, openBracketIndex, '&');
					closeBrakcetIndex -= 2;
				}
			}

			for (std::size_t b = openBracketIndex + 1; b < closeBrakcetIndex; ++b)
			{
				if (expression[b] == "||")
				{
					EvaluateBool(expression, b, openBracketIndex, '|');
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
		if (expression[a] == "&&")
			EvaluateBool(expression, a, 0, '&');
	}

	for (std::size_t a = 0; a < expression.size(); ++a)
	{
		if (expression[a] == "||")
			EvaluateBool(expression, a, 0, '|');
	}

	return expression[0];
}

void EvaluateBool(std::vector<std::string>& expression, std::size_t& index, int set, char operation)
{
	std::string value = "false";
	if (operation == '|')
	{
		if (expression[index - 1] == "true" || expression[index + 1] == "true")
			value = "true";
	}
	else
	{
		if (expression[index - 1] == "true" && expression[index + 1] == "true")
			value = "true";
	}

	expression[index - 1] = value;

	expression.erase(expression.begin() + index);
	expression.erase(expression.begin() + index);

	index = set;
}
