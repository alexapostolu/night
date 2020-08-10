#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "Parser.h"

#include "DataTypes/Error.h"
#include "DataTypes/Token.h"
#include "DataTypes/Variable.h"
#include "DataTypes/Function.h"

void Parser(std::vector<Token>& tokens, bool runtime, bool recursion = false);

namespace night {

std::string bit_to_str(bool val)
{
	return val ? "true" : "false";
}

bool str_to_bit(const Token& val)
{
	return val.token == "true" ? true : false;
}

void val_to_bit(Token& val)
{
	val.type = TokenType::BIT_VALUE;
	val.token = val.token == "1" ? "true" : "false";
}

float abs(float val)
{
	return val < 0 ? -val : val;
}

} // namespace night

float EvalNum(float val1, float val2, std::string op)
{
	if (op == "+")
		return val1 + val2;
	if (op == "-")
		return val1 - val2;
	if (op == "/")
	{
		if (night::abs(val2 - 0) < 0.1)
			throw "division by zero";
		return val1 / val2;
	}
	if (op == "*")
		return val1 * val2;
	if (op == "%")
		return (float)((int)val1 % (int)val2);
	if (op == ">")
		return val1 > val2;
	if (op == "<")
		return val1 < val2;
	if (op == ">=")
		return val1 >= val2;
	if (op == "<=")
		return val1 <= val2;

	return 0;
}

void EvaluateNumeric(std::vector<Token>& expr, std::size_t index, std::string op)
{
	try {
		if (expr[index - 1].type == TokenType::INT_VALUE && expr[index + 1].type == TokenType::INT_VALUE)
		{
			expr[index - 1].token = std::to_string((int)EvalNum(std::stof(expr[index - 1].token),
				std::stof(expr[index + 1].token), op));
		}
		else if ((expr[index - 1].type == TokenType::INT_VALUE ||
			expr[index - 1].type == TokenType::DEC_VALUE) &&
			(expr[index + 1].type == TokenType::INT_VALUE || expr[index + 1].type == TokenType::DEC_VALUE))
		{
			expr[index - 1].type = TokenType::DEC_VALUE;
			expr[index - 1].token = std::to_string(EvalNum(std::stof(expr[index - 1].token),
				std::stof(expr[index + 1].token), op));
		}
		else
		{
			throw Error(night::_invalid_expression_, expr, index - 1, index + 1,
				"operator '" + expr[index].token + "' can only be used on two values of type 'int' or "
				"'dec', or two values of type 'str'");
		}
	}
	catch (const Error&) {
		throw;
	}
	catch (const char*) { // div by 0
		throw Error(night::_invalid_expression_, expr, index - 1, index + 1,
			"operator '/' cannot divide by zero");
	}
}

void CheckIndex(const std::vector<Token>& expr, std::size_t index, bool checkLeft = true)
{
	bool rightIndexError = false;
	try {
		if (checkLeft)
			Token temp = expr.at(index - 1);

		rightIndexError = true;

		Token temp = expr.at(index + 1);
	}
	catch (const std::out_of_range&) {
		if (rightIndexError) {
			throw Error(night::_invalid_expression_, expr, index, index,
				"expected value after operator '" + expr[index].token + "'");
		}
		
		throw Error(night::_invalid_expression_, expr, index, index,
			"expected value before operator '" + expr[index].token + "'");
	}
}

void CompareValues(const std::vector<Token>& expr, std::size_t index, const std::string& op)
{
	if (expr[index - 1].type != expr[index + 1].type) {
		throw Error(night::_invalid_expression_, expr, index - 1, index + 1,
			"operator '" + op + "' cannot compare two tokens of different types");
	}
	if (expr[index - 1].type > TokenType::STR_VALUE) {
		throw Error(night::_invalid_expression_, expr, index - 1, index - 1,
			"operator '" + op + "' cannot compare token '" + expr[index - 1].token + "'; "
			"it can only compare values of type 'bit', 'syb', 'int', 'dec', or 'str'");
	}
	if (expr[index + 1].type > TokenType::STR_VALUE) {
		throw Error(night::_invalid_expression_, expr, index + 1, index + 1,
			"operator '" + op + "' cannot compare token '" + expr[index + 1].token + "'; "
			"it can only compare values of type 'bit', 'syb', 'int', 'dec', or 'str'");
	}
}

