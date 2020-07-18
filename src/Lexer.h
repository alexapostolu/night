#pragma once

#include <regex>
#include <string>
#include <vector>

#include "Parser.h"

#include "Squid.h"

#include "Token.h"

void Check(std::vector<Token>& tokens, std::string& token)
{
	if (token == "bit")
	{
		tokens.push_back(Token{ TokenType::BIT_TYPE, token });
		token = "";
	}
	else if (token == "syb")
	{
		tokens.push_back(Token{ TokenType::SYB_TYPE, token });
		token = "";
	}
	else if (token == "int")
	{
		tokens.push_back(Token{ TokenType::INT_TYPE, token });
		token = "";
	}
	else if (token == "dec")
	{
		tokens.push_back(Token{ TokenType::DEC_TYPE, token });
		token = "";
	}
	else if (token == "str")
	{
		tokens.push_back(Token{ TokenType::STR_TYPE, token });
		token = "";
	}
	else if (token == "true" || token == "false")
	{
		tokens.push_back(Token{ TokenType::BIT_VALUE, token });
		token = "";
	}
	else if (token.length() == 3 && token[0] == '\'' && token[2] == '\'')
	{
		tokens.push_back(Token{ TokenType::SYB_VALUE, std::string(1, token[1]) });
		token = "";
	}
	else if (std::regex_match(token, std::regex("[0-9]+")))
	{
		tokens.push_back(Token{ TokenType::INT_VALUE, token });
		token = "";
	}
	else if (std::regex_match(token, std::regex("[0-9]+[.]?[0-9]+|[0-9]+[.]?[0-9]+")))
	{
		tokens.push_back(Token{ TokenType::DEC_VALUE, token });
		token = "";
	}
	else if (token == "if")
	{
		tokens.push_back(Token{ TokenType::IF, token });
		token = "";
	}
	else if (token == "else")
	{
		tokens.push_back(Token{ TokenType::ELSE, token });
		token = "";
	}
	else if (token == "print")
	{
		tokens.push_back(Token{ TokenType::PRINT, token });
		token = "";
	}
	else if (std::regex_match(token, std::regex("[a-zA-Z]+")))
	{
		tokens.push_back(Token{ TokenType::VARIABLE, token });
		token = "";
	}
	else if (token != "")
	{
		throw _undefined_token_("token '" + token + "' undefined");
	}
}

void Lexer(const std::string& line)
{
	std::string token = "";
	std::vector<Token> tokens;

	int openBracket = 0;

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
			tokens.push_back(Token{ TokenType::STR_VALUE, token });
			token = "";

			isString = false;
			continue;
		}

		switch (line[a])
		{
		case '=':
			if (a < line.length() && line[a + 1] == '=')
			{
				Check(tokens, token);
				tokens.push_back(Token{ TokenType::EQUALS, "==" });

				a += 1;
			}
			else
			{
				Check(tokens, token);
				tokens.push_back(Token{ TokenType::ASSIGNMENT, "=" });
			}

			break;
		case '+':	
			Check(tokens, token);
			tokens.push_back(Token{ TokenType::PLUS, "+" });

			break;
		case '-':
			Check(tokens, token);
			tokens.push_back(Token{ TokenType::MINUS, "-" });
			break;
		case '*':
			Check(tokens, token);
			tokens.push_back(Token{ TokenType::TIMES, "*" });
			break;
		case '/':
			Check(tokens, token);
			tokens.push_back(Token{ TokenType::DIVIDE, "/" });
			break;
		case '%':
			Check(tokens, token);
			tokens.push_back(Token{ TokenType::MOD, "%" });
			break;
		case '!':
			if (a < line.length() && line[a + 1] == '=')
			{
				Check(tokens, token);
				tokens.push_back(Token{ TokenType::NOT_EQUALS, "!=" });

				a += 1;
			}
			else
			{
				Check(tokens, token);
				tokens.push_back(Token{ TokenType::NOT, "!" });
			}

			break;
		case '|':
			if (a < line.length() && line[a + 1] == '|')
			{
				Check(tokens, token);
				tokens.push_back(Token{ TokenType::OR, "||" });

				a += 1;
			}
			else
			{
				throw _undefined_token_("token '" + std::string(1, line[a]) + "' undefined");
			}

			break;
		case '&':
			if (a < line.length() && line[a + 1] == '&')
			{
				Check(tokens, token);
				tokens.push_back(Token{ TokenType::AND, "&&" });

				a += 1;
			}
			else
			{
				throw _undefined_token_("token '" + std::string(1, line[a]) + "' undefined");
			}

			break;
		case '(':
			Check(tokens, token);
			tokens.push_back(Token{ TokenType::OPEN_BRACKET, "(" });
			openBracket += 1;
			break;
		case ')':
			Check(tokens, token);
			tokens.push_back(Token{ TokenType::CLOSE_BRACKET, ")" });
			openBracket -= 1;
			break;
		case '{':
			Check(tokens, token);
			tokens.push_back(Token{ TokenType::OPEN_CURLY, "{" });
			break;
		case '}':
			Check(tokens, token);
			tokens.push_back(Token{ TokenType::CLOSE_CURLY, "}" });
			break;
		case ';':
			Check(tokens, token);
			tokens.push_back(Token{ TokenType::SEMICOLON, ";" });
			break;
		default:
			if (line[a] == ' ' && token != "")
				Check(tokens, token);
			else if (line[a] != ' ')
				token += line[a];
		}
	}

	if (openBracket > 0)
		throw _missing_open_bracket_;
	else if (openBracket < 0)
		throw _missing_close_bracket_;

	Parser(tokens);
}