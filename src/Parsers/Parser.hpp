#pragma once

#include <vector>
#include <iostream>

#include "Token.hpp"
#include "Variable.hpp"

#include "MathParser.hpp"
#include "BoolParser.hpp"

void ExtractVariableValue(const std::vector<Variable> variables, std::vector<Token>& tokens);
void CheckExpression(const std::vector<Token>& tokens, TokenTypes valueType, int start, int end,
	std::string error);
bool CheckEquation(const std::vector<Token>& tokens, int index, TokenTypes type,
	int start, int end);

void Parser(std::vector<Token>& tokens)
{
	// make a singleton class that holds these values?
	static std::vector<Variable> variables;

	// variable initialization
	if (tokens.size() >= 5 && tokens[1].type == TokenTypes::VARIABLE &&
		tokens[2].type == TokenTypes::ASSIGNMENT)
	{
		ExtractVariableValue(variables, tokens);

		if (tokens[0].type == TokenTypes::BOOL_TYPE)
		{
			for (std::size_t a = 3; a < tokens.size() - 1; ++a)
			{
				if (tokens[a].type == TokenTypes::NOT &&
					tokens[a + 1].type == TokenTypes::BOOL_VALUE)
				{
					tokens[a + 1].token = (tokens[a + 1].token == "true" ? "false" : "true");
					tokens.erase(tokens.begin() + a);
				}
				else if (tokens[a].type == TokenTypes::NOT &&
					tokens[a + 1].type == TokenTypes::OPEN_BRACKET)
				{
					int closeBracketIndex = -1;
					for (std::size_t b = a + 1; b < tokens.size(); ++b)
					{
						if (tokens[b].type == TokenTypes::CLOSE_BRACKET)
							closeBracketIndex = b;
					}

					if (closeBracketIndex == -1)
					{
						std::cout << "Error - missing closing bracket";
						exit(0);
					}

					for (std::size_t b = a + 1; b < tokens.size(); ++b)
					{
						if (tokens[b].type == TokenTypes::BOOL_VALUE)
							tokens[b].token = (tokens[b].token == "true" ? "false" : "true");
					}

					tokens.erase(tokens.begin() + a);
				}
			}

			CheckExpression(tokens, TokenTypes::BOOL_VALUE, 11, 12, "boolean");

			std::vector<Token> temp(tokens.begin() + 3, tokens.end() - 1);
			variables.push_back(Variable{ "bool", tokens[1].token, BoolParser(temp) });

			std::cout << variables.back().type << ' ' << variables.back().name << ' ' <<
				variables.back().value;
		}
		else if (tokens[0].type == TokenTypes::CHAR_TYPE)
		{
			// character types don't support arithmetic
			if (tokens[3].type == TokenTypes::OPEN_BRACKET && tokens[5].type == TokenTypes::CLOSE_BRACKET) {
				if (tokens[4].type == TokenTypes::CHAR_TYPE) {
					if (tokens[6].type == TokenTypes::INT_VALUE) {
						char c = std::stoi(tokens[6].token);
						tokens[6] = Token{ TokenTypes::CHAR_VALUE, std::string(1, c) };
					} else if (tokens[6].type == TokenTypes::BOOL_VALUE) {
						tokens[6] = Token{ TokenTypes::CHAR_VALUE, (tokens[6].token == "true") ? "1" : "0" };
					}
				} else { std::cout << "Error - invalid conversion"; exit(0); }
			} else { int d = 3; }
			if (tokens.size() > 5)
			{
				std::cout << "Error - invalid character expression";
				exit(0);
			}
			else
			{
				if (tokens[3].type == TokenTypes::CHAR_VALUE)
				{
					variables.push_back(Variable{ "char", tokens[1].token, tokens[3].token });
				}
				else
				{
					std::cout << "Error - invalid character assignment";
					exit(0);
				}
			}
		}
		else if (tokens[0].type == TokenTypes::INT_TYPE)
		{
			CheckExpression(tokens, TokenTypes::INT_VALUE, 5, 9, "integer");
			
			std::vector<Token> temp(tokens.begin() + 3, tokens.end() - 1);
			variables.push_back(Variable{ "int", tokens[1].token, MathParser(temp) });
		}
		else
		{
			std::cout << "Error - invalid variable initialization";
			exit(0);
		}
	}
	// variable assignment
	else if (tokens.size() >= 4 && tokens[0].type == TokenTypes::VARIABLE &&
		tokens[1].type == TokenTypes::ASSIGNMENT)
	{
		// make sure variable is defined
		bool definedVariable = false;
		for (std::size_t a = 0; a < variables.size(); ++a)
		{
			if (tokens[0].token == variables[a].name)
			{
				definedVariable = true;
				break;
			}
		}

		if (!definedVariable)
		{
			std::cout << "Error - variable not defined";
			exit(0);
		}

		ExtractVariableValue(variables, tokens);
	}
	else if (tokens.size() >= 3)
	{
		ExtractVariableValue(variables, tokens);
		
		for (std::size_t a = 1; a < tokens.size() - 1; ++a)
		{
			std::cout << tokens[a].token;
		}
	}
	else
	{
		std::cout << "Error - invalid language grammar\n";
		exit(0);
	}
}

