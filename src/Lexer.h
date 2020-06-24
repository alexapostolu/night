#pragma once

#include <regex>

#include "Token.h"

void check(std::string& token, std::vector<Token>& tokens)
{
	if (token == "bool")
	{
		tokens.push_back(Token{ TokenTypes::BOOL, "bool" });
		token = "";
	}
	else if (token == "char")
	{
		tokens.push_back(Token{ TokenTypes::CHAR, "char" });
		token = "";
	}
	else if (token == "int")
	{
		tokens.push_back(Token{ TokenTypes::INT, "int" });
		token = "";
	}
	else if (std::regex_match(token, std::regex("[0-9]+")))
	{
		tokens.push_back(Token{ TokenTypes::INT_VALUE, token });
		token = "";
	}
	else
	{
		tokens.push_back(Token{ TokenTypes::VARIABLE, token });
		token = "";
	}
}

void Lexer(std::vector<Token>& tokens, const std::string& line)
{
	std::string token = "";
	for (std::size_t a = 0; a < line.length(); ++a)
	{
		if (line[a] == '=')
		{
			check(token, tokens);
			tokens.push_back(Token{ TokenTypes::ASSIGNMENT, "=" });
		}
		else if (line[a] == '+')
		{
			check(token, tokens);
			tokens.push_back(Token{ TokenTypes::PLUS, "+" });
		}
		else if (line[a] == '-')
		{
			check(token, tokens);
			tokens.push_back(Token{ TokenTypes::MINUS, "-" });
		}
		else if (line[a] == '*')
		{
			check(token, tokens);
			tokens.push_back(Token{ TokenTypes::TIMES, "*" });
		}
		else if (line[a] == '/')
		{
			check(token, tokens);
			tokens.push_back(Token{ TokenTypes::DIVIDE, "/" });
		}
		else if (line[a] == '(')
		{
			check(token, tokens);
			tokens.push_back(Token{ TokenTypes::OPEN_BRACKET, "(" });
		}
		else if (line[a] == ')')
		{
			check(token, tokens);
			tokens.push_back(Token{ TokenTypes::CLOSE_BRACKET, ")" });
		}
		else
		{
			if ((line[a] == ' ' || line[a] == ';') && token != "")
			{
				check(token, tokens);
			}
			else
			{
				if (line[a] != ' ')
					token += line[a];
			}
		}

		if (line[a] == ';')
		{
			tokens.push_back(Token{ TokenTypes::SEMICOLON, ";" });
			token = "";
		}
	}

	for (std::size_t a = 0; a < tokens.size(); ++a)
	{
		if (tokens[a].token == "" || tokens[a].token == " " || tokens[a].token == "\0")
		{
			tokens.erase(tokens.begin() + a);
		}
	}
}
