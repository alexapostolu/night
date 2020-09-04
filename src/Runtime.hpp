#pragma once

#include <iostream>
#include <vector>

#include "Output.hpp"

#include "Parser/Parser.hpp"
#include "DataTypes/Error.hpp"
#include "DataTypes/Token.hpp"
#include "DataTypes/Variable.hpp"
#include "DataTypes/Function.hpp"

std::string DefaultValue(const Token& token)
{
	if (token.type == TokenType::BIT_TYPE)
		return "false";
	if (token.type == TokenType::SYB_TYPE)
		return " ";
	if (token.type == TokenType::INT_TYPE)
		return "0";
	if (token.type == TokenType::DEC_TYPE)
		return "0.0";
	if (token.type == TokenType::STR_TYPE)
		return "";
}

void ExtractCodeLines(const std::vector<Token>& codeGroup, const std::vector<Function>& funcs);

void CheckIfStatement(const std::vector<std::vector<Token> >& code, int index)
{
	try {
		if (code.at(index - 1)[0].type != TokenType::IF && code[index - 1][0].type != TokenType::ELSE)
			throw "'else if' statement must come after an 'if' or another 'else if' statement";
	}
	catch (...) {
		throw Error(night::_invalid_if_statement_, code[index], 0, 4,
			"'else if' statement must come after an 'if' or another 'else if' statement");
	}
}

bool PredefineFunction(const std::vector<Token>& code)
{
	if (code[0].token == "print")
	{
		StoreOutput(code[2].token);
		return true;
	}

	return false;
}

int CloseBracketIndex(const std::vector<Token>& tokens, int startIndex);

void Runtime(const std::vector<std::vector<Token> >& code, const std::vector<Function>& functions)
{
	static std::vector<Variable> variables;

	bool nextStatement = false;

	for (std::size_t a = 0; a < code.size(); ++a)
	{
		// variable declaration
		if (code[a].size() == 3 && code[a][0].type <= TokenType::STR_TYPE)
		{
			variables.push_back(Variable{ code[a][0].type, code[a][1].token, DefaultValue(code[a][0]) });
		}
		// variables initialization
		else if (code[a].size() == 5 && code[a][0].type <= TokenType::STR_TYPE)
		{
			variables.push_back(Variable{ code[a][0].type, code[a][1].token, code[a][3].token });
		}
		// variable assignment
		else if (code[a].size() == 4 && code[a][1].type == TokenType::ASSIGNMENT)
		{
			for (std::size_t a = 0; a < variables.size(); ++a)
			{
				if (code[a][0].token == variables[a].name)
				{
					variables[a].value = code[a][2].token;
					return;
				}
			}
		}
		// if statement
		else if (code[a].size() >= 6 && code[a][0].type == TokenType::IF)
		{
			nextStatement = code[a][2].token == "false";
			if (!nextStatement)
			{
				std::vector<Token> temp(code[a].begin() + 5, code[a].end() - 1);
				ExtractCodeLines(temp, functions);

				nextStatement = false;
			}
		}
		// else if statement
		else if (code[a].size() >= 7 && code[a][1].type == TokenType::IF)
		{
			CheckIfStatement(code, a);

			int conditionIndex = code[a - 1][0].type == TokenType::IF ? 2 : 3;
			if (nextStatement && code[a][3].token == "true")
			{
				std::vector<Token> temp(code[a].begin() + 6, code[a].end() - 1);
				ExtractCodeLines(temp, functions);

				nextStatement = false;
			}
		}
		// else statement
		else if (code[a].size() >= 3 && code[a][0].type == TokenType::ELSE)
		{
			CheckIfStatement(code, a);

			int conditionIndex = code[a - 1][0].type == TokenType::IF ? 2 : 3;
			if (nextStatement)
			{
				std::vector<Token> temp(code[a].begin() + 2, code[a].end() - 1);
				ExtractCodeLines(temp, functions);

				nextStatement = false;
			}
		}
		// loop
		else if (code[a].size() >= 6 && code[a][0].type == TokenType::LOOP)
		{
			for (int i = 0; i < std::stoi(code[a][2].token); ++i)
			{
				std::vector<Token> codeLine(code[a].begin() + CloseBracketIndex(code[a], 1) + 2,
					code[a].end() - 1);
				ExtractCodeLines(codeLine, functions);
			}
		}
		// function call
		else if (code[a].size() >= 4 && code[a][1].type == TokenType::OPEN_BRACKET)
		{
			if (PredefineFunction(code[a]))
				continue;

			for (const Function& func : functions)
			{
				if (code[a][0].token == func.name)
				{
					std::size_t variableChange = variables.size();

					for (const Variable& var : func.parameters)
						variables.push_back(var);

					ExtractCodeLines(func.code, functions);

					variables.erase(variables.begin() + variableChange, variables.end());
					break;
				}
			}
		}
	}
}

void Setup(const std::vector<Token>& tokens, const std::vector<Function>& funcs, bool runtime)
{
	static std::vector<std::vector<Token> > code;

	if (runtime)
		Runtime(code, funcs);
	else
		code.push_back(tokens);
}

void ExtractCodeLines(const std::vector<Token>& codeGroup, const std::vector<Function>& funcs)
{
	int curlyBracket = 0, startCode = 0;
	std::vector<std::vector<Token> > code;
	for (std::size_t b = 0; b < codeGroup.size(); ++b)
	{
		if (codeGroup[b].type == TokenType::OPEN_CURLY)
		{
			curlyBracket += 1;
		}
		else if (codeGroup[b].type == TokenType::CLOSE_CURLY)
		{
			curlyBracket -= 1;
		}
		else if (curlyBracket == 0 && (codeGroup[b].type == TokenType::SEMICOLON ||
				codeGroup[b].type == TokenType::CLOSE_CURLY))
		{
			std::vector<Token> temp(codeGroup.begin() + startCode, codeGroup.begin() + b + 1);
			code.push_back(temp);

			startCode = b + 1;
		}
	}

	Runtime(code, funcs);
}
