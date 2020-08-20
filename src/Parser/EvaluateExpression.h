<<<<<<< HEAD
#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "Parser.h"

#include "DataTypes/Error.h"
#include "DataTypes/Token.h"
#include "DataTypes/Variable.h"
#include "DataTypes/Function.h"

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

TokenType type_to_val(const Variable& val)
{
	if (val.type == TokenType::BIT_TYPE)
		return TokenType::BIT_VALUE;
	else if (val.type == TokenType::SYB_TYPE)
		return TokenType::SYB_VALUE;
	else if (val.type == TokenType::INT_TYPE)
		return TokenType::INT_VALUE;
	else if (val.type == TokenType::DEC_TYPE)
		return TokenType::DEC_VALUE;
	else
		return TokenType::STR_VALUE;
}

float abs(float val)
{
	return val < 0 ? -val : val;
}

} // namespace night

void Parser(std::vector<Token>& tokens, bool runtime, bool recursion = false);
void ExtractLine(std::vector<Token>& tokens, Token* returnType = nullptr,
	std::vector<Variable>* vars = nullptr, std::vector<Function>* funcs = nullptr, int omg = 0);

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
	std::string rightIndexError = "before";
	try {
		if (checkLeft)
			Token temp = expr.at(index - 1);

		rightIndexError = "after";

		Token temp = expr.at(index + 1);
	}
	catch (const std::out_of_range&) {
		throw Error(night::_invalid_expression_, expr, index, index,
			"expected value " + rightIndexError + " operator '" + expr[index].token + "'");
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

void EvalExpr(std::vector<Token>& expr, std::size_t& index, std::size_t* bracketIndex = nullptr)
{
	CheckIndex(expr, index);

	switch (expr[index].type)
	{
	case TokenType::PLUS:
		if (expr[index - 1].type == TokenType::STR_VALUE && expr[index + 1].type <= TokenType::STR_VALUE)
			//expr[index - 1].type == expr[index + 1].type)
		{
			expr[index - 1].token += expr[index + 1].token;
			break;
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

	if (bracketIndex != nullptr)
		*bracketIndex -= 2;
}

Token EvaluateExpression(std::vector<Token>& expr, std::vector<Variable>& vars,
	std::vector<Function>& funcs)
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
					expr[a].type = night::type_to_val(var);
					expr[a].token = var.value;

					isDefined = true;
					break;
				}
			}
		
			if (!isDefined)
			{
				for (Function& func : funcs)
				{
					if (expr[a].token == func.name)
					{
						try {
							if (expr.at(a + 1).type != TokenType::OPEN_BRACKET)
								throw 1;
						}
						catch (...) {
							break;
						}

						std::vector<Token> temp;
						int functionVariable = 0, openBracket = 0, end = a + 2;
						for (std::size_t b = a + 2; expr[b].type != TokenType::CLOSE_BRACKET; ++b)
						{
							if (expr[b].type == TokenType::OPEN_BRACKET)
								openBracket += 1;
							else if (expr[b].type == TokenType::CLOSE_BRACKET)
								openBracket -= 1;

							if (expr[b].type != TokenType::COMMA && expr[b].type != TokenType::CLOSE_BRACKET)
							{
								temp.push_back(expr[b]);
							}
							else
							{
								if (openBracket != 0 && b != expr.size() - 2)
								{
									temp.push_back(expr[b]);
									continue;
								}

								if (expr[b].type == TokenType::CLOSE_BRACKET && b != expr.size() - 2)
									temp.push_back(expr[b]);

								try {
									vars.push_back(Variable{
										func.parameters.at(functionVariable).type,
										func.parameters[functionVariable].name,
										EvaluateExpression(temp, vars, funcs).token
									});

									functionVariable += 1;
								}
								catch (const Error& e) {
									throw Error(night::_invalid_expression_, expr, 0, 0, "");
								}
								catch (...) {
									end = b;
									break;
								}

								temp.clear();
							}

							end = b;
						}

						if (!temp.empty())
						{
							vars.push_back(Variable{
								func.parameters.at(functionVariable).type,
								func.parameters[functionVariable].name,
								EvaluateExpression(temp, vars, funcs).token
							});
						}

						ExtractLine(func.code, &expr[a], &vars, &funcs);

						expr.erase(expr.begin() + a + 1, expr.begin() + end + (temp.empty() ? 1 : 2));

						isDefined = true;
						break;
					}
				}

				if (!isDefined) {
					throw Error(night::_undefined_token_, expr, a, a,
						"token '" + expr[a].token + "' is not defined");
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
					EvalExpr(expr, b, &a);
			}

			for (std::size_t b = openBracketIndex + 1; b < a; ++b)
			{
				if (expr[b].type >= TokenType::PLUS && expr[b].type <= TokenType::MINUS)
					EvalExpr(expr, b, &a);
			}

			for (std::size_t b = openBracketIndex + 1; b < a; ++b)
			{
				if (expr[b].type >= TokenType::GREATER && expr[b].type <= TokenType::SMALLER_EQUAL)
					EvalExpr(expr, b, &a);
			}

			for (std::size_t b = openBracketIndex + 1; b < a; ++b)
			{
				if (expr[b].type >= TokenType::EQUALS && expr[b].type <= TokenType::NOT_EQUALS)
					EvalExpr(expr, b, &a);
			}

			for (std::size_t b = openBracketIndex + 1; b < a; ++b)
			{
				if (expr[b].type == TokenType::AND)
					EvalExpr(expr, b, &a);
			}

			for (std::size_t b = openBracketIndex + 1; b < a; ++b)
			{
				if (expr[b].type == TokenType::OR)
					EvalExpr(expr, b, &a);
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
=======
#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "Parser.h"

#include "../DataTypes/Error.h"
#include "../DataTypes/Token.h"
#include "../DataTypes/Variable.h"
#include "../DataTypes/Function.h"

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

TokenType type_to_val(const Variable& val)
{
	if (val.type == TokenType::BIT_TYPE)
		return TokenType::BIT_VALUE;
	else if (val.type == TokenType::SYB_TYPE)
		return TokenType::SYB_VALUE;
	else if (val.type == TokenType::INT_TYPE)
		return TokenType::INT_VALUE;
	else if (val.type == TokenType::DEC_TYPE)
		return TokenType::DEC_VALUE;
	else if (val.type == TokenType::STR_TYPE)
		return TokenType::STR_VALUE;
	else
		std::cout << "ERROR - line 45";
	return val.type;
}

float abs(float val)
{
	return val < 0 ? -val : val;
}

} // namespace night

void Parser(std::vector<Token>& tokens, bool runtime, bool recursion = false);
void ExtractLine(std::vector<Token>& tokens, Token* returnType = nullptr,
	std::vector<Variable>* vars = nullptr, std::vector<Function>* funcs = nullptr, int omg = 0);

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
	std::string rightIndexError = "before";
	try {
		if (checkLeft)
			Token temp = expr.at(index - 1);

		rightIndexError = "after";

		Token temp = expr.at(index + 1);
	}
	catch (const std::out_of_range&) {
		throw Error(night::_invalid_expression_, expr, index, index,
			"expected value " + rightIndexError + " operator '" + expr[index].token + "'");
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

void EvalExpr(std::vector<Token>& expr, std::size_t& index, std::size_t* bracketIndex = nullptr)
{
	CheckIndex(expr, index);

	switch (expr[index].type)
	{
	case TokenType::PLUS:
		if ((expr[index - 1].type == TokenType::STR_VALUE && expr[index + 1].type >= TokenType::BIT_VALUE && 
				expr[index + 1].type <= TokenType::STR_VALUE) ||
			(expr[index - 1].type >= TokenType::BIT_VALUE && expr[index - 1].type <= TokenType::STR_VALUE && 
				expr[index + 1].type == TokenType::STR_VALUE))
		{
			expr[index - 1].type = TokenType::STR_VALUE;
			expr[index - 1].token += expr[index + 1].token;
			break;
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

	if (bracketIndex != nullptr)
		*bracketIndex -= 2;
}

Token EvaluateExpression(std::vector<Token>& expr, std::vector<Variable>& vars,
	std::vector<Function>& funcs)
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
					expr[a].type = night::type_to_val(var);
					expr[a].token = var.value;

					isDefined = true;
					break;
				}
			}
		
			if (!isDefined)
			{
				for (Function& func : funcs)
				{
					if (expr[a].token == func.name)
					{
						try {
							if (expr.at(a + 1).type != TokenType::OPEN_BRACKET)
								throw 1;
						}
						catch (...) {
							break;
						}

						int localVariables = vars.size();

						std::vector<Token> temp;
						int functionVariable = 0, openBracket = 0, end = a + 2;
						for (std::size_t b = a + 2; expr[b].type != TokenType::CLOSE_BRACKET; ++b)
						{
							if (expr[b].type == TokenType::OPEN_BRACKET)
								openBracket += 1;
							else if (expr[b].type == TokenType::CLOSE_BRACKET)
								openBracket -= 1;

							if (expr[b].type != TokenType::COMMA && expr[b].type != TokenType::CLOSE_BRACKET)
							{
								temp.push_back(expr[b]);
							}
							else
							{
								if (openBracket != 0 && b != expr.size() - 2)
								{
									temp.push_back(expr[b]);
									continue;
								}

								if (expr[b].type == TokenType::CLOSE_BRACKET && b != expr.size() - 2)
									temp.push_back(expr[b]);

								try {
									vars.push_back(Variable{
										func.parameters.at(functionVariable).type,
										func.parameters[functionVariable].name,
										EvaluateExpression(temp, vars, funcs).token
									});

									functionVariable += 1;
								}
								catch (const Error&) {
									throw Error(night::_invalid_expression_, expr, 0, 0, "");
								}
								catch (...) {
									end = b;
									break;
								}

								temp.clear();
							}

							end = b;
						}

						if (!temp.empty())
						{
							vars.push_back(Variable{
								func.parameters.at(functionVariable).type,
								func.parameters[functionVariable].name,
								EvaluateExpression(temp, vars, funcs).token
							});
						}

						ExtractLine(func.code, &expr[a], &vars, &funcs);

						expr.erase(expr.begin() + a + 1, expr.begin() + end + (temp.empty() ? 1 : 2));

						vars.erase(vars.begin() + localVariables, vars.end());

						isDefined = true;
						break;
					}
				}

				if (!isDefined) {
					throw Error(night::_undefined_token_, expr, a, a,
						"token '" + expr[a].token + "' is not defined");
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
					EvalExpr(expr, b, &a);
			}

			for (std::size_t b = openBracketIndex + 1; b < a; ++b)
			{
				if (expr[b].type >= TokenType::PLUS && expr[b].type <= TokenType::MINUS)
					EvalExpr(expr, b, &a);
			}

			for (std::size_t b = openBracketIndex + 1; b < a; ++b)
			{
				if (expr[b].type >= TokenType::GREATER && expr[b].type <= TokenType::SMALLER_EQUAL)
					EvalExpr(expr, b, &a);
			}

			for (std::size_t b = openBracketIndex + 1; b < a; ++b)
			{
				if (expr[b].type >= TokenType::EQUALS && expr[b].type <= TokenType::NOT_EQUALS)
					EvalExpr(expr, b, &a);
			}

			for (std::size_t b = openBracketIndex + 1; b < a; ++b)
			{
				if (expr[b].type == TokenType::AND)
					EvalExpr(expr, b, &a);
			}

			for (std::size_t b = openBracketIndex + 1; b < a; ++b)
			{
				if (expr[b].type == TokenType::OR)
					EvalExpr(expr, b, &a);
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
>>>>>>> test1
}