// turn variables into their types
void ExtractVariableValue(const std::vector<Variable> variables, std::vector<Token>& tokens)
{
	for (std::size_t a = 2; a < tokens.size() - 1; ++a)
	{
		if (tokens[a].type != TokenTypes::VARIABLE)
			continue;

		bool definedVariable = false;
		for (std::size_t b = 0; b < variables.size(); ++b)
		{
			if (tokens[a].token == variables[a].name)
			{
				tokens[a].type = (variables[a].type == "bool" ?
					TokenTypes::BOOL_TYPE : (variables[a].type == "char" ?
						TokenTypes::CHAR_TYPE : TokenTypes::INT_TYPE));

				tokens[a].token = variables[a].value;

				break;
			}
		}

		if (!definedVariable)
		{
			std::cout << "Error - variable not defined";
			exit(0);
		}
	}
}

void CheckExpression(const std::vector<Token>& tokens, TokenTypes type, int start, int end, 
	std::string error)
{
	int openBrackets = 0;
	for (std::size_t a = 3; a < tokens.size() - 1; ++a)
	{
		if (!CheckEquation(tokens, a, type, start, end))
		{
			std::cout << "Error - invalid " << error << " expression";
			exit(0);
		}

		if (tokens[a].type == TokenTypes::OPEN_BRACKET)
			openBrackets += 1;
		else if (tokens[a].type == TokenTypes::CLOSE_BRACKET)
			openBrackets -= 1;
	}

	if (openBrackets > 0)
	{
		std::cout << "Error - missing closing bracket";
		exit(0);
	}
	else if (openBrackets < 0)
	{
		std::cout << "Error - missing opening bracket";
		exit(0);
	}
}

bool CheckEquation(const std::vector<Token>& tokens, int index, TokenTypes valueType, int start, int end)
{
	// checks all numbers
	if (tokens[index].type == valueType)
	{
		if ((tokens[index - 1].type != TokenTypes::ASSIGNMENT ||
				tokens[index + 1].type != TokenTypes::SEMICOLON) &&
			(tokens[index - 1].type != TokenTypes::ASSIGNMENT ||
				(int)tokens[index + 1].type < start || (int)tokens[index + 1].type > end) &&
			((int)tokens[index - 1].type < start || (int)tokens[index - 1].type > end ||
				(int)tokens[index + 1].type < start || (int)tokens[index + 1].type > end) &&
			(tokens[index - 1].type != TokenTypes::OPEN_BRACKET ||
				(int)tokens[index + 1].type < start || (int)tokens[index + 1].type > end) &&
			((int)tokens[index - 1].type < start || (int)tokens[index - 1].type > end ||
				tokens[index + 1].type != TokenTypes::CLOSE_BRACKET) &&
			(tokens[index - 1].type != TokenTypes::OPEN_BRACKET ||
				tokens[index + 1].type != TokenTypes::CLOSE_BRACKET) &&
			((int)tokens[index - 1].type < start || (int)tokens[index - 1].type > end ||
				(tokens[index + 1].type != TokenTypes::SEMICOLON)))
		{
			return false;
		}
	}
	// checks all operators
	else if ((int)tokens[index].type >= start && (int)tokens[index].type <= end)
	{
		if ((tokens[index - 1].type != valueType ||
				tokens[index + 1].type != valueType) &&
			(tokens[index - 1].type != valueType ||
				tokens[index + 1].type != TokenTypes::OPEN_BRACKET) &&
			(tokens[index - 1].type != TokenTypes::CLOSE_BRACKET ||
				tokens[index + 1].type != valueType) &&
			(tokens[index - 1].type != TokenTypes::CLOSE_BRACKET ||
				tokens[index + 1].type != TokenTypes::OPEN_BRACKET))
		{
			return false;
		}
	}
	// checks all open brackets
	else if (tokens[index].type == TokenTypes::OPEN_BRACKET)
	{
		if (((int)tokens[index - 1].type < start || (int)tokens[index - 1].type > end ||
				tokens[index + 1].type != valueType) &&
			((int)tokens[index - 1].type < start || (int)tokens[index - 1].type > end ||
				tokens[index + 1].type != TokenTypes::OPEN_BRACKET) &&
			(tokens[index - 1].type != TokenTypes::OPEN_BRACKET ||
				tokens[index + 1].type != valueType) &&
			(tokens[index - 1].type != TokenTypes::OPEN_BRACKET ||
				tokens[index + 1].type != TokenTypes::OPEN_BRACKET) &&
			(tokens[index - 1].type != TokenTypes::ASSIGNMENT ||
				tokens[index + 1].type != TokenTypes::OPEN_BRACKET) &&
			(tokens[index - 1].type != TokenTypes::ASSIGNMENT ||
				tokens[index + 1].type != valueType))
		{
			return false;
		}
	}
	// checks all close brackets
	else if (tokens[index].type == TokenTypes::CLOSE_BRACKET)
	{
		if ((tokens[index - 1].type != valueType ||
				(int)tokens[index + 1].type < start || (int)tokens[index + 1].type > end) &&
			(tokens[index - 1].type != TokenTypes::CLOSE_BRACKET ||
				(int)tokens[index + 1].type < start || (int)tokens[index + 1].type > end) &&
			(tokens[index - 1].type != valueType ||
				tokens[index + 1].type != TokenTypes::CLOSE_BRACKET) &&
			(tokens[index - 1].type != TokenTypes::CLOSE_BRACKET ||
				tokens[index + 1].type != TokenTypes::CLOSE_BRACKET) &&
			(tokens[index - 1].type != valueType ||
				tokens[index + 1].type != TokenTypes::SEMICOLON) &&
			(tokens[index - 1].type != TokenTypes::CLOSE_BRACKET ||
				tokens[index + 1].type != TokenTypes::SEMICOLON))
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	return true;
}