#pragma once

#include "lib/string.h"
#include "lib/array.h"
#include "lib/error.h"

#include "containers/token.h"

bool is_letter(char c)
{
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool match_character(night::string& token)
{
	if (token.length() == 4 && token[1] == '\\' && token[2] == 'n')
	{
		token = '\n';
		return true;
	}

	if (token.length() != 3)
		return false;

	if (token[0] == '\'' && token[2] == '\'')
	{
		token = token[1];
		return true;
	}

	return false;
}

int match_number(const night::string& token)
{
	int decimalCount = 0;
	for (int a = 0; a < token.length(); ++a)
	{
		if ((token[a] - '0' < 0 || token[a] - '0' > 9) && token[a] != '.')
			return -1;

		if (token[a] == '.' && ++decimalCount > 1)
			return -1;
	}

	return decimalCount == 1;
}

bool match_variable(const night::string& token)
{
	if (token[0] != '_' && !is_letter(token[0]))
		return false;

	for (int a = 1; a < token.length(); ++a)
	{
		if (token[a] != '_' && !is_letter(token[a]) && (token[a] - '0' < 0 || token[a] - '0' > 9))
			return false;
	}

	return true;
}

void AddKeyword(night::array<Token>& tokens, night::string& token, TokenType&& type)
{
	tokens.add_back(Token{ type, token });
	token = "";
}

void CheckToken(night::array<Token>& tokens, night::string& token)
{
	if (token.length() == 0)
		return;

	if (token == "bit")
		AddKeyword(tokens, token, TokenType::BIT_TYPE);
	else if (token == "syb")
		AddKeyword(tokens, token, TokenType::SYB_TYPE);
	else if (token == "int")
		AddKeyword(tokens, token, TokenType::INT_TYPE);
	else if (token == "dec")
		AddKeyword(tokens, token, TokenType::DEC_TYPE);
	else if (token == "str")
		AddKeyword(tokens, token, TokenType::STR_TYPE);
	else if (token == "null")
		AddKeyword(tokens, token, TokenType::NULL_TYPE);
	else if (token == "true" || token == "false")
		AddKeyword(tokens, token, TokenType::BIT_VALUE);
	else if (token == "if")
		AddKeyword(tokens, token, TokenType::IF);
	else if (token == "else")
		AddKeyword(tokens, token, TokenType::ELSE);
	else if (token == "return")
		AddKeyword(tokens, token, TokenType::RETURN);
	else if (token == "loop")
		AddKeyword(tokens, token, TokenType::LOOP);
	else if (token == "while")
		AddKeyword(tokens, token, TokenType::WHILE);
	else if (token == "for")
		AddKeyword(tokens, token, TokenType::FOR);
	else if (token == "import" || token == "include")
		AddKeyword(tokens, token, TokenType::IMPORT);
	else if (match_character(token))
		AddKeyword(tokens, token, TokenType::SYB_VALUE);
	else if (match_number(token) == 0)
		AddKeyword(tokens, token, TokenType::INT_VALUE);
	else if (match_number(token) == 1)
		AddKeyword(tokens, token, TokenType::DEC_VALUE);
	else if (match_variable(token))
		AddKeyword(tokens, token, TokenType::VARIABLE);
}

void AddSymbol(night::array<Token>& tokens, night::string& token, char symbol, TokenType&& type)
{
	CheckToken(tokens, token);
	tokens.add_back(Token{ type, symbol });
}

night::array<Token> Lexer(const night::string& line)
{
	night::string token;
	night::array<Token> tokens;

	bool isString = false;
	for (int a = 0; a < line.length(); ++a)
	{
		if (a < line.length() - 1 && line[a] == '/' && line[a + 1] == '/')
			break;

		if (line[a] == '"' && !isString)
		{
			CheckToken(tokens, token);

			isString = true;
			continue;
		}
		else if (line[a] != '"' && isString)
		{
			token += line[a] == '\t' ? ' ' : line[a];
			continue;
		}
		else if (line[a] == '"' && isString)
		{
			AddKeyword(tokens, token, TokenType::STR_VALUE);

			isString = false;
			continue;
		}

		switch (line[a])
		{
		case '=':
			CheckToken(tokens, token);
			if (a < line.length() - 1 && line[a + 1] == '=')
			{
				tokens.add_back(Token{ TokenType::EQUALS, "==" });
				a++;
			}
			else
			{
				tokens.add_back(Token{ TokenType::ASSIGNMENT, "=" });
			}

			break;
		case '+':
			CheckToken(tokens, token);
			if (a < line.length() - 1 && line[a + 1] == '=')
			{
				tokens.add_back(Token{ TokenType::PLUS_ASSIGN, "+=" });
				a++;
			}
			else
			{
				tokens.add_back(Token{ TokenType::PLUS, "+" });
			}

			break;
		case '-':
			CheckToken(tokens, token);
			if (a < line.length() - 1 && line[a + 1] == '=')
			{
				tokens.add_back(Token{ TokenType::MINUS_ASSIGN, "-=" });
				a++;
			}
			else
			{
				tokens.add_back(Token{ TokenType::MINUS, "-" });
			}

			break;
		case '*':
			CheckToken(tokens, token);
			if (a < line.length() - 1 && line[a + 1] == '=')
			{
				tokens.add_back(Token{ TokenType::TIMES_ASSIGN, "*=" });
				a++;
			}
			else
			{
				tokens.add_back(Token{ TokenType::TIMES, "*" });
			}

			break;
		case '/':
			CheckToken(tokens, token);
			if (a < line.length() - 1 && line[a + 1] == '=')
			{
				tokens.add_back(Token{ TokenType::DIVIDE_ASSIGN, "/=" });
				a++;
			}
			else
			{
				tokens.add_back(Token{ TokenType::DIVIDE, "/" });
			}

			break;
		case '%':
			CheckToken(tokens, token);
			if (a < line.length() - 1 && line[a + 1] == '=')
			{
				tokens.add_back(Token{ TokenType::MOD_ASSIGN, "%=" });
				a++;
			}
			else
			{
				tokens.add_back(Token{ TokenType::MOD, "%" });
			}

			break;
		case '!':
			CheckToken(tokens, token);
			if (a < line.length() - 1 && line[a + 1] == '=')
			{
				tokens.add_back(Token{ TokenType::NOT_EQUALS, "!=" });
				a++;
			}
			else
			{
				tokens.add_back(Token{ TokenType::NOT, "!" });
			}

			break;
		case '|':
			if (a < line.length() - 1 && line[a + 1] == '|')
			{
				CheckToken(tokens, token);
				tokens.add_back(Token{ TokenType::OR, "||" });

				a++;
			}

			break;
		case '&':
			if (a < line.length() - 1 && line[a + 1] == '&')
			{
				CheckToken(tokens, token);
				tokens.add_back(Token{ TokenType::AND, "&&" });

				a++;
			}

			break;
		case '>':
			CheckToken(tokens, token);
			if (a < line.length() && line[a + 1] == '=')
			{
				tokens.add_back(Token{ TokenType::GREATER_EQUAL, "<=" });
				a++;
			}
			else
			{
				tokens.add_back(Token{ TokenType::GREATER, ">" });
			}

			break;
		case '<':
			CheckToken(tokens, token);
			if (a < line.length() && line[a + 1] == '=')
			{
				tokens.add_back(Token{ TokenType::SMALLER_EQUAL, "<=" });
				a++;
			}
			else
			{
				tokens.add_back(Token{ TokenType::SMALLER, "<" });
			}

			break;
		case '(':
			AddSymbol(tokens, token, line[a], TokenType::OPEN_BRACKET);
			break;
		case ')':
			AddSymbol(tokens, token, line[a], TokenType::CLOSE_BRACKET);
			break;
		case '[':
			AddSymbol(tokens, token, line[a], TokenType::OPEN_SQUARE);
			break;
		case ']':
			AddSymbol(tokens, token, line[a], TokenType::CLOSE_SQUARE);
			break;
		case '{':
			AddSymbol(tokens, token, line[a], TokenType::OPEN_CURLY);
			break;
		case '}':
			AddSymbol(tokens, token, line[a], TokenType::CLOSE_CURLY);
			break;
		case ',':
			AddSymbol(tokens, token, line[a], TokenType::COMMA);
			break;
		case ';':
			AddSymbol(tokens, token, line[a], TokenType::SEMICOLON);
			break;
		case ' ':
			CheckToken(tokens, token);
			break;
		case '\n':
			CheckToken(tokens, token);
			break;
		case '\t':
			CheckToken(tokens, token);
			break;
		default:
			token += line[a];
		}
	}

	CheckToken(tokens, token);
	if (token != "")
		throw Error(night::_invalid_token_, tokens, tokens.length() - 1, tokens.length() - 1, "token "_s + token + "' is not a valid token"_s);

	return tokens;
}