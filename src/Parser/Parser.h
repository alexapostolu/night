#pragma once

#include <iostream>
#include <vector>

#include "EvaluateExpression.h"
#include "../Runtime.h"
#include "../Output.h"

#include "../DataTypes/Error.h"
#include "../DataTypes/Token.h"
#include "../DataTypes/Variable.h"
#include "../DataTypes/Function.h"
#include "../DataTypes/Array.h"

template <typename T>
TokenType type_to_val(const T& value)
{
	if (value.type == TokenType::BIT_TYPE)
		return TokenType::BIT_VALUE;
	if (value.type == TokenType::SYB_TYPE)
		return TokenType::SYB_VALUE;
	if (value.type == TokenType::INT_TYPE)
		return TokenType::INT_VALUE;
	if (value.type == TokenType::DEC_TYPE)
		return TokenType::DEC_VALUE;
	if (value.type == TokenType::STR_TYPE)
		return TokenType::STR_VALUE;
}

std::string type_to_str(const TokenType& type)
{
	if (type == TokenType::BIT_TYPE || type == TokenType::BIT_VALUE)
		return "bit";
	if (type == TokenType::SYB_TYPE || type == TokenType::SYB_VALUE)
		return "syb";
	if (type == TokenType::INT_TYPE || type == TokenType::INT_VALUE)
		return "int";
	if (type == TokenType::DEC_TYPE || type == TokenType::DEC_VALUE)
		return "dec";
	if (type == TokenType::STR_TYPE || type == TokenType::STR_VALUE)
		return "str";
}

Token assign_to_op(const Token& tk)
{
	if (tk.type == TokenType::PLUS_ASSIGN)
		return Token{ TokenType::PLUS, "+" };
	if (tk.type == TokenType::MINUS_ASSIGN)
		return Token{ TokenType::MINUS, "-" };
	if (tk.type == TokenType::TIMES_ASSIGN)
		return Token{ TokenType::TIMES, "*" };
	if (tk.type == TokenType::DIVIDE_ASSIGN)
		return Token{ TokenType::DIVIDE, "/" };
	if (tk.type == TokenType::MOD_ASSIGN)
		return Token{ TokenType::MOD, "%" };
}

template <typename T>
bool IsDefined(const Token& token, const std::vector<T>& dataType)
{
	for (const T& type : dataType)
	{
		if (token.token == type.name)
			return true;
	}

	return false;
}

int CloseBracketIndex(const std::vector<Token>& tokens, int startIndex)
{
	int openBracketCount = 0;
	for (std::size_t a = startIndex; a < tokens.size(); ++a)
	{
		if (tokens[a].type == TokenType::OPEN_BRACKET)
		{
			openBracketCount += 1;
		}
		else if (tokens[a].type == TokenType::CLOSE_BRACKET)
		{
			if (--openBracketCount == 0)
				return a;
		}
	}
}

