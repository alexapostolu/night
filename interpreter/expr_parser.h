#pragma once

#include "../lib/string.h"
#include "../lib/array.h"
#include "../lib/error.h"

#include "../containers/variable.h"
#include "../containers/token.h"

#include "./helpers.h"

float EvalNum(const night::array<Token>& expr, int index, const night::string& op)
{
	if (op == "/" && night::stof(expr[index - 1].value) == 0)
		throw Error(night::_invalid_expression_, expr, index - 1, index + 1, "division by zero isn't allowed smh");

	if (op == "+")
		return night::stof(expr[index - 1].value) + night::stof(expr[index + 1].value);
	if (op == "-")
		return night::stof(expr[index - 1].value) - night::stof(expr[index + 1].value);
	if (op == "/")
		return night::stof(expr[index - 1].value) / night::stof(expr[index + 1].value);
	if (op == "*")
		return night::stof(expr[index - 1].value) * night::stof(expr[index + 1].value);
	if (op == "%")
		return night::stoi(expr[index - 1].value) % night::stoi(expr[index + 1].value);
}

void EvaluateNumeric(night::array<Token>& expr, int index, const night::string& op)
{
	if ((expr[index - 1].type == TokenType::STR_VALUE && expr[index + 1].type <= TokenType::STR_VALUE) ||
		(expr[index - 1].type <= TokenType::STR_VALUE && expr[index + 1].type == TokenType::STR_VALUE))
	{
		expr[index - 1] = Token{ TokenType::STR_VALUE, expr[index - 1].value + expr[index + 1].value };
	}
	else if (expr[index - 1].type == TokenType::INT_VALUE && expr[index + 1].type == TokenType::INT_VALUE)
	{
		expr[index - 1].value = night::itos(EvalNum(expr, index, op));
	}
	else if ((expr[index - 1].type == TokenType::INT_VALUE || expr[index - 1].type == TokenType::DEC_VALUE) 
		&& (expr[index + 1].type == TokenType::INT_VALUE || expr[index + 1].type == TokenType::DEC_VALUE))
	{
		if (op == "%" && expr[index - 1].type == TokenType::DEC_VALUE || expr[index + 1].type == TokenType::DEC_VALUE)
			throw Error(night::_invalid_expression_, expr, index - 1, index + 1, "operator '%' can only work on values of type 'int'");

		expr[index - 1].type = TokenType::DEC_VALUE;
		expr[index - 1].value = night::ftos(EvalNum(expr, index, op));
	}
	else
	{
		throw Error(night::_invalid_expression_, expr, index - 1, index + 1, "operator '"_s + op + "' can only be used on values of type 'int' or 'dec', or if one of the values is of type 'str'");
	}
}

bool EvalNumComp(const Token& val1, const Token& val2, const night::string& op)
{
	if (op == ">")
		return night::stof(val1.value) > night::stof(val2.value);
	if (op == "<")
		return night::stof(val1.value) < night::stof(val2.value);
	if (op == ">=")
		return night::stof(val1.value) >= night::stof(val2.value);
	if (op == "<=")
		return night::stof(val1.value) <= night::stof(val2.value);
}

void EvaluateNumComparison(night::array<Token>& expr, int index, const night::string& op)
{
	if ((expr[index - 1].type != TokenType::INT_VALUE && expr[index - 1].type != TokenType::DEC_VALUE) ||
		(expr[index + 1].type != TokenType::INT_VALUE && expr[index + 1].type != TokenType::DEC_VALUE))
		throw Error(night::_invalid_expression_, expr, index - 1, index + 1, "operator '"_s + op + "' can only be used on values of type 'int' or 'dec'");

	expr[index - 1].type = TokenType::BIT_VALUE;
	expr[index - 1].value = EvalNumComp(expr[index - 1], expr[index + 1], op) ? "true" : "false";
}

