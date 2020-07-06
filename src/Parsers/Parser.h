#pragma once

#include <iostream>
#include <vector>

#include "BoolParser.h"
#include "IntParser.h"
#include "StrParser.h"

#include "Variable.h"
#include "Error.h"
#include "Token.h"

void ExtractVariable(std::vector<Token>& tokens, const std::vector<Variable>& variables, int start = 3, int end = 1)
{
	// turn variables into their values
	for (std::size_t a = start; a < tokens.size() - end; ++a)
	{
		if (tokens[a].type == TokenType::VARIABLE)
		{
			bool definedVariable = false;
			for (std::size_t b = 0; b < variables.size(); ++b)
			{
				if (tokens[a].token == variables[b].name)
				{
					if (variables[b].type == "bool")
						tokens[a].type = TokenType::BOOL_VALUE;
					else if (variables[b].type == "char")
						tokens[a].type = TokenType::CHAR_VALUE;
					else if (variables[b].type == "int")
						tokens[a].type = TokenType::INT_VALUE;
					else if (variables[b].type == "str")
						tokens[a].type = TokenType::STRING_VALUE;
					else
						error("Did you forget to add a data type?");

					tokens[a].token = variables[b].value;

					definedVariable = true;
					break;
				}
			}

			if (!definedVariable)
				error("variable '" + tokens[a].token + "' undefined");
		}
	}
}

void CheckVariable(const std::vector<Variable>& vars, const Token& var)
{
	for (std::size_t a = 0; a < vars.size(); ++a)
	{
		if (vars[a].name == var.token)
			error("variable '" + var.token + "' redefined");
	}
}

void Parser(std::vector<Token>& tokens)
{
	static std::vector<Variable> variables;

	// variable initialization
	if (tokens.size() >= 5 && tokens[1].type == TokenType::VARIABLE &&
		tokens[2].type == TokenType::ASSIGNMENT && tokens.back().type == TokenType::SEMICOLON)
	{
		ExtractVariable(tokens, variables);

		if (tokens[0].type == TokenType::BOOL_TYPE)
		{
			CheckBool(3, tokens, variables);
			CheckVariable(variables, tokens[1]);

			std::vector<Token> temp(tokens.begin() + 3, tokens.end() - 1);
			variables.push_back(Variable{ tokens[0].token , tokens[1].token, BoolParser(temp) });
		}
		else if (tokens[0].type == TokenType::CHAR_TYPE && tokens[3].type == TokenType::CHAR_VALUE)
		{
			CheckVariable(variables, tokens[1]);
			variables.push_back(Variable{ "char", tokens[1].token, tokens[3].token });
		}
		else if (tokens[0].type == TokenType::INT_TYPE)
		{
			CheckInt(3, tokens, variables);
			CheckVariable(variables, tokens[1]);

			std::vector<Token> temp(tokens.begin() + 3, tokens.end() - 1);
			variables.push_back(Variable{ tokens[0].token , tokens[1].token, IntParser(temp) });
		}
		else if (tokens[0].type == TokenType::STRING_TYPE)
		{
			CheckStr(3, tokens, variables);
			CheckVariable(variables, tokens[1]);

			std::vector<Token> temp(tokens.begin() + 3, tokens.end() - 1);
			variables.push_back(Variable{ tokens[0].token , tokens[1].token, StrParser(temp) });
		}
		else
		{
			error("invalid variable initialization");
		}
	}
	// variable assignment
	else if (tokens.size() >= 4 && tokens[0].type == TokenType::VARIABLE &&
		tokens[1].type == TokenType::ASSIGNMENT && tokens.back().type == TokenType::SEMICOLON)
	{
		bool isDefined = false;
		int index = -1;
		for (std::size_t a = 0; a < variables.size(); ++a)
		{
			if (variables[a].name == tokens[0].token)
			{
				isDefined = true;
				index = a;
				break;
			}
		}

		if (!isDefined || index == -1)
			error("variable '" + tokens[0].token + "' undefined");

		ExtractVariable(tokens, variables, 2);

		if (variables[index].type == "bool")
		{
			CheckBool(3, tokens, variables);

			std::vector<Token> temp(tokens.begin() + 1, tokens.end());
			variables[index].value = BoolParser(temp);
		}
		else if (variables[index].type == "char" && tokens[2].type == TokenType::CHAR_VALUE)
		{
			variables[index].value = tokens[2].token;
		}
		else if (variables[index].type == "int")
		{
			CheckInt(3, tokens, variables);

			std::vector<Token> temp(tokens.begin() + 2, tokens.end() - 1);
			variables[index].value = IntParser(temp);
		}
		else if (variables[index].type == "str")
		{
			CheckStr(3, tokens, variables);

			std::vector<Token> temp(tokens.begin() + 1, tokens.end());
			variables[index].value = StrParser(temp);
		}
		else
		{
			error("invalid variable assignment");
		}
	}
	// print
	else if (tokens.size() >= 3 && tokens[0].type == TokenType::PRINT &&
		tokens.back().type == TokenType::SEMICOLON)
	{
		ExtractVariable(tokens, variables, 1, 1);

		/*if (tokens[1].type == TokenType::VARIABLE)
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
		*/
		if (tokens[1].type == TokenType::BOOL_VALUE)
		{
			CheckBool(1, tokens, variables, TokenType::PRINT);

			std::vector<Token> temp(tokens.begin() + 1, tokens.end());
			std::cout << BoolParser(temp) << '\n';
		}
		else if (tokens[1].type == TokenType::CHAR_VALUE && tokens.size() == 3)
		{
			std::cout << tokens[1].token << '\n';
		}
		else if (tokens[1].type == TokenType::INT_VALUE)
		{
			CheckInt(1, tokens, variables, TokenType::PRINT);

			std::vector<Token> temp(tokens.begin() + 1, tokens.end());
			std::cout << IntParser(temp) << '\n';
		}
		else if (tokens[1].type == TokenType::STRING_VALUE)
		{
			CheckStr(3, tokens, variables, TokenType::PRINT);

			std::vector<Token> temp(tokens.begin() + 1, tokens.end());
			std::cout << StrParser(temp) << '\n';
		}
		else
		{
			error("invalid print statement");
		}
	}
	// if statement
	else if (tokens.size() >= 6 && tokens[0].type == TokenType::IF &&
		tokens[1].type == TokenType::OPEN_BRACKET)
	{
		for (std::size_t a = 0; a < tokens.size(); ++a)
		{
			if (tokens[a].token == "{")
			{
				ExtractVariable(tokens, variables, 2, tokens.size() - (a - 1));
				CheckBool(2, tokens, variables, TokenType::IF, tokens.size() - (a - 1));

				std::vector<Token> temp(tokens.begin() + 2, tokens.begin() + a - 1);
				if (BoolParser(temp) == "true")
				{
					temp.clear();

					int openCurly = 0;
					for (std::size_t b = a + 1; b < tokens.size() - 1; ++b)
					{
						temp.push_back(tokens[b]);

						if (tokens[b].token == "{")
							openCurly += 1;
						else if (tokens[b].token == "}")
							openCurly -= 1;

						if ((tokens[b].token == ";" && openCurly == 0) ||
							(tokens[b].token == "}" && openCurly == 0))
						{
							Parser(temp);
							temp.clear();
						}
					}
				}

				break;
			}
		}
	}
	else
	{
		error("invalid language grammar");
	}
}
