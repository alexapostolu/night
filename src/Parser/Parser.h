#pragma once

#include <vector>

#include "EvaluateExpression.h"
#include "../Runtime.h"
#include "../Output.h"

#include "DataTypes/Error.h"
#include "DataTypes/Token.h"
#include "DataTypes/Variable.h"
#include "DataTypes/Function.h"

bool CheckDefined(const Token& var, const std::vector<Variable>& vars)
{
	for (std::size_t a = 0; a < vars.size(); ++a)
	{
		if (var.token == vars[a].name)
			return true;
	}

	return false;
}

void ExtractLine(std::vector<Token>& tokens, int start, Token* returnType = nullptr,
	std::vector<Variable>* vars = nullptr, std::vector<Function>* funcs = nullptr);

TokenType type_to_val(Variable& value)
{
	if (value.type == TokenType::BIT_TYPE)
		value.type = TokenType::BIT_VALUE;
	else if (value.type == TokenType::SYB_TYPE)
		value.type = TokenType::SYB_VALUE;
	else if (value.type == TokenType::INT_TYPE)
		value.type = TokenType::INT_VALUE;
	else if (value.type == TokenType::DEC_TYPE)
		value.type = TokenType::DEC_VALUE;
	else if (value.type == TokenType::STR_TYPE)
		value.type = TokenType::STR_VALUE;

	return value.type;
}