void EvaluateComparison(night::array<Token>& expr, int index, const night::string& op)
{
	if (expr[index - 1].type != expr[index + 1].type) {
		throw Error(night::_invalid_expression_, expr, index - 1, index + 1,
			"operator '"_s + op + "' can only compare to values of the same type");
	}
	if (expr[index - 1].type < TokenType::BIT_VALUE && expr[index - 1].type > TokenType::STR_VALUE) {
		throw Error(night::_invalid_expression_, expr, index - 1, index - 1, "operator '"_s + op + "' can only compare values of type 'bit', 'syb', 'int', 'dec', or 'str'");
	}
	if (expr[index + 1].type < TokenType::BIT_VALUE || expr[index + 1].type > TokenType::STR_VALUE) {
		throw Error(night::_invalid_expression_, expr, index + 1, index + 1, "operator '"_s + op + "' can only compare values of type 'bit', 'syb', 'int', 'dec', or 'str'");
	}

	expr[index - 1].type = TokenType::BIT_VALUE;

	if (op == "==")
		expr[index - 1].value = expr[index - 1].value == expr[index + 1].value ? "true" : "false";
	else
		expr[index - 1].value = expr[index - 1].value != expr[index + 1].value ? "true" : "false";
}

bool EvalBool(const Token& val1, const Token& val2, const night::string& op)
{
	if (op == "||")
		return val1.value == "true" || val1.value == "true";
	if (op == "&&")
		return val1.value == "true" || val1.value == "true";
}

void EvaluateBoolean(night::array<Token>& expr, int index, const night::string& op)
{
	if (expr[index - 1].type != TokenType::BIT_VALUE || expr[index + 1].type != TokenType::BIT_VALUE)
		throw Error(night::_invalid_expression_, expr, index - 1, index + 1, "operator '"_s + op + "' can only be used on values of type 'bit'");

	expr[index - 1].value = EvalBool(expr[index - 1], expr[index + 1], op) ? "true" : "false";
}

void EvaluateExpression(night::array<Token>& expr, int& index, int* bracketIndex = nullptr)
{
	switch (expr[index].type)
	{
	case TokenType::PLUS:
		EvaluateNumeric(expr, index, "+");
		break;
	case TokenType::MINUS:
		EvaluateNumeric(expr, index, "-");
		break;
	case TokenType::DIVIDE:
		EvaluateNumeric(expr, index, "/");
		break;
	case TokenType::TIMES:
		EvaluateNumeric(expr, index, "*");
		break;
	case TokenType::MOD:
		EvaluateNumeric(expr, index, "%");
		break;
	case TokenType::GREATER:
		EvaluateNumComparison(expr, index, ">");
		break;
	case TokenType::SMALLER:
		EvaluateNumComparison(expr, index, "<");
		break;
	case TokenType::GREATER_EQUAL:
		EvaluateNumComparison(expr, index, ">=");
		break;
	case TokenType::SMALLER_EQUAL:
		EvaluateNumComparison(expr, index, "<=");
		break;
	case TokenType::EQUALS:
		EvaluateComparison(expr, index, "==");
		break;
	case TokenType::NOT_EQUALS:
		EvaluateComparison(expr, index, "!=");
		break;
	case TokenType::OR:
		EvaluateBoolean(expr, index, "||");
		break;
	case TokenType::AND:
		EvaluateBoolean(expr, index, "&&");
		break;
	}

	expr.remove(index);
	expr.remove(index);

	index--;

	if (bracketIndex != nullptr)
		*bracketIndex -= 2;
}

void EvalNot(night::array<Token>& expr, int& index)
{
	if (expr[index + 1].type != TokenType::BIT_VALUE)
		throw Error(night::_invalid_expression_, expr, index, index + 1, "operator '!' can only be performed on a value of type 'bit'");

	expr[index + 1].value = expr[index + 1].value == "true" ? "false" : "true";
	expr.remove(index--);
}

void EvalRegular(night::array<Token>& expr, const TokenType& type1, const TokenType& type2)
{
	for (int a = 0; a < expr.length(); ++a)
	{
		if (expr[a].type >= type1 && expr[a].type <= type2)
			EvaluateExpression(expr, a);
	}
}

