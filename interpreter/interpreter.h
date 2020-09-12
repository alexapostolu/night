#pragma once

#include "../lib/string.h"
#include "../lib/array.h"
#include "../lib/error.h"

#include "../containers/variable.h"
#include "../containers/function.h"
#include "../containers/token.h"

#include "./expr_parser.h"
#include "./helpers.h"

#include "../output.h"

bool IsType(const Token& token)
{
	return token.type >= TokenType::BIT_TYPE && token.type <= TokenType::STR_TYPE;
}

bool IsVar(const Token& token)
{
	return token.type == TokenType::VARIABLE;
}

bool IsAssign(const Token& token)
{
	return token.type >= TokenType::ASSIGNMENT && token.type <= TokenType::MOD_ASSIGN;
}

night::string DefaultValue(const TokenType& type)
{
	switch (type)
	{
	case TokenType::BIT_TYPE:
		return "true";
	case TokenType::SYB_TYPE:
		return " ";
	case TokenType::INT_TYPE:
		return "0";
	case TokenType::DEC_TYPE:
		return "0.0";
	case TokenType::STR_TYPE:
		return "";
	}
}

template <typename T>
bool TokenDefined(const night::array<T>& container, const Token& token)
{
	for (int a = 0; a < container.length(); ++a)
	{
		if (token.value == container[a].name)
			return true;
	}

	return false;
}

void SplitCode(const night::array<Token>& code);

