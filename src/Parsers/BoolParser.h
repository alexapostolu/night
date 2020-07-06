#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "Variable.h"
#include "Error.h"
#include "Token.h"

void CheckBool(int start, std::vector<Token>& expr, const std::vector<Variable>& vars, TokenType ss = TokenType::ASSIGNMENT, int end = 1)
{
	/* valid boolean expressions:
	 =  true &&          true && true          =  ( true          true ) &&          =  ! true
	 =  true ;           true && (             =  ( (             true ) )           =  ! (
	 && true )           )    && true          && ( true          true ) ;           (  ! true
	 && true &&          )    && (             && ( (             )    ) &&          (  ! (
	 && true ;                                 (  ( true          )    ) )           && ! true
	 (  true &&                                (  ( (             )    ) ;           && ! (
	 (  true )
	 */

	int openBracket = 0;
	for (std::size_t a = start; a < expr.size() - end; ++a)
	{
		if (expr[a].type == TokenType::OPEN_BRACKET)
			openBracket += 1;
		else if (expr[a].type == TokenType::CLOSE_BRACKET)
			openBracket -= 1;

		// checks value
		if (expr[a].type == TokenType::BOOL_VALUE)
		{
			if ((expr[a - 1].type != ss ||
					expr[a + 1].type < TokenType::OR || expr[a + 1].type > TokenType::AND) &&
				(expr[a - 1].type != ss ||
					expr[a + 1].type != TokenType::SEMICOLON)
				&&
				(expr[a - 1].type < TokenType::OR || expr[a - 1].type > TokenType::AND ||
					expr[a + 1].type != TokenType::CLOSE_BRACKET) &&
				(expr[a - 1].type < TokenType::OR || expr[a - 1].type > TokenType::AND ||
					expr[a + 1].type < TokenType::OR || expr[a + 1].type > TokenType::AND) &&
				(expr[a - 1].type < TokenType::OR || expr[a - 1].type > TokenType::AND ||
					expr[a + 1].type != TokenType::SEMICOLON)
				&&
				(expr[a - 1].type != TokenType::OPEN_BRACKET ||
					expr[a + 1].type < TokenType::OR || expr[a + 1].type > TokenType::AND) &&
				(expr[a - 1].type != TokenType::OPEN_BRACKET ||
					expr[a + 1].type != TokenType::CLOSE_BRACKET))
			{
				if (expr[a - 1].type != TokenType::NOT && expr[a + 1].type != TokenType::NOT)
					error("1invalid boolean expression");
			}
		}
		// checks OR and AND operator
		else if (expr[a].type >= TokenType::OR && expr[a].type <= TokenType::AND)
		{
			if ((expr[a - 1].type != TokenType::BOOL_VALUE ||
					expr[a + 1].type != TokenType::BOOL_VALUE) &&
				(expr[a - 1].type != TokenType::BOOL_VALUE ||
					expr[a + 1].type != TokenType::OPEN_BRACKET)
				&&
				(expr[a - 1].type != TokenType::CLOSE_BRACKET ||
					expr[a + 1].type != TokenType::BOOL_VALUE) &&
				(expr[a - 1].type != TokenType::CLOSE_BRACKET ||
					expr[a + 1].type != TokenType::OPEN_BRACKET))
			{
				if (expr[a - 1].type != TokenType::NOT && expr[a + 1].type != TokenType::NOT)
					error("2invalid boolean expression");
			}
		}
		// checks open bracket
		else if (expr[a].type == TokenType::OPEN_BRACKET)
		{
			if ((expr[a - 1].type != ss ||
					expr[a + 1].type != TokenType::BOOL_VALUE) &&
				(expr[a - 1].type != ss ||
					expr[a + 1].type != TokenType::OPEN_BRACKET)
				&&
				(expr[a].type < TokenType::OR || expr[a].type > TokenType::AND ||
					expr[a + 1].type != TokenType::BOOL_VALUE) &&
				(expr[a].type < TokenType::OR || expr[a].type > TokenType::AND ||
					expr[a + 1].type != TokenType::OPEN_BRACKET)
				&&
				(expr[a - 1].type != TokenType::OPEN_BRACKET ||
					expr[a + 1].type != TokenType::BOOL_VALUE) &&
				(expr[a - 1].type != TokenType::OPEN_BRACKET ||
					expr[a + 1].type != TokenType::OPEN_BRACKET))
			{
				if (expr[a - 1].type != TokenType::NOT && expr[a + 1].type != TokenType::NOT)
					error("3invalid boolean expression");
			}
		}
		// checks close bracket
		else if (expr[a].type == TokenType::CLOSE_BRACKET)
		{
			if ((expr[a - 1].type != TokenType::BOOL_VALUE ||
					expr[a + 1].type < TokenType::OR || expr[a + 1].type > TokenType::AND) &&
				(expr[a - 1].type != TokenType::BOOL_VALUE ||
					expr[a + 1].type != TokenType::CLOSE_BRACKET) &&
				(expr[a - 1].type != TokenType::BOOL_VALUE ||
					expr[a + 1].type != TokenType::SEMICOLON)
				&&
				(expr[a - 1].type != TokenType::CLOSE_BRACKET ||
					expr[a + 1].type < TokenType::OR || expr[a + 1].type > TokenType::AND) &&
				(expr[a - 1].type != TokenType::CLOSE_BRACKET ||
					expr[a + 1].type != TokenType::CLOSE_BRACKET) &&
				(expr[a - 1].type != TokenType::CLOSE_BRACKET ||
					expr[a + 1].type != TokenType::SEMICOLON))
			{
				if (expr[a - 1].type != TokenType::NOT && expr[a + 1].type != TokenType::NOT)
					error("4invalid boolean expression");
			}
		}
		// checks NOT operator
		else if (expr[a].type == TokenType::NOT)
		{
			if ((expr[a - 1].type != ss ||
					expr[a + 1].type != TokenType::BOOL_VALUE) &&
				(expr[a - 1].type != ss ||
					expr[a + 1].type != TokenType::OPEN_BRACKET)
				&&
				(expr[a - 1].type != TokenType::OPEN_BRACKET ||
					expr[a + 1].type != TokenType::BOOL_VALUE) &&
				(expr[a - 1].type != TokenType::OPEN_BRACKET ||
					expr[a + 1].type != TokenType::OPEN_BRACKET)
				&&
				(expr[a - 1].type < TokenType::OR || expr[a - 1].type > TokenType::AND ||
					expr[a + 1].type != TokenType::BOOL_VALUE) &&
				(expr[a - 1].type < TokenType::OR || expr[a - 1].type > TokenType::AND ||
					expr[a + 1].type != TokenType::OPEN_BRACKET))
			{
				error("5invalid boolean expression");
			}
		}
		else
		{
			error("6invalid boolean expression");
		}
	}

	if (openBracket > 0)
		error("closing bracket not found");
	else if (openBracket < 0)
		error("opening bracket not found");
}

void EvaluateBoolExpression(std::vector<std::string>& expr, std::size_t& index)
{
	if (expr[index] == "|")
	{
		if (expr[index - 1] == "true" || expr[index + 1] == "true")
			expr[index - 1] = "true";
		else
			expr[index - 1] = "false";
	}
	else if (expr[index] == "&")
	{
		if (expr[index - 1] == "true" && expr[index + 1] == "true")
			expr[index - 1] = "true";
		else
			expr[index - 1] = "false";
	}
	else
	{
		error("Did you forget to add a boolean operator?");
	}

	expr.erase(expr.begin() + index);
	expr.erase(expr.begin() + index);

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

	return expr[0];
}