void Parser(std::vector<Token>& tokens, bool runtime, bool recursion)
{
	static std::vector<Variable> variables;
	static std::vector<Function> functions;
	static bool predefined = true;

	if (predefined)
	{
		functions.push_back(Function{ "print" });
		predefined = false;
	}

	if (runtime)
	{
		Setup(tokens, variables, runtime);
		return;
	}

	// variable declaration
	if (tokens.size() == 3 && tokens[0].type <= TokenType::STR_TYPE &&
		tokens[1].type == TokenType::VARIABLE)
	{
		if (CheckDefined(tokens[1], variables)) {
			throw Error(night::_token_redefinition_, tokens, 1, 1,
				"variable '" + tokens[1].token + "' is already defined");
		}

		variables.push_back(Variable{ tokens[0].type, tokens[1].token,
			tokens[0].type == TokenType::BIT_TYPE ? "false" :
			tokens[0].type == TokenType::SYB_TYPE ? " " :
			tokens[0].type == TokenType::INT_TYPE ? "0" :
			tokens[0].type == TokenType::DEC_TYPE ? "0.0" :
			"" });
	}
	// variable initialization
	else if (tokens.size() >= 5 && tokens[0].type <= TokenType::STR_TYPE &&
		tokens[1].type == TokenType::VARIABLE && tokens[2].type == TokenType::ASSIGNMENT)
	{
		if (CheckDefined(tokens[1], variables)) {
			throw Error(night::_token_redefinition_, tokens, 1, 1,
				"variable '" + tokens[1].token + "' is already defined");
		}

		std::vector<Token> expression(tokens.begin() + 3, tokens.end() - 1);

		try {
			tokens[3] = EvaluateExpression(expression, variables, functions);
		}
		catch (const Error& e) {
			int changeSize = tokens.size() - expression.size() - 4;
			throw Error(e.error, tokens, e.start + 3 + changeSize, e.end + 3 + changeSize, e.desc);
		}
		
		if (tokens[0].type == TokenType::BIT_TYPE && tokens[3].type != TokenType::BIT_VALUE) {
			throw Error(night::_invalid_grammar_, tokens, 0, tokens.size() - 1,
				"value '" + tokens[3].token + "' can only be assigned to 'bit' variable");
		}
		if (tokens[0].type == TokenType::SYB_TYPE && tokens[3].type != TokenType::SYB_VALUE) {
			throw Error(night::_invalid_grammar_, tokens, 0, tokens.size() - 1,
				"value '" + tokens[3].token + "' can only be assigned to 'syb' variable");
		}
		if (tokens[0].type == TokenType::INT_TYPE && (tokens[3].type != TokenType::INT_VALUE &&
			tokens[3].type != TokenType::DEC_VALUE)) {
			throw Error(night::_invalid_grammar_, tokens, 0, tokens.size() - 1,
				"value '" + tokens[3].token + "' can only be assigned to 'int' variable");
		}
		if (tokens[0].type == TokenType::DEC_TYPE && (tokens[3].type != TokenType::DEC_VALUE &&
			tokens[3].type != TokenType::INT_VALUE)) {
			throw Error(night::_invalid_grammar_, tokens, 0, tokens.size() - 1,
				"value '" + tokens[3].token + "' can only be assigned to 'dec' variable");
		}
		if (tokens[0].type == TokenType::STR_TYPE && tokens[3].type != TokenType::STR_VALUE) {
			throw Error(night::_invalid_grammar_, tokens, 0, tokens.size() - 1,
				"value '" + tokens[3].token + "' can only be assigned to 'str' variable");
		}

		tokens.erase(tokens.begin() + 4, tokens.end() - 1);

		variables.push_back(Variable{ tokens[0].type, tokens[1].token, tokens[3].token });
	}
	// variable assignment
	else if (tokens.size() >= 4 && tokens[0].type == TokenType::VARIABLE &&
		tokens[1].type == TokenType::ASSIGNMENT)
	{
		if (!CheckDefined(tokens[0], variables)) {
			throw Error(night::_undefined_token_, tokens, 0, 0,
				"variable '" + tokens[0].token + "' is not defined");
		}

		std::vector<Token> expression(tokens.begin() + 2, tokens.end() - 1);

		try {
			tokens[2] = EvaluateExpression(expression, variables, functions);
		}
		catch (const Error& e) {
			int changeSize = tokens.size() - expression.size() - 3;
			throw Error(e.error, tokens, e.start + 2 + changeSize, e.end + 2 + changeSize, e.desc);
		}

		tokens.erase(tokens.begin() + 3, tokens.end() - 1);
	}
	// if statement
	else if (tokens.size() >= 6 && tokens[0].type == TokenType::IF &&
		tokens[1].type == TokenType::OPEN_BRACKET)
	{
		int closeBracketIndex = 0;
		for (std::size_t a = 2; a < tokens.size(); ++a)
		{
			if (tokens[a].type == TokenType::CLOSE_BRACKET)
			{
				closeBracketIndex = a;
				break;
			}
		}

		std::vector<Token> expression(tokens.begin() + 2, tokens.begin() + closeBracketIndex);

		try {
			tokens[2] = EvaluateExpression(expression, variables, functions);
		}
		catch(const Error& e) {
			int changeSize = tokens.size() - expression.size() - 3;
			throw Error(e.error, tokens, e.start + 2 + changeSize, e.end + 2 + changeSize, e.desc);
		}

		tokens.erase(tokens.begin() + 3, tokens.begin() + closeBracketIndex);

		ExtractLine(tokens, 5);
	}
	// else if statement
	else if (tokens.size() >= 7 && tokens[0].type == TokenType::ELSE && tokens[1].type == TokenType::IF)
	{
		int closeBracketIndex = 0;
		for (std::size_t a = 2; a < tokens.size(); ++a)
		{
			if (tokens[a].type == TokenType::CLOSE_BRACKET)
			{
				closeBracketIndex = a;
				break;
			}
		}

		std::vector<Token> expression(tokens.begin() + 3, tokens.begin() + closeBracketIndex);

		try {
			tokens[3] = EvaluateExpression(expression, variables, functions);
		}
		catch (const Error& e) {
			int changeSize = tokens.size() - expression.size() - 3;
			throw Error(e.error, tokens, e.start + changeSize, e.end + changeSize, e.desc);
		}

		tokens.erase(tokens.begin() + 4, tokens.begin() + closeBracketIndex);

		ExtractLine(tokens, 6);
	}
	// else statement
	else if (tokens.size() >= 3 && tokens[0].type == TokenType::ELSE)
	{
		ExtractLine(tokens, 2);
	}
	// function call
	else if (tokens.size() >= 4 && tokens[0].type == TokenType::VARIABLE &&
		tokens[1].type == TokenType::OPEN_BRACKET)
	{
		// predefined functions
		if (tokens[0].token == "print")
		{
			std::vector<Token> temp(tokens.begin() + 2, tokens.end() - 2);
			StoreOutput(EvaluateExpression(temp, variables, functions).token);

			if (!recursion)
				Setup(tokens, variables, false);
			
			return;
		}

		// get function
		int functionIndex = 0;
		for (std::size_t a = 0; a < functions.size(); ++a)
		{
			if (tokens[0].token == functions[a].name)
			{
				functionIndex = a;
				break;
			}
		}

		// undefined function
		if (functionIndex == -1) {
			throw Error(night::_undefined_token_, tokens, 0, tokens.size() - 1,
				"function '" + tokens[0].token + "' is undefined");
		}
		
		// check parameters
		/*
		if ((tokens.size() - 4 == 0 && functions[functionIndex].parameters.size() == 0) ||
			(tokens.size() - 4 != functions[functionIndex].parameters.size() * 2 - 1)) {
			throw Error(night::_invalid_expression_, tokens, 0, tokens.size() - 1,
				"function parameters do not match up");
		}
		*/

		int localVariables = variables.size();

		// parameters
		int functionVariable = 0;
		std::vector<Token> temp;
		int openBracket = 0;
		for (std::size_t a = 2; a < tokens.size() - 1; ++a)
		{
			if (tokens[a].type == TokenType::OPEN_BRACKET)
				openBracket += 1;
			else if (tokens[a].type == TokenType::CLOSE_BRACKET)
				openBracket -= 1;
			
			// check for openBrakcet to avoid commas inside nested function calls
			if (tokens[a].type != TokenType::COMMA && tokens[a].type != TokenType::CLOSE_BRACKET)
			{
				temp.push_back(tokens[a]);
			}
			else
			{
				try {
					variables.push_back(Variable{
						type_to_val(functions[functionIndex].parameters.at(functionVariable)),
						functions[functionIndex].parameters[functionVariable].name,
						EvaluateExpression(temp, variables, functions).token
					});

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

		//functions[functionIndex].code.push_back(Token{ TokenType::SEMICOLON, ";" });
		ExtractLine(functions[functionIndex].code, 0, &tokens[0], &variables, &functions);
		//functions[functionIndex].code.erase(functions[functionIndex].code.end() - 1);

		// return type
		tokens.erase(tokens.begin() + 1, tokens.end());
		//okens[0].type;
		//tokens[0].token;

		variables.erase(variables.begin() + localVariables, variables.end());
	}
	// function definition
	else if (tokens.size() >= 6 && tokens[0].type <= TokenType::STR_TYPE &&
		tokens[1].type == TokenType::VARIABLE && tokens[2].type == TokenType::OPEN_BRACKET)
	{
		for (std::size_t a = 0; a < functions.size(); ++a)
		{
			if (tokens[1].token == functions[a].name) {
				throw Error(night::_token_redefinition_, tokens, 0, tokens.size() - 1,
					"function '" + tokens[1].token + "' is already defined");
			}
		}

		functions.push_back(Function{ tokens[1].token });

		std::vector<Token> temp;
		int start = 3;
		for (std::size_t a = 3; tokens[a].type != TokenType::CLOSE_BRACKET; a += 2)
		{
			if (tokens[a].type == TokenType::COMMA)
				a += 1;

			if (tokens[a].type > TokenType::STR_TYPE) {
				throw Error(night::_invalid_expression_, tokens, 0, tokens.size() - 1,
					"excepted variable parameter");
			}

			try {
				if (tokens.at(a + 1).type != TokenType::VARIABLE) {
					throw "";
				}
			}
			catch (...) {
				throw Error(night::_invalid_expression_, tokens, 0, tokens.size() - 1,
					"expected variable parameter");
			}

			functions.back().parameters.push_back(Variable{ tokens[a].type, tokens[a + 1].token });
		}
		
		int startCode = functions.back().parameters.size() * 3 + 4 +
			(functions.back().parameters.size() == 0 ? 1 : 0);
		for (std::size_t a = startCode; a < tokens.size() - 1; ++a)
			functions.back().code.push_back(tokens[a]);
	}
	// error
	else
	{
		throw Error(night::_invalid_grammar_, tokens, 0, tokens.size() - 1,
			"language grammar is invalid");
	}

	if (!recursion)
		Setup(tokens, variables, false);
}

void ExtractLine(std::vector<Token>& tokens, int start, Token* returnType,
	std::vector<Variable>* vars, std::vector<Function>* funcs)
{
	std::vector<Token> temp;
	int openCurly = 0;
	bool activateReturn = false;
	for (std::size_t a = start; a < tokens.size(); ++a) // a < tokens.size() - 1
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
				Token value = EvaluateExpression(temp, *vars, *funcs);
				*returnType = Token{ value.type, value.token };
				return;
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