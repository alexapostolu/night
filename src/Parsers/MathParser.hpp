#pragma once

#include <string>
#include <vector>

#include "Token.hpp"

void EvaluateInt(std::vector<std::string>& expression, std::size_t& index, int set, char operation);

std::string MathParser(const std::vector<Token>& tokens)
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
				if (expression[b] == "*")
				{
					EvaluateInt(expression, b, openBracketIndex, '*');
					closeBrakcetIndex -= 2;
				}
				else if (expression[b] == "/")
				{
					EvaluateInt(expression, b, openBracketIndex, '/');
					closeBrakcetIndex -= 2;
				}
				else if (expression[b] == "%")
				{
					EvaluateInt(expression, b, openBracketIndex, '%');
					closeBrakcetIndex -= 2;
				}
			}

			for (std::size_t b = openBracketIndex + 1; b < closeBrakcetIndex; ++b)
			{
				if (expression[b] == "+")
				{
					EvaluateInt(expression, b, openBracketIndex, '+');
					closeBrakcetIndex -= 2;
				}
				else if (expression[b] == "-")
				{
					EvaluateInt(expression, b, openBracketIndex, '-');
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
			EvaluateInt(expression, a, 0, '*');
		else if (expression[a] == "/")
			EvaluateInt(expression, a, 0, '/');
		else if (expression[a] == "%")
			EvaluateInt(expression, a, 0, '%');
	}

	for (std::size_t a = 0; a < expression.size(); ++a)
	{
		if (expression[a] == "+")
			EvaluateInt(expression, a, 0, '+');
		else if (expression[a] == "-")
			EvaluateInt(expression, a, 0, '-');
	}

	return expression[0];
}

void EvaluateInt(std::vector<std::string>& expression, std::size_t& index, int set, char operation)
{
	int value;
	if (operation == '+')
		value = stoi(expression[index - 1]) + stoi(expression[index + 1]);
	else if (operation == '-')
		value = stoi(expression[index - 1]) - stoi(expression[index + 1]);
	else if (operation == '*')
		value = stoi(expression[index - 1]) * stoi(expression[index + 1]);
	else if (operation == '/')
		value = stoi(expression[index - 1]) / stoi(expression[index + 1]);
	else
		value = stoi(expression[index - 1]) % stoi(expression[index + 1]);

	expression[index - 1] = std::to_string(value);

	expression.erase(expression.begin() + index);
	expression.erase(expression.begin() + index);

	index = set;
}
