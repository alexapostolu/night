#pragma once

#include "../lib/string.h"
#include "../lib/array.h"
#include "../lib/error.h"

#include "../containers/variable.h"
#include "../containers/function.h"
#include "../containers/array_var.h"
#include "../containers/token.h"

#include "./expr_parser.h"
#include "./helpers.h"

#include "../Output.h"

#define CONTAINERS variables, functions, arrays

bool IsType(const Token& token)
{
	return token.type >= TokenType::BIT_TYPE && token.type <= TokenType::STR_TYPE;
}

bool IsFuncType(const Token& token)
{
	return token.type >= TokenType::BIT_TYPE && token.type <= TokenType::NULL_TYPE;
}

bool IsVar(const Token& token)
{
	return token.type == TokenType::VARIABLE;
}

bool IsAssign(const Token& token)
{
	return token.type == TokenType::ASSIGNMENT;
}

bool IsAssignExpr(const Token& token)
{
	return token.type >= TokenType::ASSIGNMENT && token.type <= TokenType::MOD_ASSIGN;
}

bool IsOpenBracket(const Token& token)
{
	return token.type == TokenType::OPEN_BRACKET;
}

bool IsOpenSquare(const Token& token)
{
	return token.type == TokenType::OPEN_SQUARE;
}

night::string DefaultValue(const TokenType& type)
{
	switch (type)
	{
	case TokenType::BIT_TYPE:
		return "false";
	case TokenType::SYB_TYPE:
		return " ";
	case TokenType::INT_TYPE:
		return "0";
	case TokenType::DEC_TYPE:
		return "0.0";
	default:
		return "";
	}
}