void EvalExpr(std::vector<Token>& expr, std::size_t& index)
{
	CheckIndex(expr, index);

	switch (expr[index].type)
	{
	case TokenType::PLUS:
		if (expr[index - 1].type == TokenType::STR_VALUE && expr[index - 1].type == expr[index + 1].type)
		{
			expr[index - 1].token = expr[index - 1].token + expr[index + 1].token;
			break;
		}
		else if (expr[index - 1].type == TokenType::STR_VALUE &&
			expr[index - 1].type != expr[index + 1].type)
		{
			throw Error(night::_invalid_expression_, expr, index + 1, index + 1,
				"value '" + expr[index + 1].token + "' must be a value of type 'str'; "
				"string concatenation only works for two values of type 'str'");
		}
		else if (expr[index + 1].type == TokenType::STR_VALUE &&
			expr[index + 1].type != expr[index - 1].type)
		{
			throw Error(night::_invalid_expression_, expr, index - 1, index - 1,
				"value '" + expr[index - 1].token + "' must be a value of type 'str'; "
				"string concatenation only works for two values of type 'str'");
		}

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
		if (expr[index - 1].type != TokenType::INT_VALUE) {
			throw Error(night::_invalid_expression_, expr, index - 1, index + 1,
				"value '" + expr[index - 1].token + "' must be of 'int' type");
		}
		if (expr[index + 1].type != TokenType::INT_VALUE) {
			throw Error(night::_invalid_expression_, expr, index - 1, index + 1,
				"value '" + expr[index + 1].token + "' must be of 'int' type");
		}

		EvaluateNumeric(expr, index, "%");
		break;
	case TokenType::GREATER:
		EvaluateNumeric(expr, index, ">");
		night::val_to_bit(expr[index - 1]);
		break;
	case TokenType::SMALLER:
		EvaluateNumeric(expr, index, "<");
		night::val_to_bit(expr[index - 1]);
		break;
	case TokenType::GREATER_EQUAL:
		EvaluateNumeric(expr, index, ">=");
		night::val_to_bit(expr[index - 1]);
		break;
	case TokenType::SMALLER_EQUAL:
		EvaluateNumeric(expr, index, "<=");
		night::val_to_bit(expr[index - 1]);
		break;
	case TokenType::EQUALS:
		CompareValues(expr, index, "==");
		expr[index - 1].token = night::bit_to_str(expr[index - 1].token == expr[index + 1].token);
		expr[index - 1].type = TokenType::BIT_VALUE;
		break;
	case TokenType::NOT_EQUALS:
		CompareValues(expr, index, "!=");
		expr[index - 1].token = night::bit_to_str(expr[index - 1].token != expr[index + 1].token);
		expr[index - 1].type = TokenType::BIT_VALUE;
		break;
	case TokenType::OR:
		if (expr[index - 1].type != TokenType::BIT_VALUE) {
			throw Error(night::_invalid_expression_, expr, index - 1, index + 1,
				"value '" + expr[index - 1].token + "' must be of 'int' type");
		}
		if (expr[index + 1].type != TokenType::BIT_VALUE) {
			throw Error(night::_invalid_expression_, expr, index - 1, index + 1,
				"value '" + expr[index + 1].token + "' must be of 'int' type");
		}

		expr[index - 1].token = night::bit_to_str(night::str_to_bit(expr[index - 1]) ||
			night::str_to_bit(expr[index + 1]));
		break;
	case TokenType::AND:
		if (expr[index - 1].type != TokenType::BIT_VALUE) {
			throw Error(night::_invalid_expression_, expr, index - 1, index + 1,
				"value '" + expr[index - 1].token + "' must be of 'bit' type");
		}
		if (expr[index + 1].type != TokenType::BIT_VALUE) {
			throw Error(night::_invalid_expression_, expr, index - 1, index + 1,
				"value '" + expr[index + 1].token + "' must be of 'bit' type");
		}

		expr[index - 1].token = night::bit_to_str(night::str_to_bit(expr[index - 1]) &&
			night::str_to_bit(expr[index + 1]));
		break;
	}

	expr.erase(expr.begin() + index);
	expr.erase(expr.begin() + index);

	index -= 1;
}

