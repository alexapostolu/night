#pragma once

#include <vector>

#include "BitParser.h"
#include "IntParser.h"
#include "DecParser.h"
#include "StrParser.h"

#include "Token.h"
#include "Variable.h"

#include "Output.h"

bool CheckVariable(const std::string& variable, const std::vector<Variable>& variables)
{
	for (std::size_t a = 0; a < variables.size(); ++a)
	{
		if (variable == variables[a].name)
			return false;
	}

	return true;
}

bool ExtractVariables(std::size_t s, std::size_t e, std::vector<Token>& expression,
	const std::vector<Variable>& variables)
{
	bool isDefined = true;
	for (std::size_t a = s; a < e; ++a)
	{
		if (expression[a].type == TokenType::VARIABLE)
		{
			isDefined = false;
			for (std::size_t b = 0; b < variables.size(); ++b)
			{
				if (expression[a].token == variables[b].name)
				{
					expression[a].type = variables[b].type == "bit" ? TokenType::BIT_VALUE :
						variables[b].type == "syb" ? TokenType::SYB_VALUE :
						variables[b].type == "int" ? TokenType::INT_VALUE :
						variables[b].type == "dec" ? TokenType::DEC_VALUE :
						TokenType::STR_VALUE;

					expression[a].token = variables[b].value;

					isDefined = true;
					break;
				}
			}
		}

		if (isDefined)
			break;
	}

	return isDefined;
}

int Parser(std::vector<Token>& tokens)
{
	static bool ifElseStatement = false;

	static std::vector<Variable> variables;

	// variable declaration
	if (tokens.size() == 3 && tokens[0].type != TokenType::PRINT && tokens[1].type == TokenType::VARIABLE &&
		tokens[2].type == TokenType::SEMICOLON)
	{
		switch(tokens[0].type)
		{
		case TokenType::BIT_TYPE:
			variables.push_back(Variable{ "bit", tokens[1].token, "false" });
			break;
		case TokenType::SYB_TYPE:
			variables.push_back(Variable{ "syb", tokens[1].token, " " });
			break;
		case TokenType::INT_TYPE:
			variables.push_back(Variable{ "int", tokens[1].token, "0" });
			break;
		case TokenType::DEC_TYPE:
			variables.push_back(Variable{ "dec", tokens[1].token, "0.0" });
			break;
		case TokenType::STR_TYPE:
			variables.push_back(Variable{ "str", tokens[1].token, "" });
			break;
		default:
			return error("invalid variable declaration");
		}
	}
	// variable initialization
	else if (tokens.size() >= 5 && tokens[1].type == TokenType::VARIABLE &&
		tokens[2].type == TokenType::ASSIGNMENT && tokens.back().type == TokenType::SEMICOLON)
	{
		if (!CheckVariable(tokens[1].token, variables))
			return error("variable '" + tokens[1].token + "' already defined");
		if (!ExtractVariables(3, tokens.size() - 1, tokens, variables))
			return error("variable 'MEEP' is not defined");

		std::vector<Token> expression(tokens.begin() + 3, tokens.end() - 1);

		switch (tokens[0].type)
		{
		case TokenType::BIT_TYPE:
			variables.push_back(Variable{ "bit", tokens[1].token, BitParser(expression) });
			break;
		case TokenType::SYB_TYPE:
			if (tokens.size() > 5)
				return error("invalid character variable initialization");

			variables.push_back(Variable{ "syb", tokens[1].token, tokens[3].token });
			break;
		case TokenType::INT_TYPE:
			variables.push_back(Variable{ "int", tokens[1].token, IntParser(expression) });
			break;
		case TokenType::DEC_TYPE:
			variables.push_back(Variable{ "dec", tokens[1].token, DecParser(expression) });
			break;
		case TokenType::STR_TYPE:
			variables.push_back(Variable{ "str", tokens[1].token, StrParser(expression) });
			break;
		default:
			return error("invalid variable initialization");
		}
	}
	// variable assignment
	else if (tokens.size() >= 4 && tokens[1].type == TokenType::VARIABLE &&
		tokens[2].type == TokenType::ASSIGNMENT && tokens.back().type == TokenType::SEMICOLON)
	{
		if (CheckVariable(tokens[0].token, variables))
			return error("variable not defined");
		if (!ExtractVariables(2, tokens.size() - 1, tokens, variables))
			return error("idk");

		int index = -1;
		for (std::size_t a = 0; a < variables.size(); ++a)
		{
			if (variables[a].name == tokens[0].token)
			{
				index = a;
				break;
			}
		}

		// remove this later!!!!
		if (index == -1)
			return error("DJFNKDF");

		if (variables[index].type == "bool")
		{
			std::vector<Token> temp(tokens.begin() + 2, tokens.end() - 1);
			variables[index].value = BitParser(temp);
		}
		else if (variables[index].type == "char" && tokens[2].type == TokenType::SYB_VALUE)
		{
			variables[index].value = tokens[2].token;
		}
		else if (variables[index].type == "int")
		{
			std::vector<Token> temp(tokens.begin() + 2, tokens.end() - 1);
			variables[index].value = IntParser(temp);
		}
		else if (variables[index].type == "dec")
		{
			std::vector<Token> temp(tokens.begin() + 2, tokens.end() - 1);
			variables[index].value = DecParser(temp);
		}
		else if (variables[index].type == "str")
		{
			std::vector<Token> temp(tokens.begin() + 2, tokens.end() - 1);
			variables[index].value = StrParser(temp);
		}
		else
		{
			error("invalid variable assignment");
		}
	}
	// print
	// make sure that in the future, 'print' is a keyword, and it'll only print one thing
	// make a print function to support multiple print values
	else if (tokens.size() >= 3 && tokens[0].type == TokenType::PRINT &&
		tokens.back().type == TokenType::SEMICOLON)
	{
		if (!ExtractVariables(1, tokens.size() - 1, tokens, variables))
			return error("error");

		if (tokens[1].type == TokenType::BIT_VALUE)
		{
			std::vector<Token> temp(tokens.begin() + 1, tokens.end() - 1);
			storeOutput(BitParser(temp));
		}
		else if (tokens[1].type == TokenType::SYB_VALUE && tokens.size() == 3)
		{
			storeOutput(tokens[1].token);
		}
		else if (tokens[1].type == TokenType::INT_VALUE)
		{
			std::vector<Token> temp(tokens.begin() + 1, tokens.end() - 1);
			storeOutput(IntParser(temp));
		}
		else if (tokens[1].type == TokenType::DEC_VALUE)
		{
			std::vector<Token> temp(tokens.begin() + 1, tokens.end() - 1);
			storeOutput(DecParser(temp));
		}
		else if (tokens[1].type == TokenType::STR_VALUE)
		{
			std::vector<Token> temp(tokens.begin() + 1, tokens.end() - 1);
			storeOutput(StrParser(temp));
		}
		else
		{
			return error("invalid print statement");
		}
	}
	// if statement
	else if (tokens.size() >= 6 && tokens[0].type == TokenType::IF &&
		tokens[1].type == TokenType::OPEN_BRACKET && tokens.back().type == TokenType::CLOSE_CURLY)
	{
		// if expression is false
		ifElseStatement = true;
	}
	else if (tokens.size() >= 3 && tokens[0].type == TokenType::ELSE &&
		tokens[1].type == TokenType::OPEN_CURLY && tokens.back().type == TokenType::CLOSE_CURLY)
	{
		if (ifElseStatement)
		{
			// do stuff

			ifElseStatement = false;
		}
	}
	else
	{
		return error("wat a joek u ar");
	}

	return 0;
}