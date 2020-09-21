#pragma once

#include "../lib/string.h"
#include "../lib/array.h"
#include "../lib/error.h"

#include "../containers/variable.h"
#include "../containers/function.h"
#include "../containers/array_var.h"
#include "../containers/token.h"

#include "./helpers.h"

void Interpreter(night::array<night::array<Token> >& code);
void SplitCode(const night::array<Token>& code);

Token returnToken{ TokenType::NULL_TYPE };
int variableScope = -1, arrayScope = -1;

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

Token ParseExpression(night::array<Token> expr, night::array<Variable>& vars,
	night::array<Function>& funcs, night::array<Array>& arrs)
{
	for (int a = 0; a < expr.length(); ++a)
	{
		if (expr[a].type == TokenType::VARIABLE)
		{
			Variable* variable = GetObject(vars, expr[a]);
			if (variable != nullptr)
			{
				expr[a] = Token{ night::ttov(variable->type), variable->value };
				continue;
			}

			Function* function = GetObject(funcs, expr[a]);
			if (function != nullptr)
			{
				if (function->type == TokenType::NULL_TYPE)
					throw Error(night::_invalid_expression_, expr, a, a, "function of type 'null' cannot be used in an expression");

				variableScope = vars.length();

				int closeBracket = 0;
				night::array<Token> paramExpr;
				for (int b = a + 2, paramIndex = 0, openBracket = 0; b < expr.length(); ++b)
				{
					if (expr[b].type == TokenType::OPEN_BRACKET)
						openBracket++;
					else if (expr[b].type == TokenType::CLOSE_BRACKET)
						openBracket--;

					if ((expr[b].type == TokenType::COMMA && openBracket == 0) ||
						(expr[b].type == TokenType::CLOSE_BRACKET && openBracket == -1))
					{
						if (paramIndex >= function->params.length())
							throw Error(night::_invalid_function_, expr, 0, expr.length(), "expecting "_s + night::itos(function->params.length()) + " parameters");

						Token paramExprToken = ParseExpression(paramExpr, vars, funcs, arrs);
						if (night::ttov(function->params[paramIndex].type) != paramExprToken.type)
							throw Error(night::_invalid_function_, expr, 0, expr.length(), "parameter expression does not match with parameter type");

						vars.push_back(Variable{
							function->params[paramIndex].type,
							function->params[paramIndex].name,
							paramExprToken.value
						});

						paramIndex++;
						paramExpr.clear();

						closeBracket = b;
						if (expr[b].type == TokenType::CLOSE_BRACKET)
							break;

						continue;
					}

					paramExpr.push_back(expr[b]);
				}

				arrayScope = arrs.length();

				SplitCode(function->code);

				expr[a] = returnToken;
				returnToken.type = TokenType::NULL_TYPE;

				for (int b = a + 1; b <= closeBracket; ++b)
					expr.remove(a + 1);

				int variableLength = vars.length();
				for (int b = variableScope; b < variableLength; ++b)
					vars.remove(variableScope);

				continue;
			}

			Array* array = GetObject(arrs, expr[a]);
			if (array == nullptr)
				throw Error(night::_undefined_object_, expr, a, a, "object is not defined");

			for (int b = a + 3, squareCount = 0; ; ++b)
			{
				if (expr[b].type == TokenType::OPEN_SQUARE)
					squareCount++;
				else if (expr[b].type == TokenType::CLOSE_SQUARE)
					squareCount--;

				if (expr[b].type == TokenType::CLOSE_SQUARE && squareCount == -1)
				{
					Token arrayIndexToken = ParseExpression(expr.access(a + 2, b - 1),
						vars, funcs, arrs);
					int arrayIndex = night::stoi(arrayIndexToken.value);

					if (arrayIndexToken.type != TokenType::INT_VALUE)
						throw Error(night::_invalid_array_, expr, a + 3, b - 1, "array index can only be a value of type 'int'");
					if (arrayIndex < 0 || arrayIndex >= array->elems.length())
						throw Error(night::_invalid_array_, expr, a + 3, b - 1, "array index is out of range; the last element is at index '"_s + night::itos(array->elems.length() - 1) + "'");

					expr[a] = array->elems[arrayIndex];
					for (int c = a + 1; c <= b; ++c)
						expr.remove(a + 1);

					break;
				}
			}
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