void Interpreter(const night::array<night::array<Token> >& code)
{
	static night::array<Variable> variables;
	static night::array<Function> functions;

	static bool predefined = false;
	if (!predefined)
	{
		functions.push_back(Function{ TokenType::NULL_TYPE, "print" });
		predefined = true;
	}

	for (int a = 0; a < code.length(); ++a)
	{
		if (code[a].length() == 3 && IsType(code[a][0]) && IsVar(code[a][0]))
		{
			if (TokenDefined(variables, code[a][1]))
				throw Error(night::_redefined_token_, code[a], 1, 1, "variable is already defined");

			variables.push_back(Variable{
				code[a][0].type,
				code[a][1].value,
				DefaultValue(code[a][0].type)
			});
		}
		else if (code[a].length() >= 5 && IsType(code[a][0]) && IsAssign(code[a][2]))
		{
			if (TokenDefined(variables, code[a][1]))
				throw Error(night::_redefined_token_, code[a], 1, 1, "variable is already defined");

			Token evalExpr = ParseExpression(code[a].access(3, code[a].length() - 1), variables);
			if (code[a][0].type == TokenType::INT_TYPE && evalExpr.type == TokenType::DEC_VALUE)
				evalExpr.value = night::stoi(evalExpr.value);
			else if (code[a][0].type == TokenType::DEC_TYPE && evalExpr.type == TokenType::INT_VALUE)
				evalExpr.value = night::stof(evalExpr.value);
			else if (night::ttov(code[a][0].type) != evalExpr.type)
				throw Error(night::_invalid_variable_, code[a], 3, code.length(), "variable of type '"_s + night::ttos(code[a][0].type) + "' cannot be assigned with expression of type '" + night::ttos(evalExpr.type) + "'");

			variables.push_back(Variable{
				code[a][0].type,
				code[a][1].value,
				evalExpr.value
			});
		}
		else if (code[a].length() >= 4 && IsAssign(code[a][1]))
		{
			// do not modify the code!!
			if (code[a][1].type != TokenType::ASSIGNMENT)
			{
				code[a].insert(2, code[a][0]);
				code[a].insert(3, night::atoo(code[a][1].type));
			}

			Variable* variable = nullptr;
			for (int b = 0; b < variables.length(); ++b)
			{
				if (code[a][0].value == variables[b].name)
				{
					variable = &variables[b];
					break;
				}
			}

			if (variable == nullptr)
				throw Error(night::_undefined_token_, code[a], 0, 0, "variable has not been defined");

			Token evalExpr = ParseExpression(code[a].access(2, code[a].length() - 1), variables);
			if (variable->type == TokenType::INT_TYPE && evalExpr.type == TokenType::DEC_VALUE)
				evalExpr.value = night::stoi(evalExpr.value);
			else if (variable->type == TokenType::DEC_TYPE && evalExpr.type == TokenType::INT_VALUE)
				evalExpr.value = night::stof(evalExpr.value);
			else if (night::ttov(variable->type) != evalExpr.type)
				throw Error(night::_invalid_variable_, code[a], 2, code[a].length() - 1, "variable of type '"_s + night::ttos(variable->type) + "' cannot be assigned with expression of type '" + night::ttos(evalExpr.type) + "'");

			variable->value = evalExpr.value;
		}
		else if (code[a].length() >= 6 && code[a][0].type == TokenType::IF)
		{
			int bracketIndex = 2;
			for (; code[a][bracketIndex].type != TokenType::CLOSE_BRACKET; ++bracketIndex);

			Token condition = ParseExpression(code[a].access(2, bracketIndex - 1), variables);
			if (condition.type != TokenType::BIT_VALUE)
				throw Error(night::_invalid_statement_, code[a], 2, bracketIndex, "if statement condition must evaluate to a value of type 'bit'");

			if (condition.value == "true")
			{
				code[a][0].value[0] = 'c';
				SplitCode(code[a].access(bracketIndex + 2, code[a].length() - 1));
			}
		}
		else if (code[a].length() >= 7 && code[a][1].type == TokenType::IF)
		{
			if (code.length() == 0 || (code[a - 1][0].type != TokenType::IF &&
				code[a - 1][1].type != TokenType::IF))
				throw Error(night::_invalid_statement_, code[a], 0, 1, "else if statement must come after an if statement");

			int bracketIndex = 3;
			for (; code[a][bracketIndex].type != TokenType::CLOSE_BRACKET; ++bracketIndex);

			Token condition = ParseExpression(code[a].access(3, bracketIndex - 1), variables);
			if (condition.type != TokenType::BIT_VALUE)
				throw Error(night::_invalid_statement_, code[a], 2, bracketIndex, "if statement condition must evaluate to a value of type 'bit'");

			if (condition.value == "true" && code[a - 1][0].value[0] != 'c')
			{
				code[a][0].value[0] = 'c';
				SplitCode(code[a].access(bracketIndex + 2, code[a].length() - 1));
			}
		}
		else if (code[a].length() >= 3 && code[a][0].type == TokenType::ELSE)
		{
			if (code.length() == 0 || (code[a - 1][0].type != TokenType::IF &&
				code[a - 1][1].type != TokenType::IF))
				throw Error(night::_invalid_statement_, code[a], 0, 1, "else statement must come after an if statement or an else if statement");

			if (code[a - 1][0].value[0] != 'c')
				SplitCode(code[a].access(2, code[a].length() - 1));
		}
		else if (code[a].length() >= 6 && code[a][2].type == TokenType::OPEN_BRACKET)
		{
			if (TokenDefined(functions, code[a][1]))
				throw Error(night::_redefined_token_, code[a], 1, 1, "function '"_s + code[a][1].value + "' is already defined");
			if (TokenDefined(variables, code[a][1]))
				throw Error(night::_redefined_token_, code[a], 1, 1, "function '"_s + code[a][1].value+ "' cannot have the same name as a variable");

			functions.push_back(Function{ code[a][0].type, code[a][1].value });

			int b;
			for (b = 3; code[a][b].type != TokenType::CLOSE_BRACKET; b += 2)
			{
				if (code[a][b].type == TokenType::COMMA)
					b++;

				functions.back().params.push_back(Variable{ code[a][b].type, code[a][b + 1].value });
			}

			bool findReturn = false;
			for (b++; b < code[a].length() - 1; ++b)
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
		else if (code[a].length() >= 4 && IsVar(code[a][0]) && code[a][1].type == TokenType::OPEN_BRACKET)
		{
			if (code[a][0].value == "print")
			{
				StoreOutput(ParseExpression(code[a].access(1, code[a].length() - 2), variables).value);
				continue;
			}

			Function* function = nullptr;
			for (int b = 0; b < functions.length(); ++b)
			{
				if (code[a][0].value == functions[b].name)
				{
					function = &functions[b];
					break;
				}
			}

			if (function == nullptr)
				throw Error(night::_invalid_function_, code[a], 0, code[a].length(), "function is not defined");

			int variableScope = variables.length();

			int openBracket = 0, paramIndex = 0;
			night::array<Token> paramExpr;
			for (int b = 2; b < code[a].length(); ++b)
			{
				if (code[a][b].type == TokenType::OPEN_BRACKET)
					openBracket++;
				else if (code[a][b].type == TokenType::CLOSE_BRACKET)
					openBracket--;

				if ((openBracket == 0 && code[a][b].type == TokenType::COMMA) ||
					(openBracket == -1 && code[a][b].type == TokenType::CLOSE_BRACKET))
				{
					if (paramIndex + 1 > function->params.length())
						throw Error(night::_invalid_function_, code[a], 0, code[a].length(), "expecting "_s + night::itos(function->params.length()) + " parameters");

					Token paramExprVal = ParseExpression(paramExpr, variables);
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
				throw Error(night::_invalid_function_, code[a], 0, code[a].length(), "expecting "_s + night::itos(function->params.length()) + " parameters");

			night::array<night::array<Token> > splitCode;
			splitCode.push_back(night::array<Token>());

			bool onReturn = false;
			int openCurly = 0;
			for (int b = 0; b < function->code.length(); ++b)
			{
				splitCode.back().push_back(function->code[b]);

				if (function->code[b].type == TokenType::OPEN_CURLY)
					openCurly++;
				else if (function->code[b].type == TokenType::CLOSE_CURLY)
					openCurly--;
				else if (function->code[b].type == TokenType::RETURN)
					onReturn = true;

				if (openCurly == 0 && (function->code[b].type == TokenType::SEMICOLON ||
					function->code[b].type == TokenType::CLOSE_CURLY))
				{
					Token returnValue = ParseExpression(splitCode.back().access(1, splitCode.length() - 1),
						variables);
					if (returnValue.type != night::ttov(function->type))
						throw Error(night::_invalid_function_, function->code, b - splitCode.length(), b, "return value does not match with function type");

					splitCode.push_back(night::array<Token>());
				}
			}

			Interpreter(splitCode);
		}
		else if (code[a].length() >= 7 && code[a][1].type == TokenType::WHILE)
		{
			//


		}
		else if (code[a].length() >= 7 && code[a][1].type == TokenType::FOR)
		{
			//


		}
	}
}

void SplitCode(const night::array<Token>& code)
{
	night::array<night::array<Token> > splitCode;
	splitCode.push_back(night::array<Token>());

	int openCurly = 0;
	for (int a = 0; a < code.length(); ++a)
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