void Parser(std::vector<Token>& tokens, bool runtime, bool recursion)
{
	static std::vector<Variable> variables;
	static std::vector<Function> functions;
	static std::vector<Array> arrays;
	static bool predefined = true;

	if (predefined)
	{
		functions.push_back(Function{ TokenType::NULL_TYPE, "print",
			std::vector<Variable>{ Variable{ TokenType::STR_TYPE, "text" } } });
		functions.push_back(Function{ TokenType::BIT_TYPE, "input" });

		predefined = false;
	}

	if (runtime)
	{
		Setup(tokens, functions, true);
		return;
	}

	// variable declaration
	if (tokens.size() == 3 && tokens[0].type <= TokenType::STR_TYPE &&
		tokens[1].type == TokenType::VARIABLE)
	{
		if (IsDefined(tokens[1], variables)) {
			throw Error(night::_token_redefinition_, tokens, 1, 1,
				"variable '" + tokens[1].token + "' is already defined");
		}
		if (IsDefined(tokens[1], functions)) {
			throw Error(night::_token_redefinition_, tokens, 1, 1,
				"variable '" + tokens[1].token + "' cannot have the same name as a function");
		}

		variables.push_back(Variable{ tokens[0].type, tokens[1].token, DefaultValue(tokens[0]) });
	}
	// variable initialization
	else if (tokens.size() >= 5 && tokens[0].type <= TokenType::STR_TYPE &&
		tokens[1].type == TokenType::VARIABLE && tokens[2].type == TokenType::ASSIGNMENT)
	{
		if (IsDefined(tokens[1], variables)) {
			throw Error(night::_token_redefinition_, tokens, 1, 1,
				"variable '" + tokens[1].token + "' is already defined");
		}
		if (IsDefined(tokens[1], functions)) {
			throw Error(night::_token_redefinition_, tokens, 1, 1,
				"variable '" + tokens[1].token + "' cannot have the same name as a function");
		}

		std::vector<Token> expression(tokens.begin() + 3, tokens.end() - 1);

		try {
			tokens[3] = EvaluateExpression(expression, variables, functions, arrays);
		}
		catch (const Error& e) {
			int changeSize = tokens.size() - expression.size() - 4;
			throw Error(e.error, tokens, e.start + 3 + changeSize, e.end + 3 + changeSize, e.desc);
		}
		
		if (tokens[0].type == TokenType::INT_TYPE && tokens[3].type == TokenType::DEC_VALUE)
		{
			tokens[3].type = TokenType::INT_VALUE;
			tokens[3].token = std::to_string(stoi(tokens[3].token));
		}
		else if (tokens[0].type == TokenType::DEC_TYPE && tokens[3].type == TokenType::INT_VALUE)
		{
			tokens[3].type = TokenType::DEC_VALUE;
			tokens[3].token = std::to_string(stof(tokens[3].token));
		}
		else if (tokens[3].type != type_to_val(tokens[0]))
		{
			throw Error(night::_invalid_expression_, tokens, 0, tokens.size() - 1,
				"variable of type '" + type_to_str(tokens[3].type) + "' cannot be assigned to value of "
				"type '" + type_to_str(tokens[0].type) + "'");
		}

		tokens.erase(tokens.begin() + 4, tokens.end() - 1);

		variables.push_back(Variable{ tokens[0].type, tokens[1].token, tokens[3].token });
	}
	// variable assignment
	else if (tokens.size() >= 4 && tokens[0].type == TokenType::VARIABLE &&
		tokens[1].type >= TokenType::ASSIGNMENT && tokens[1].type <= TokenType::MOD_ASSIGN)
	{
		Variable* variable = nullptr;
		for (Variable& var : variables)
		{
			if (tokens[0].token == var.name)
			{
				variable = &var;
				break;
			}
		}

		if (variable == nullptr) {
			throw Error(night::_undefined_token_, tokens, 0, 0,
				"variable '" + tokens[0].token + "' is not defined");
		}

		if (tokens[1].type != TokenType::ASSIGNMENT)
		{
			tokens.insert(tokens.begin() + 2, Token{ TokenType::VARIABLE, tokens[0].token });
			tokens.insert(tokens.begin() + 3, assign_to_op(tokens[1]));
			tokens[1] = Token{ TokenType::ASSIGNMENT, "=" };
		}

		std::vector<Token> expression(tokens.begin() + 2, tokens.end() - 1);

		try {
			tokens[2] = EvaluateExpression(expression, variables, functions, arrays);
		}
		catch (const Error& e) {
			int changeSize = tokens.size() - expression.size() - 3;
			throw Error(e.error, tokens, e.start + 2 + changeSize, e.end + 2 + changeSize, e.desc);
		}

		if (variable->type == TokenType::INT_TYPE && tokens[2].type == TokenType::DEC_VALUE)
		{
			tokens[2].type = TokenType::INT_VALUE;
			tokens[2].token = std::to_string(stoi(tokens[2].token));
		}
		else if (variable->type == TokenType::DEC_TYPE && tokens[2].type == TokenType::INT_VALUE)
		{
			tokens[2].type = TokenType::DEC_VALUE;
			tokens[2].token = std::to_string(stof(tokens[2].token));
		}
		else if (type_to_val(*variable) != tokens[2].type)
		{
			throw Error(night::_invalid_expression_, tokens, 0, tokens.size() - 1,
				"variable of type '" + type_to_str(variable->type) + "' cannot be assigned to a value of "
				"type '" + type_to_str(tokens[2].type) + "'");
		}

		tokens.erase(tokens.begin() + 3, tokens.end() - 1);

		variable->value = tokens[2].token;
	}
	// if statement
	else if (tokens.size() >= 6 && tokens[0].type == TokenType::IF &&
		tokens[1].type == TokenType::OPEN_BRACKET)
	{
		int closeBracketIndex = CloseBracketIndex(tokens, 1);

		std::vector<Token> expression(tokens.begin() + 2, tokens.begin() + closeBracketIndex);

		try {
			tokens[2] = EvaluateExpression(expression, variables, functions, arrays);
		}
		catch(const Error& e) {
			int changeSize = tokens.size() - expression.size() - 3;
			throw Error(e.error, tokens, 2, e.end + changeSize, e.desc);
		}

		tokens.erase(tokens.begin() + 3, tokens.begin() + closeBracketIndex);

		if (tokens[4].type != TokenType::OPEN_CURLY) {
			throw Error(night::_invalid_expression_, tokens, 0, 0,
				"if statement invalid; expected open curly bracket");
		}

		std::vector<Token> temp(tokens.begin() + 5, tokens.end() - 1);
		ExtractLine(temp);

		tokens.erase(tokens.begin() + 5, tokens.end() - 1);
		tokens.insert(tokens.begin() + 5, temp.begin(), temp.end());
	}
	// else if statement
	else if (tokens.size() >= 7 && tokens[0].type == TokenType::ELSE &&
		tokens[1].type == TokenType::IF)
	{
		int closeBracketIndex = CloseBracketIndex(tokens, 2);

		std::vector<Token> expression(tokens.begin() + 3, tokens.begin() + closeBracketIndex);

		try {
			tokens[3] = EvaluateExpression(expression, variables, functions, arrays);
		}
		catch (const Error& e) {
			int changeSize = tokens.size() - expression.size() - 3;
			throw Error(e.error, tokens, e.start + changeSize, e.end + changeSize, e.desc);
		}

		tokens.erase(tokens.begin() + 4, tokens.begin() + closeBracketIndex);

		if (tokens[5].type != TokenType::OPEN_CURLY) {
			throw Error(night::_invalid_expression_, tokens, 0, 0,
				"'else if' statement invalid; expected open curly bracket");
		}

		std::vector<Token> temp(tokens.begin() + 6, tokens.end() - 1);
		ExtractLine(temp);

		tokens.erase(tokens.begin() + 6, tokens.end() - 1);
		tokens.insert(tokens.begin() + 6, temp.begin(), temp.end());
	}
	// else statement
	else if (tokens.size() >= 3 && tokens[0].type == TokenType::ELSE)
	{
		if (tokens[1].type != TokenType::OPEN_CURLY) {
			throw Error(night::_invalid_expression_, tokens, 0, 0,
				"else statement invalid; expected open curly bracket");
		}

		std::vector<Token> temp(tokens.begin() + 2, tokens.end() - 1);
		ExtractLine(temp);

		tokens.erase(tokens.begin() + 6, tokens.end() - 1);
		tokens.insert(tokens.begin() + 6, temp.begin(), temp.end());
	}
	// function call
	else if (tokens.size() >= 4 && tokens[0].type == TokenType::VARIABLE &&
		tokens[1].type == TokenType::OPEN_BRACKET)
	{
		Function* function = nullptr;
		for (Function& func : functions)
		{
			if (tokens[0].token == func.name)
			{
				function = &func;
				break;
			}
		}

		if (function == nullptr) {
			throw Error(night::_undefined_token_, tokens, 0, tokens.size() - 1,
				"function '" + tokens[0].token + "' is undefined");
		}

		int localVariables = variables.size();

		std::vector<Token> temp;
		int functionVariable = 0, openBracketCount = 0, startCode = 2;
		for (std::size_t a = 2; a < tokens.size() - 1; ++a)
		{
			if (tokens[a].type == TokenType::OPEN_BRACKET)
				openBracketCount += 1;
			else if (tokens[a].type == TokenType::CLOSE_BRACKET)
				openBracketCount -= 1;

			if (tokens[a].type != TokenType::COMMA && tokens[a].type != TokenType::CLOSE_BRACKET)
			{
				temp.push_back(tokens[a]);
			}
			else
			{
				if (openBracketCount != 0 && a != tokens.size() - 2)
				{
					temp.push_back(tokens[a]);
					continue;
				}

				if (tokens[a].type == TokenType::CLOSE_BRACKET && a != tokens.size() - 2)
					temp.push_back(tokens[a]);

				try {
					variables.push_back(Variable{
						function->parameters.at(functionVariable).type,
						function->parameters[functionVariable].name,
						EvaluateExpression(temp, variables, functions, arrays).token
					});

					tokens[startCode].token = temp[0].token;
					tokens.erase(tokens.begin() + startCode + 1, tokens.begin() + a);

					a = startCode + 1;
					startCode = a + 1;
					functionVariable += 1;
				}
				catch (const Error& e) {
					throw Error(e.error, tokens, 0, tokens.size() - 1, e.desc);
				}
				catch (...) {
					break;
				}

				temp.clear();
			}
		}

		Token returnToken = { TokenType::BIT_VALUE, "null" };
		ExtractLine(function->code, &returnToken, &variables, &functions, &arrays, 1);

		if (returnToken.type != TokenType::BIT_VALUE && returnToken.token != "null")
		{
			if (function->type == TokenType::INT_TYPE && returnToken.type == TokenType::DEC_VALUE)
			{
			}
			else if (function->type == TokenType::DEC_TYPE && tokens[3].type == TokenType::INT_VALUE)
			{
			}
			else if (returnToken.type != type_to_val(*function))
			{
				throw Error(night::_invalid_expression_, tokens, 0, tokens.size() - 1,
					"function of type '" + type_to_str(function->type) + "' cannot have a return value of "
					"type '" + type_to_str(returnToken.type) + "'");
			}
		}

		variables.erase(variables.begin() + localVariables, variables.end());
	}
	// function definition
	else if (tokens.size() >= 6 && tokens[0].type <= TokenType::NULL_TYPE &&
		tokens[1].type == TokenType::VARIABLE && tokens[2].type == TokenType::OPEN_BRACKET)
	{
		if (IsDefined(tokens[1], functions)) {
			throw Error(night::_token_redefinition_, tokens, 0, tokens.size() - 1,
				"function '" + tokens[1].token + "' is already defined");
		}
		if (IsDefined(tokens[1], variables)) {
			throw Error(night::_token_redefinition_, tokens, 1, 1,
				"function '" + tokens[1].token + "' cannot have the same name as a variable");
		}

		functions.push_back(Function{ tokens[0].type, tokens[1].token });

		for (std::size_t a = 3; tokens[a].type != TokenType::CLOSE_BRACKET; a += 2)
		{
			if (tokens[a].type == TokenType::COMMA)
				a += 1;

			if (tokens[a].type > TokenType::STR_TYPE) {
				throw Error(night::_invalid_expression_, tokens, 0, tokens.size() - 1,
					"excepted variable parameter");
			}

			try {
				if (tokens.at(a + 1).type != TokenType::VARIABLE)
					throw "expected variable parameter";
			}
			catch (...) {
				throw Error(night::_invalid_expression_, tokens, 0, tokens.size() - 1,
					"expected variable parameter");
			}

			functions.back().parameters.push_back(Variable{ tokens[a].type, tokens[a + 1].token });
		}
		
		bool findReturn = false;
		int startCode = (functions.back().parameters.size() * 3) + 4 +
			(int)(functions.back().parameters.size() == 0);
		for (std::size_t a = startCode; a < tokens.size() - 1; ++a)
		{
			if (tokens[a].type == TokenType::RETURN)
			{
				if (tokens[0].type == TokenType::NULL_TYPE) {
					throw Error(night::_invalid_expression_, tokens, a, a,
						"function of type 'null' cannot return a value");
				}

				findReturn = true;
			}

			functions.back().code.push_back(tokens[a]);
		}

		if (tokens[0].type != TokenType::NULL_TYPE && !findReturn) {
			throw Error(night::_invalid_grammar_, tokens, 0, 0,
				"function must return a value");
		}
	}
	// loop
	else if (tokens.size() >= 6 && tokens[0].type == TokenType::LOOP &&
		tokens[1].type == TokenType::OPEN_BRACKET)
	{
		//

		std::vector<Token> expression(tokens.begin() + 2, tokens.begin() + CloseBracketIndex(tokens, 1));
		Token condition = EvaluateExpression(expression, variables, functions, arrays);
		if (condition.type != TokenType::INT_VALUE) {
			throw Error(night::_invalid_expression_, tokens, 0, 3,
				"loop value must be of type 'int'");
		}

		tokens.erase(tokens.begin() + 3, tokens.begin() + CloseBracketIndex(tokens, 1));

		for (int i = 0; i < std::stoi(condition.token); ++i)
		{
			std::vector<Token> codeLine(tokens.begin() + CloseBracketIndex(tokens, 1) + 2, tokens.end() - 1);
			ExtractLine(codeLine);
		}

		
	}
	// array
	else if (tokens.size() >= 10 && tokens[0].type <= TokenType::STR_TYPE &&
		tokens[1].type == TokenType::OPEN_SQUARE)
	{
		arrays.push_back(Array{ tokens[4].token });

		for (int a = 7; a < tokens.size() - 2; a += 2)
			arrays.back().elements.push_back(Token{ tokens[a].type, tokens[a].token });
	}
	// array element
	else if (tokens.size() >= 7 && tokens[0].type == TokenType::VARIABLE &&
		tokens[1].type == TokenType::OPEN_SQUARE)
	{
		Array* arr = nullptr;
		for (std::size_t a = 0; a < arrays.size(); ++a)
		{
			if (tokens[0].token == arrays[a].name)
			{
				arr = &arrays[a];
				break;
			}
		}

		if (arr == nullptr) {
			throw Error(night::_undefined_token_, tokens, 0, tokens.size() - 1,
				"array is not defined");
		}

		std::vector<Token> temp(tokens.begin() + 5, tokens.end() - 1);
		arr->elements[std::stoi(tokens[2].token)] = EvaluateExpression(temp, variables, functions, arrays);
	}
	// error
	else
	{
		throw Error(night::_invalid_grammar_, tokens, 0, tokens.size() - 1,
			"language grammar is invalid");
	}

	if (!recursion)
		Setup(tokens, functions, false);
}

