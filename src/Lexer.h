#pragma once

#include <regex>
#include <string>
#include <vector>

#include "Token.h"
#include "Parser.h"

void Check(std::vector<Token>& tokens, std::string& token)
{
	if (token == "bool")
	{
		tokens.push_back(Token{ TokenType::BOOL_TYPE, "bool" });
		token = "";
	}
	else if (token == "char")
	{
		tokens.push_back(Token{ TokenType::CHAR_TYPE, "char" });
		token = "";
	}
	else if (token == "int")
	{
		tokens.push_back(Token{ TokenType::INT_TYPE, "int" });
		token = "";
	}
	else if (token == "str")
	{
		tokens.push_back(Token{ TokenType::STRING_TYPE, "str" });
		token = "";
	}
	else if (token.length() == 3 && token[0] == '\'' && token[2] == '\'')
	{
		tokens.push_back(Token{ TokenType::CHAR_VALUE, std::string(1, token[1]) });
		token = "";
	}
	else if (token == "false" || token == "true")
	{
		tokens.push_back(Token{ TokenType::BOOL_VALUE, token });
		token = "";
	}
	else if (std::regex_match(token, std::regex("[0-9]+")))
	{
		tokens.push_back(Token{ TokenType::INT_VALUE, token });
		token = "";
	}
	else if (token == "print")
	{
		tokens.push_back(Token{ TokenType::PRINT, "print" });
		token = "";
	}
	else if (std::regex_match(token, std::regex("[a-zA-Z]+")))
	{
		tokens.push_back(Token{ TokenType::VARIABLE, token });
		token = "";
	}
	else if (token != "")
	{
		std::cout << "Error - undefined token '" << token << "'\n";
		exit(0);
	}
}

void Lexer(const std::string& line)
{
	std::string token;
	std::vector<Token> tokens;

	bool isString = false;
	for (std::size_t a = 0; a < line.length(); ++a)
	{
		if (line[a] == '"' && !isString)
		{
			Check(tokens, token);

			isString = true;
			continue;
		}
		else if (line[a] != '"' && isString)
		{
			token += line[a];
			continue;
		}
		else if (line[a] == '"' && isString)
		{
			tokens.push_back(Token{ TokenType::STRING_VALUE, token });
			token = "";

			isString = false;
			continue;
		}
		
		if (line[a] == '=')
		{
			Check(tokens, token);
			tokens.push_back(Token{ TokenType::ASSIGNMENT, "=" });
		}
		else if (line[a] == '+')
		{
			Check(tokens, token);
			tokens.push_back(Token{ TokenType::PLUS, "+" });
		}
		else if (line[a] == '-')
		{
			Check(tokens, token);
			tokens.push_back(Token{ TokenType::MINUS, "-" });
		}
		else if (line[a] == '*')
		{
			Check(tokens, token);
			tokens.push_back(Token{ TokenType::TIMES, "*" });
		}
		else if (line[a] == '/')
		{
			Check(tokens, token);
			tokens.push_back(Token{ TokenType::DIVIDE, "/" });
		}
		else if (line[a] == '%')
		{
			Check(tokens, token);
			tokens.push_back(Token{ TokenType::MOD, "%" });
		}
		else if (line[a] == '!')
		{
			Check(tokens, token);
			tokens.push_back(Token{ TokenType::NOT, "!" });
		}
		else if (line[a] == '|')
		{
			Check(tokens, token);
			tokens.push_back(Token{ TokenType::OR, "|" });
		}
		else if (line[a] == '&')
		{
			Check(tokens, token);
			tokens.push_back(Token{ TokenType::AND, "&" });
		}
		else if (line[a] == '(')
		{
			Check(tokens, token);
			tokens.push_back(Token{ TokenType::OPEN_BRACKET, "(" });
		}
		else if (line[a] == ')')
		{
			Check(tokens, token);
			tokens.push_back(Token{ TokenType::CLOSE_BRACKET, ")" });
		}
		else if (line[a] == ';')
		{
			Check(tokens, token);
			tokens.push_back(Token{ TokenType::SEMICOLON, ";" });
		}
		else
		{
			if ((line[a] == ' ' || line[a] == ';') && token != "")
				Check(tokens, token);
			else if (line[a] != ' ')
					token += line[a];
		}
	}

	Parser(tokens);
}
