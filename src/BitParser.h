#pragma once

#include <string>
#include <vector>

#include "Squid.h"

#include "Variable.h"
#include "Token.h"

void EvaluateBitExpression(std::vector<Token>& expr, std::size_t& index)
{
	try {
		if (expr[index].type == TokenType::EQUALS)
		{
			if (expr.at(index - 1).type != expr.at(index + 1).type)
				throw "";

			expr.at(index - 1).token = (expr.at(index - 1).token == expr.at(index + 1).token ?
				"true" : "false");

			expr.at(index - 1).type = TokenType::BIT_VALUE;
		}
		else if (expr[index].type == TokenType::NOT_EQUALS)
		{
			if (expr.at(index - 1).type != TokenType::BIT_VALUE ||
				expr.at(index + 1).type != TokenType::BIT_VALUE)
				throw "";

			expr.at(index - 1).token = (expr.at(index - 1).token != expr.at(index + 1).token ?
				"true" : "false");

			expr.at(index - 1).type = TokenType::BIT_VALUE;
		}
		else if (expr[index].type == TokenType::OR)
		{
			if (expr.at(index - 1).type != TokenType::BIT_VALUE ||
				expr.at(index + 1).type != TokenType::BIT_VALUE)
				throw "";

			expr.at(index - 1).token = (expr.at(index - 1).token == "true" ||
				expr.at(index + 1).token == "true" ? "true" : "false");
		}
		else if (expr[index].type == TokenType::AND)
		{
			if (expr.at(index - 1).type != TokenType::BIT_VALUE ||
				expr.at(index + 1).type != TokenType::BIT_VALUE)
				throw "";

			expr.at(index - 1).token = (expr.at(index - 1).token == "true" &&
				expr.at(index + 1).token == "true" ? "true" : "false");
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
		throw _invalid_bit_expr_;
	}

	for (int a = 0; a < 2; ++a)
	{
		if (index < expr.size())
			expr.erase(expr.begin() + index);
		else
			throw _invalid_bit_expr_;
	}

	index -= 1;
}

std::string BitParser(std::vector<Token>& expr)
{
	unsigned int openBracketIndex = 0, closeBracketIndex;
	for (std::size_t a = 0; a < expr.size(); ++a)
	{
		if (expr[a].type == TokenType::OPEN_BRACKET)
		{
			openBracketIndex = a;
		}
		else if (expr[a].type == TokenType::CLOSE_BRACKET)
		{
			closeBracketIndex = a;

			for (std::size_t b = openBracketIndex + 1; b < closeBracketIndex; ++b)
			{
				if (expr[b].type == TokenType::NOT)
				{
					expr[b + 1].token = (expr[b + 1].token == "true" ? "false" : "true");
					expr.erase(expr.begin() + b);

					closeBracketIndex -= 1;
				}
			}

			for (std::size_t b = openBracketIndex + 1; b < closeBracketIndex; ++b)
			{
				if (expr[b].type == TokenType::EQUALS || expr[b].type == TokenType::NOT_EQUALS)
				{
					EvaluateBitExpression(expr, b);
					closeBracketIndex -= 2;
				}
			}

			for (std::size_t b = openBracketIndex + 1; b < closeBracketIndex; ++b)
			{
				if (expr[b].type == TokenType::AND)
				{
					EvaluateBitExpression(expr, b);
					closeBracketIndex -= 2;
				}
			}

			for (std::size_t b = openBracketIndex + 1; b < closeBracketIndex; ++b)
			{
				if (expr[b].type == TokenType::OR)
				{
					EvaluateBitExpression(expr, b);
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
		if (expr[a].type == TokenType::NOT)
		{
			expr[a + 1].token = (expr[a + 1].token == "true" ? "false" : "true");
			expr.erase(expr.begin() + a);
		}
	}

	for (std::size_t a = 0; a < expr.size(); ++a)
	{
		if (expr[a].type == TokenType::EQUALS || expr[a].type == TokenType::NOT_EQUALS)
			EvaluateBitExpression(expr, a);
	}

	for (std::size_t a = 0; a < expr.size(); ++a)
	{
		if (expr[a].type == TokenType::AND)
			EvaluateBitExpression(expr, a);
	}

	for (std::size_t a = 0; a < expr.size(); ++a)
	{
		if (expr[a].type == TokenType::OR)
			EvaluateBitExpression(expr, a);
	}

	if (expr.size() != 1 || expr[0].type != TokenType::BIT_VALUE)
		throw _invalid_bit_expr_;

	return expr[0].token;
}