void ExtractLine(std::vector<Token>& tokens, Token* returnType, std::vector<Variable>* vars,
	std::vector<Function>* funcs, std::vector<Array>* arrs, int exitFunction)
{
	std::vector<Token> temp;

	int start = 0, openCurly = 0;
	bool activateReturn = false;
	for (std::size_t a = 0; a < tokens.size(); ++a)
	{
		if (tokens[a].type == TokenType::RETURN)
		{
			if (returnType == nullptr) {
				throw Error(night::_invalid_grammar_, tokens, start, tokens.size() - 1,
					"return must be in a function");
			}

			activateReturn = true;
			continue;
		}

		temp.push_back(tokens[a]);

		if (tokens[a].type == TokenType::OPEN_CURLY)
			openCurly += 1;
		else if (tokens[a].type == TokenType::CLOSE_CURLY)
			openCurly -= 1;

		if (openCurly == 0 && (tokens[a].type == TokenType::SEMICOLON ||
			tokens[a].type == TokenType::CLOSE_CURLY))
		{
			if (activateReturn && returnType != nullptr)
			{
				temp.erase(temp.end() - 1);
				Token value = EvaluateExpression(temp, *vars, *funcs, *arrs);
				*returnType = value;//Token{ value.type, value.token };

				temp.clear();

				if (exitFunction == 1)
					return;
				continue;
			}

			Parser(temp, false, true);

			tokens.erase(tokens.begin() + start, tokens.begin() + a + 1);
			tokens.insert(tokens.begin() + start, temp.begin(), temp.end());

			a -= (a + 1 - start) - temp.size();
			start = a + 1;

			temp.clear();
		}
	}

	if (!temp.empty()) {
		throw Error(night::_invalid_grammar_, temp, 0, tokens.size() - 1,
			"missing semicolon");
	}
}