Token EvaluateExpression(std::vector<Token>& expr, const std::vector<Variable>& vars,
	const std::vector<Function>& funcs)
{
	for (std::size_t a = 0; a < expr.size(); ++a)
	{
		if (expr[a].type == TokenType::VARIABLE)
		{
			bool isDefined = false;
			for (const Variable& var : vars)
			{
				if (expr[a].token == var.name)
				{
					expr[a].type = var.type;
					expr[a].token = var.value;

					isDefined = true;
					break;
				}
			}
		
			if (!isDefined)
			{
				for (const Function& func : funcs)
				{
					if (expr[a].token == func.name)
					{
						int end = a;
						std::vector<Token> temp;
						for (std::size_t b = a; expr[b].type != TokenType::CLOSE_BRACKET; ++b)
						{
							temp.push_back(expr[b]);
							end = b;
						}

						temp.push_back(Token{ TokenType::CLOSE_BRACKET, ")" });
						temp.push_back(Token{ TokenType::SEMICOLON, ";" });
						
						expr.erase(expr.begin() + a, expr.begin() + end + 1);

						Parser(temp, false, false);

						expr[a] = temp[0];

						isDefined = true;
						break;
					}
				}

				if (!isDefined) {
					throw Error(night::_undefined_token_, expr, a, a,
						"variable '" + expr[a].token + "' is not defined");
				}
			}
		}
	}

	int openBracketIndex = 0;
	for (std::size_t a = 0; a < expr.size(); ++a)
	{
		if (expr[a].type == TokenType::OPEN_BRACKET)
		{
			openBracketIndex = a;
		}
		else if (expr[a].type == TokenType::CLOSE_BRACKET)
		{
			for (std::size_t b = openBracketIndex + 1; b < a; ++b)
			{
				if (expr[b].type == TokenType::NOT)
				{
					CheckIndex(expr, b, false);

					if (expr[b + 1].type != TokenType::BIT_VALUE) {
						throw Error(night::_invalid_expression_, expr, b, b + 1,
							"operator '!' can only be performed on a value of type 'bit'");
					}

					expr[b + 1].token = night::bit_to_str(!night::str_to_bit(expr[b + 1]));
					expr.erase(expr.begin() + b);

					b -= 1;
					a -= 1;
				}
			}

			for (std::size_t b = openBracketIndex + 1; b < a; ++b)
			{
				if (expr[b].type >= TokenType::DIVIDE && expr[b].type <= TokenType::MOD)
				{
					EvalExpr(expr, b);
					a -= 2;
				}
			}

			for (std::size_t b = openBracketIndex + 1; b < a; ++b)
			{
				if (expr[b].type >= TokenType::PLUS && expr[b].type <= TokenType::MINUS)
				{
					EvalExpr(expr, b);
					a -= 2;
				}
			}

			for (std::size_t b = openBracketIndex + 1; b < a; ++b)
			{
				if (expr[b].type >= TokenType::GREATER && expr[b].type <= TokenType::SMALLER_EQUAL)
				{
					EvalExpr(expr, b);
					a -= 2;
				}
			}

			for (std::size_t b = openBracketIndex + 1; b < a; ++b)
			{
				if (expr[b].type >= TokenType::EQUALS && expr[b].type <= TokenType::NOT_EQUALS)
				{
					EvalExpr(expr, b);
					a -= 2;
				}
			}

			for (std::size_t b = openBracketIndex + 1; b < a; ++b)
			{
				if (expr[b].type == TokenType::AND)
				{
					EvalExpr(expr, b);
					a -= 2;
				}
			}

			for (std::size_t b = openBracketIndex + 1; b < a; ++b)
			{
				if (expr[b].type == TokenType::OR)
				{
					EvalExpr(expr, b);
					a -= 2;
				}
			}

			expr.erase(expr.begin() + openBracketIndex);
			expr.erase(expr.begin() + a - 1);

			a = -1;
		}
	}

	for (std::size_t a = 0; a < expr.size(); ++a)
	{
		if (expr[a].type == TokenType::NOT)
		{
			CheckIndex(expr, a, false);

			if (expr[a + 1].type != TokenType::BIT_VALUE) {
				throw Error(night::_invalid_expression_, expr, a, a + 1,
					"operator '!' can only be performed on a value of type 'bit'");
			}

			expr[a + 1].token = night::bit_to_str(!night::str_to_bit(expr[a + 1]));
			expr.erase(expr.begin() + a);

			a -= 1;
		}
	}

	for (std::size_t a = 0; a < expr.size(); ++a)
	{
		if (expr[a].type >= TokenType::DIVIDE && expr[a].type <= TokenType::MOD)
			EvalExpr(expr, a);
	}

	for (std::size_t a = 0; a < expr.size(); ++a)
	{
		if (expr[a].type >= TokenType::PLUS && expr[a].type <= TokenType::MINUS)
			EvalExpr(expr, a);
	}

	for (std::size_t a = 0; a < expr.size(); ++a)
	{
		if (expr[a].type >= TokenType::GREATER && expr[a].type <= TokenType::SMALLER_EQUAL)
			EvalExpr(expr, a);
	}

	for (std::size_t a = 0; a < expr.size(); ++a)
	{
		if (expr[a].type >= TokenType::EQUALS && expr[a].type <= TokenType::NOT_EQUALS)
			EvalExpr(expr, a);
	}

	for (std::size_t a = 0; a < expr.size(); ++a)
	{
		if (expr[a].type == TokenType::AND)
			EvalExpr(expr, a);
	}

	for (std::size_t a = 0; a < expr.size(); ++a)
	{
		if (expr[a].type == TokenType::OR)
			EvalExpr(expr, a);
	}

	if (expr.size() != 1) {
		throw Error(night::_invalid_expression_, expr, 0, expr.size() - 1,
			"expression is invalid; expected operators in between values");
	}

	return expr[0];
}