#pragma once

#include <iostream>
#include <vector>

#include "Token.h"
#include "Variable.h"

#include "BoolParser.h"
#include "IntParser.h"
#include "StrParser.h"

void CheckVariable(const std::vector<Variable>& vars, const Token& var)
{
	for (std::size_t a = 0; a < vars.size(); ++a)
	{
		if (vars[a].name == var.token)
		{
			std::cout << "Error - redefinition of variable '" << var.token << "'\n";
			exit(0);
		}
	}
}

void Parser(std::vector<Token>& tokens)
{
	static std::vector<Variable> variables;

	if (tokens.size() == 3 && tokens[0].type == TokenType::PRINT)
	{
		if (tokens[1].type == TokenType::VARIABLE)
		{
			bool definedVariable = false;
			for (std::size_t b = 0; b < variables.size(); ++b)
			{
				if (tokens[1].token == variables[b].name)
				{
					std::cout << variables[b].value << '\n';

					definedVariable = true;
					break;
				}
			}

			if (!definedVariable)
			{
				std::cout << "Error - undefined variable '" << tokens[1].token << "'\n";
				exit(0);
			}
		}
		else if (tokens[1].type == TokenType::BOOL_VALUE || tokens[1].type == TokenType::CHAR_VALUE ||
			tokens[1].type == TokenType::INT_VALUE || tokens[1].type == TokenType::STRING_VALUE)
		{
			std::cout << tokens[1].token << '\n';
		}
		else
		{
			std::cout << "Error - invalid print statement\n";
			exit(0);
		}
	}
	else if (tokens.size() >= 5)
	{
		if (tokens[0].type == TokenType::BOOL_TYPE && tokens[1].type == TokenType::VARIABLE &&
			tokens[2].type == TokenType::ASSIGNMENT)
		{
			if (!CheckBool(tokens, variables))
			{
				std::cout << "Error - invalid boolean expression\n";
				exit(0);
			}

			CheckVariable(variables, tokens[1]);

			std::vector<Token> temp(tokens.begin() + 3, tokens.end() - 1);
			variables.push_back(Variable{tokens[0].token , tokens[1].token, BoolParser(temp)});
		}
		else if (tokens[0].type == TokenType::CHAR_TYPE && tokens[1].type == TokenType::VARIABLE &&
			tokens[2].type == TokenType::ASSIGNMENT && tokens[3].type == TokenType::CHAR_VALUE)
		{
			variables.push_back(Variable{ "char", tokens[1].token, tokens[3].token });
		}
		else if (tokens[0].type == TokenType::INT_TYPE && tokens[1].type == TokenType::VARIABLE &&
			tokens[2].type == TokenType::ASSIGNMENT)
		{
			if (!CheckInt(tokens, variables))
			{
				std::cout << "Error - invalid integer expression\n";
				exit(0);
			}

			CheckVariable(variables, tokens[1]);

			std::vector<Token> temp(tokens.begin() + 3, tokens.end() - 1);
			variables.push_back(Variable{ tokens[0].token , tokens[1].token, IntParser(temp) });
		}
		else if (tokens[0].type == TokenType::STRING_TYPE && tokens[1].type == TokenType::VARIABLE &&
			tokens[2].type == TokenType::ASSIGNMENT)
		{
			if (!CheckStr(tokens, variables))
			{
				std::cout << "Error - invalid string expression\n";
				exit(0);
			}

			CheckVariable(variables, tokens[1]);

			std::vector<Token> temp(tokens.begin() + 1, tokens.end() - 1);
			variables.push_back(Variable{ tokens[0].token , tokens[1].token, StrParser(temp) });
		}
		else
		{
			std::cout << "Error - invalid variable initialization\n";
			exit(0);
		}
	}
	else
	{
		std::cout << "Error - invalid language grammar\n";
		exit(0);
	}
}