void EvalBracket(night::array<Token>& expr, const TokenType& type1, const TokenType& type2, int start,
	int& end)
{
	for (int b = start + 1; b < end; ++b)
	{
		if (expr[b].type >= type1 && expr[b].type <= type2)
			EvaluateExpression(expr, b, &end);
	}
}

Token ParseExpression(night::array<Token> expr, const night::array<Variable>& vars)
{
	for (int a = 0; a < expr.length(); ++a)
	{
		if (expr[a].type == TokenType::VARIABLE)
		{
			bool isDefined = false;

			for (int b = 0; b < vars.length(); ++b)
			{
				if (expr[a].value == vars[b].name)
				{
					expr[a] = Token{ night::ttov(vars[b].type), vars[b].value };

					isDefined = true;
					break;
				}
			}

			/*

			if (!isDefined)
			{
				for (int b = 0; b < funcs.length(); ++b)
				{
					if (expr[a].value == funcs[b].name)
					{
						night::array<night::array<Token> > functionCall;
						functionCall.push_back(night::array<Token>());
						
						int bracketCount = 0;
						for (int c = a; ; ++c)
						{
							if (expr[c].type == TokenType::OPEN_BRACKET)
								bracketCount++;
							else if (expr[c].type == TokenType::CLOSE_BRACKET)
								bracketCount--;

							if (bracketCount == 0 && expr[c].type == TokenType::CLOSE_BRACKET)
							{
								functionCall.back() = expr.access(a, c);
								expr.remove(a + 1, c);

								break;
							}
						}

						Interpreter(functionCall);

						isDefined = true;
						break;
					}
				}
			}

			if (!isDefined)
			{
				for (int b = 0; b < arrs.length(); ++b)
				{
					if (expr[a].value == arrs[b].name)
					{
						expr[a] = arrs[b].elements[night::stoi(expr[a + 2].value)];

						for (int c = 0; c < 3; ++c)
							expr.remove(a + 1);

						isDefined = true;
						break;
					}
				}
			}

			*/

			if (!isDefined)
				throw Error(night::_undefined_token_, expr, a, a, "token is not defined");
		}
	}

	int openBracket = 0;
	for (int a = 0; a < expr.length(); ++a)
	{
		if (expr[a].type == TokenType::OPEN_BRACKET)
		{
			openBracket = a;
		}
		else if (expr[a].type == TokenType::CLOSE_BRACKET)
		{
			for (int b = openBracket + 1; b < a; ++b)
			{
				if (expr[b].type == TokenType::NOT)
				{
					EvalNot(expr, b);
					a--;
				}
			}

			EvalBracket(expr, TokenType::DIVIDE, TokenType::MOD, openBracket, a);
			EvalBracket(expr, TokenType::DIVIDE, TokenType::MOD, openBracket, a);
			EvalBracket(expr, TokenType::PLUS, TokenType::MINUS, openBracket, a);
			EvalBracket(expr, TokenType::GREATER, TokenType::SMALLER_EQUAL, openBracket, a);
			EvalBracket(expr, TokenType::EQUALS, TokenType::NOT_EQUALS, openBracket, a);
			EvalBracket(expr, TokenType::OR, TokenType::AND, openBracket, a);

			expr.remove(openBracket);
			expr.remove(a - 1);

			a = -1;
		}
	}

	for (int a = 0; a < expr.length(); ++a)
	{
		if (expr[a].type == TokenType::NOT)
			EvalNot(expr, a);
	}

	EvalRegular(expr, TokenType::DIVIDE, TokenType::MOD);
	EvalRegular(expr, TokenType::PLUS, TokenType::MINUS);
	EvalRegular(expr, TokenType::GREATER, TokenType::SMALLER_EQUAL);
	EvalRegular(expr, TokenType::EQUALS, TokenType::NOT_EQUALS);
	EvalRegular(expr, TokenType::OR, TokenType::AND);

	return expr[0];
}