void Interpreter(night::array<night::array<Token> >& code)
{
	static night::array<Variable> variables;
	static night::array<Function> functions;
	static night::array<Array> arrays;

	static bool predefined = false;
	if (!predefined)
	{
		functions.push_back(Function{ TokenType::NULL_TYPE, "print" });
		predefined = true;
	}

	for (int a = 0; a < code.length(); ++a)
	{
		if (returnToken.type != TokenType::NULL_TYPE)
			break;

		if (code[a].length() == 3 && IsType(code[a][0]) && IsVar(code[a][1]))
		{
			if (GetObject(variables, code[a][1]) != nullptr)
				throw Error(night::_redefined_object_, code[a], 1, 1, "variable is already defined");
			if (GetObject(functions, code[a][1]) != nullptr)
				throw Error(night::_redefined_object_, code[a], 1, 1, "variable cannot have the same name as a function");
			if (GetObject(arrays, code[a][1]) != nullptr)
				throw Error(night::_redefined_object_, code[a], 1, 1, "variable cannot have the same name as an array");

			variables.push_back(Variable{
				code[a][0].type,
				code[a][1].value,
				DefaultValue(code[a][0].type)
			});
		}
		else if (code[a].length() >= 5 && IsType(code[a][0]) && IsAssign(code[a][2]))
		{
			if (GetObject(variables, code[a][1]) != nullptr)
				throw Error(night::_redefined_object_, code[a], 1, 1, "variable is already defined");
			if (GetObject(functions, code[a][1]) != nullptr)
				throw Error(night::_redefined_object_, code[a], 1, 1, "variable cannot have the same name as a function");
			if (GetObject(arrays, code[a][1]) != nullptr)
				throw Error(night::_redefined_object_, code[a], 1, 1, "variable cannot have the same name as an array");

			Token evalExpr = ParseExpression(code[a].access(3, code[a].length() - 1), CONTAINERS);
			if (code[a][0].type == TokenType::INT_TYPE && evalExpr.type == TokenType::DEC_VALUE)
				evalExpr.value = night::stoi(evalExpr.value);
			else if (code[a][0].type == TokenType::DEC_TYPE && evalExpr.type == TokenType::INT_VALUE)
				evalExpr.value = night::stof(evalExpr.value);
			else if (night::ttov(code[a][0].type) != evalExpr.type)
				throw Error(night::_invalid_variable_, code[a], 3, code.length(), "variable of type '" + night::ttos(code[a][0].type) + "' cannot be assigned with an expression of type '" + night::ttos(evalExpr.type) + "'");

			variables.push_back(Variable{
				code[a][0].type,
				code[a][1].value,
				evalExpr.value
			});
		}
		else if (code[a].length() >= 4 && IsAssignExpr(code[a][1]))
		{
			Variable* variable = GetObject(variables, code[a][0]);
			if (variable != nullptr)
			{
				Token evalExpr = ParseExpression(code[a].access(2, code[a].length() - 2), CONTAINERS);
				if (variable->type == TokenType::INT_TYPE && evalExpr.type == TokenType::DEC_VALUE)
					evalExpr.value = night::stoi(evalExpr.value);
				else if (variable->type == TokenType::DEC_TYPE && evalExpr.type == TokenType::INT_VALUE)
					evalExpr.value = night::stof(evalExpr.value);
				else if (night::ttov(variable->type) != evalExpr.type)
					throw Error(night::_invalid_variable_, code[a], 2, code[a].length() - 1, "variable of type '" + night::ttos(variable->type) + "' cannot be assigned with expression of type '" + night::ttos(evalExpr.type) + "'");

				if (code[a][1].type != TokenType::ASSIGNMENT)
				{
					night::array<Token> assignExpression;
					assignExpression.push_back(code[a][0]);
					switch (code[a][1].type)
					{
					case TokenType::PLUS_ASSIGN:
						assignExpression.push_back(Token{ TokenType::PLUS, "+" });
						break;
					case TokenType::MINUS_ASSIGN:
						assignExpression.push_back(Token{ TokenType::MINUS, "-" });
						break;
					case TokenType::TIMES_ASSIGN:
						assignExpression.push_back(Token{ TokenType::TIMES, "*" });
						break;
					case TokenType::DIVIDE_ASSIGN:
						assignExpression.push_back(Token{ TokenType::DIVIDE, "/" });
						break;
					case TokenType::MOD_ASSIGN:
						assignExpression.push_back(Token{ TokenType::MOD, "%" });
						break;
					}

					assignExpression.push_back(evalExpr);
					variable->value = ParseExpression(assignExpression, CONTAINERS).value;

					continue;
				}

				variable->value = evalExpr.value;
				continue;
			}

			Array* array = GetObject(arrays, code[a][0]);
			if (array == nullptr)
				throw Error(night::_undefined_object_, code[a], 0, 0, "object is not defined");

			if (code[a][1].type != TokenType::ASSIGNMENT)
				throw Error(night::_invalid_array_, code[a], 1, 1, "invalid operation on an array");

			if (code[a].length() == 4)
			{
				Array* assignArray = GetObject(arrays, code[a][2]);
				if (assignArray == nullptr)
					throw Error(night::_invalid_array_, code[a], 2, 2, "array can only be assigned to other arrays");
				if (array->type != assignArray->type)
					throw Error(night::_invalid_array_, code[a], 2, 2, "array must be of type '" + night::ttos(array->type) + "'");

				*array = *assignArray;
				continue;
			}

			array->elems.clear();

			night::array<Token> elementExpression;
			for (int b = 3, elementIndex = 0, bracketCount = 0; b < code[a].length() - 2; ++b)
			{
				if (code[a][b].type == TokenType::OPEN_BRACKET)
				{
					bracketCount++;
				}
				else if (code[a][b].type == TokenType::CLOSE_BRACKET)
				{
					bracketCount--;
				}
				else if ((code[a][b].type == TokenType::COMMA && bracketCount == 0) ||
					code[a][b + 2].type == TokenType::SEMICOLON)
				{
					if (code[a][b + 2].type == TokenType::SEMICOLON)
						elementExpression.push_back(code[a][b]);

					Token evaluateElement = ParseExpression(elementExpression, CONTAINERS);
					if (evaluateElement.type != night::ttov(array->type))
						throw Error(night::_invalid_array_, code[a], b, b, "array of type '" + night::ttos(array->type) + "' cannot be assigned to array of type '" + night::ttos(evaluateElement.type) + "'");

					array->elems.push_back(evaluateElement);
					elementExpression.clear();

					continue;
				}

				elementExpression.push_back(code[a][b]);
			}
		}
		else if (code[a].length() >= 6 && code[a][0].type == TokenType::IF)
		{
			int bracketIndex = 2;
			for (; code[a][bracketIndex].type != TokenType::CLOSE_BRACKET; ++bracketIndex);

			Token condition = ParseExpression(code[a].access(2, bracketIndex - 1), CONTAINERS);
			if (condition.type != TokenType::BIT_VALUE)
				throw Error(night::_invalid_statement_, code[a], 2, bracketIndex, "if statement condition must evaluate to a value of type 'bit'");

			if (condition.value == "true")
			{
				code[a][0].value[0] = 'c';
				SplitCode(code[a].access(bracketIndex + 2, code[a].length() - 2));
			}
		}
		else if (code[a].length() >= 7 && code[a][1].type == TokenType::IF)
		{
			if (code.length() == 0 || (code[a - 1][0].type != TokenType::IF &&
				code[a - 1][1].type != TokenType::IF))
				throw Error(night::_invalid_statement_, code[a], 0, 1, "else if statement must come after an if statement");

			int bracketIndex = 3;
			for (; code[a][bracketIndex].type != TokenType::CLOSE_BRACKET; ++bracketIndex);

			Token condition = ParseExpression(code[a].access(3, bracketIndex - 1), CONTAINERS);
			if (condition.type != TokenType::BIT_VALUE)
				throw Error(night::_invalid_statement_, code[a], 2, bracketIndex, "else if statement condition must evaluate to a value of type 'bit'");

			if (condition.value == "true" && code[a - 1][0].value[0] != 'c')
			{
				code[a][0].value[0] = 'c';
				SplitCode(code[a].access(bracketIndex + 2, code[a].length() - 2));
			}
		}
		else if (code[a].length() >= 3 && code[a][0].type == TokenType::ELSE)
		{
			if (code.length() == 0 || (code[a - 1][0].type != TokenType::IF &&
				code[a - 1][1].type != TokenType::IF))
				throw Error(night::_invalid_statement_, code[a], 0, 1, "else statement must come after an if statement or an else if statement");

			if (code[a - 1][0].value[0] != 'c')
				SplitCode(code[a].access(2, code[a].length() - 2));
		}
		else if (code[a].length() >= 6 && IsFuncType(code[a][0]) && IsVar(code[a][1]) && IsOpenBracket(code[a][2]))
		{
			if (GetObject(variables, code[a][1]) != nullptr)
				throw Error(night::_redefined_object_, code[a], 1, 1, "function cannot have the same name as a variable");
			if (GetObject(functions, code[a][1]) != nullptr)
				throw Error(night::_redefined_object_, code[a], 1, 1, "function is already defined");
			if (GetObject(arrays, code[a][1]) != nullptr)
				throw Error(night::_redefined_object_, code[a], 1, 1, "function cannot have the same name as an array");

			functions.push_back(Function{ code[a][0].type, code[a][1].value,
				night::array<Variable>(), night::array<Token>() });

			int b = 3;
			for (; code[a][b].type != TokenType::CLOSE_BRACKET; b += 2)
			{
				if (code[a][b].type == TokenType::COMMA)
					b++;

				functions.back().params.push_back(Variable{ code[a][b].type, code[a][b + 1].value });
			}

			bool findReturn = false;
			for (b += 2; b < code[a].length() - 1; ++b)
			{
				if (code[a][b].type == TokenType::RETURN)
				{
					if (code[a][0].type == TokenType::NULL_TYPE)
						throw Error(night::_invalid_expression_, code[a], b, b, "function of type 'null' cannot return a value");

					findReturn = true;
				}

				functions.back().code.push_back(code[a][b]);
			}

			if (code[a][0].type != TokenType::NULL_TYPE && !findReturn)
				throw Error(night::_invalid_function_, code[a], 0, 1, "function must return a value");
		}
		else if (code[a].length() >= 4 && IsVar(code[a][0]) && IsOpenBracket(code[a][1]))
		{
			if (code[a][0].value == "print")
			{
				StoreOutput(ParseExpression(code[a].access(2, code[a].length() - 3), CONTAINERS).value);
				continue;
			}

			Function* function = GetObject(functions, code[a][0]);
			if (function == nullptr)
				throw Error(night::_invalid_function_, code[a], 0, 0, "function is not defined");

			int variableScope = variables.length();

			int openBracket = 0, paramIndex = 0;
			night::array<Token> paramExpr;
			for (int b = 2; b < code[a].length() && function->params.length() != 0; ++b)
			{
				if (code[a][b].type == TokenType::OPEN_BRACKET)
					openBracket++;
				else if (code[a][b].type == TokenType::CLOSE_BRACKET)
					openBracket--;

				if ((openBracket == 0 && code[a][b].type == TokenType::COMMA) ||
					(openBracket == -1 && code[a][b].type == TokenType::CLOSE_BRACKET))
				{
					if (paramIndex >= function->params.length())
						throw Error(night::_invalid_function_, code[a], 0, code[a].length(), "expecting " + night::itos(function->params.length()) + " parameters");

					Token paramExprVal = ParseExpression(paramExpr, CONTAINERS);
					if (night::ttov(function->params[paramIndex].type) != paramExprVal.type)
						throw Error(night::_invalid_function_, code[a], 0, code[a].length(), "parameter does not match with type");

					variables.push_back(Variable{
						function->params[paramIndex].type,
						function->params[paramIndex].name,
						paramExprVal.value
					});

					paramExpr = night::array<Token>();

					paramIndex++;
					continue;
				}

				paramExpr.push_back(code[a][b]);
			}

			if (paramIndex != function->params.length())
				throw Error(night::_invalid_function_, code[a], 0, code[a].length(), "expecting " + night::itos(function->params.length()) + " parameters");

			SplitCode(function->code);

			int endVariable = variables.length();
			for (int b = variableScope; b < endVariable; ++b)
				variables.remove(variableScope);
		}
		else if (code[a].length() >= 7 && code[a][1].type == TokenType::WHILE)
		{
			int bracketIndex = 4;
			for (; code[a][bracketIndex + 1].type != TokenType::OPEN_CURLY; ++bracketIndex);

			Token condition = ParseExpression(code[a].access(3, bracketIndex - 1), CONTAINERS);
			if (condition.type != TokenType::BIT_VALUE)
				throw Error(night::_invalid_statement_, code[a], 3, bracketIndex - 1, "while loop condition must evaluate to a value of type 'bit'");

			while (condition.value == "true")
			{
				SplitCode(code[a].access(bracketIndex + 2, code[a].length() - 2));
				condition = ParseExpression(code[a].access(3, bracketIndex - 1), CONTAINERS);
			}
		}
		else if (code[a].length() >= 7 && code[a][1].type == TokenType::FOR)
		{
			int bracketIndex = 4;
			for (; code[a][bracketIndex + 1].type != TokenType::OPEN_CURLY; ++bracketIndex);

			Token condition = ParseExpression(code[a].access(3, bracketIndex - 1), CONTAINERS);
			if (condition.type != TokenType::INT_VALUE)
				throw Error(night::_invalid_statement_, code[a], 3, bracketIndex - 1, "for loop condition must evaluate to a value of type 'int'");

			int loopFor = night::stoi(condition.value);
			for (int b = 0; b < loopFor; ++b)
				SplitCode(code[a].access(bracketIndex + 2, code[a].length() - 2));
		}
		else if (code[a].length() == 6 && IsType(code[a][0]) && IsOpenSquare(code[a][1]))
		{
			if (GetObject(variables, code[a][code[a].length() - 2]) != nullptr)
				throw Error(night::_redefined_object_, code[a], code[a].length() - 2, code[a].length() - 2, "array cannot have the same name as a variable");
			if (GetObject(functions, code[a][code[a].length() - 2]) != nullptr)
				throw Error(night::_redefined_object_, code[a], code[a].length() - 2, code[a].length() - 2, "array cannot have the same name as a function");
			if (GetObject(arrays, code[a][code[a].length() - 2]) != nullptr)
				throw Error(night::_redefined_object_, code[a], code[a].length() - 2, code[a].length() - 2, "array is already defined");

			arrays.push_back(Array{ code[a][0].type, code[a][code[a].length() - 2].value });

			int arrayLength = night::stoi(ParseExpression(code[a].access(2, code[a].length() - 4), CONTAINERS).value);
			for (int b = 0; b < arrayLength; ++b)
			{
				arrays.back().elems.push_back(Token{
					night::ttov(code[a][0].type),
					DefaultValue(code[a][0].type)
				});
			}
		}
		else if (code[a].length() >= 8 && IsType(code[a][0]) && IsOpenSquare(code[a][1]))
		{
			int closeSquare = 2;
			for (; code[a][closeSquare + 2].type != TokenType::ASSIGNMENT; ++closeSquare);

			if (GetObject(variables, code[a][closeSquare + 1]) != nullptr)
				throw Error(night::_redefined_object_, code[a], closeSquare + 1, closeSquare + 1, "array cannot have the same name as a variable");
			if (GetObject(functions, code[a][closeSquare + 1]) != nullptr)
				throw Error(night::_redefined_object_, code[a], closeSquare + 1, closeSquare + 1, "array cannot have the same name as a function");
			if (GetObject(arrays, code[a][closeSquare + 1]) != nullptr)
				throw Error(night::_redefined_object_, code[a], closeSquare + 1, closeSquare + 1, "array is already defined");

			if (code[a].length() == closeSquare + 4 && code[a][closeSquare + 3].type == TokenType::VARIABLE)
			{
				Array* assignArray = GetObject(arrays, code[a][2]);
				if (GetObject(arrays, code[a][2]) == nullptr)
					throw Error(night::_invalid_array_, code[a], closeSquare + 3, code[a].length() - 1, "arrays can only be assigned to other arrays");
				if (code[a][0].type != assignArray->type)
					throw Error(night::_invalid_array_, code[a], closeSquare + 3, closeSquare + 3, "array of type '" + night::ttos(code[a][0].type) + "' cannot be initialized with an array of type '" + night::ttos(assignArray->type) + "'");

				arrays.push_back(*assignArray);
				continue;
			}

			if (code[a][closeSquare + 3].type != TokenType::OPEN_SQUARE)
				throw Error(night::_invalid_array_, code[a], closeSquare + 3, code[a].length() - 1, "array can only be initialized to other arrays");

			arrays.push_back(Array{ code[a][0].type, code[a][closeSquare + 1].value });

			int arrayLength = code[a][closeSquare - 1].type != TokenType::OPEN_SQUARE
				? night::stoi(ParseExpression(code[a].access(2, closeSquare - 1), CONTAINERS).value)
				: -1;

			int elementIndex = 0, bracketCount = 0;
			night::array<Token> elementExpression;
			for (int b = closeSquare + 4; code[a][b + 1].type != TokenType::SEMICOLON; ++b)
			{
				if ((code[a][b].type == TokenType::COMMA && bracketCount == 0) ||
					code[a][b + 2].type == TokenType::SEMICOLON)
				{
					if (elementIndex == arrayLength)
						throw Error(night::_invalid_array_, code[a], closeSquare, code[a].length() - 1, "too many elements in array; expected " + night::itos(arrayLength) + " elements");

					if (code[a][b + 2].type == TokenType::SEMICOLON)
						elementExpression.push_back(code[a][b]);

					Token evaluateElement = ParseExpression(elementExpression, CONTAINERS);
					arrays.back().elems.push_back(evaluateElement);

					elementExpression.clear();
					continue;
				}
				else if (code[a][b].type == TokenType::OPEN_BRACKET)
				{
					bracketCount++;
				}
				else if (code[a][b].type == TokenType::CLOSE_BRACKET)
				{
					bracketCount--;
				}

				elementExpression.push_back(code[a][b]);
			}

			if (arrayLength != -1 && arrayLength >= arrays.back().elems.length())
			{
				int fillArrayLength = arrays.back().elems.length();
				for (int b = 0; b < arrayLength - fillArrayLength; ++b)
				{
					arrays.back().elems.push_back(Token{
						night::ttov(code[a][0].type),
						DefaultValue(code[a][0].type)
					});
				}
			}
		}
		else if (code[a].length() >= 7 && IsVar(code[a][0]) && IsOpenSquare(code[a][1]))
		{
			Array* array = GetObject(arrays, code[a][0]);
			if (array == nullptr)
				throw Error(night::_invalid_array_, code[a], 0, 0, "array is not defined");

			int closeSquare = 3;
			for (; code[a][closeSquare + 1].type != TokenType::ASSIGNMENT; ++closeSquare);

			Token arrayIndexToken = ParseExpression(code[a].access(2, closeSquare), CONTAINERS);
			int arrayIndex = night::stoi(arrayIndexToken.value);

			if (arrayIndexToken.type != TokenType::INT_VALUE)
				throw Error(night::_invalid_array_, code[a], 2, closeSquare, "array index can only be a value of type 'int'");
			if (arrayIndex < 0 || arrayIndex >= array->elems.length())
				throw Error(night::_invalid_array_, code[a], 2, closeSquare, "array index is out of range; the last element is at index '" + night::itos(array->elems.length() - 1) + "'");

			Token assignValue = ParseExpression(code[a].access(closeSquare + 2, code[a].length() - 2),
				CONTAINERS);
			if (assignValue.type != night::ttov(array->type))
				throw Error(night::_invalid_array_, code[a], closeSquare + 2, code[a].length() - 2, "array can only be assigned to an expression of type '" + night::ttos(array->type) + "'");

			array->elems[arrayIndex].value = assignValue.value;
		}
		else if (code[a].length() >= 3 && code[a][0].type == TokenType::RETURN)
		{
			for (int b = 1; b < code[a].length() - 1; ++b)
			{
				if (code[a][b].type == TokenType::VARIABLE)
				{
					Variable* variable = GetObject(variables, code[a][b]);
					if (variable != nullptr)
					{
						code[a][b] = Token{ night::ttov(variable->type), variable->value };
					}
					else
					{
						Array* array = GetObject(arrays, code[a][b]);
						if (array == nullptr)
							continue;

						for (int c = b + 3, squareCount = 0; ; ++c)
						{
							if (code[a][c].type == TokenType::OPEN_SQUARE)
								squareCount++;
							else if (code[a][c].type == TokenType::CLOSE_SQUARE)
								squareCount--;

							if (code[a][c].type == TokenType::CLOSE_SQUARE && squareCount == -1)
							{
								Token arrayIndexToken = ParseExpression(code[a].access(b + 2, c - 1),
									CONTAINERS);
								int arrayIndex = night::stoi(arrayIndexToken.value);

								if (arrayIndexToken.type != TokenType::INT_VALUE)
									throw Error(night::_invalid_array_, code[a], b + 3, c - 1, "array index can only be a value of type 'int'");
								if (arrayIndex < 0 || arrayIndex >= array->elems.length())
									throw Error(night::_invalid_array_, code[a], b + 3, c - 1, "array index is out of range; the last element is at index '" + night::itos(array->elems.length() - 1) + "'");

								code[a][b] = array->elems[arrayIndex];
								for (int d = b + 1; d <= c; ++d)
									code[a].remove(b + 1);

								break;
							}
						}
					}
				}
			}

			int variableLength = variables.length();
			for (int b = variableScope; b < variableLength; ++b)
				variables.remove(variableScope);

			int arrayLength = arrays.length();
			for (int b = arrayScope; b < arrayLength; ++b)
				arrays.remove(arrayScope);

			returnToken = ParseExpression(code[a].access(1, code[a].length() - 2), CONTAINERS);
			break;
		}
		else if (code[a].length() != 0)
		{
			throw Error(night::_invalid_grammar_, code[a], 0, code[a].length(), "language grammar is invalid");
		}
	}
}

void SplitCode(const night::array<Token>& code)
{
	night::array<night::array<Token> > splitCode;
	splitCode.push_back(night::array<Token>());
	for (int a = 0, openCurly = 0; a < code.length(); ++a)
	{
		splitCode.back().push_back(code[a]);

		if (code[a].type == TokenType::OPEN_CURLY)
			openCurly++;
		else if (code[a].type == TokenType::CLOSE_CURLY)
			openCurly--;

		if (openCurly == 0 && (code[a].type == TokenType::SEMICOLON ||
			code[a].type == TokenType::CLOSE_CURLY))
			splitCode.push_back(night::array<Token>());
	}

	Interpreter(splitCode);
}
