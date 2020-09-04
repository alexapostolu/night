#pragma once

#include <regex>
#include <string>
#include <vector>

#include "Parser/Parser.hpp"

#include "DataTypes/Error.hpp"
#include "DataTypes/Token.hpp"

void CheckToken(std::vector<Token>& tokens, std::string& token)
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
	else if (token == "null")
	{
		tokens.push_back(Token{ TokenType::NULL_TYPE, token });
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
	else if (token == "return")
	{
		tokens.push_back(Token{ TokenType::RETURN, token });
		token = "";
	}
	else if (token == "loop")
	{
		tokens.push_back(Token{ TokenType::LOOP, token });
		token = "";
	}
	else if (std::regex_match(token, std::regex("[a-zA-Z_][a-zA-Z_0-9]*")))
	{
		tokens.push_back(Token{ TokenType::VARIABLE, token });
		token = "";
	}
}

void Lexer(const std::string& line)
{
	std::string token = "";
	std::vector<Token> tokens;

	bool isString = false;
	int openBracket = 0, openSquare = 0, openCurly = 0;
	for (std::size_t a = 0; a < line.length(); ++a)
	{
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
				CheckToken(tokens, token);
				tokens.push_back(Token{ TokenType::EQUALS, "==" });

				a += 1;
			}
			else
			{
				CheckToken(tokens, token);
				tokens.push_back(Token{ TokenType::ASSIGNMENT, "=" });
			}

			break;
		case '+':
			if (a < line.length() && line[a + 1] == '=')
			{
				CheckToken(tokens, token);
				tokens.push_back(Token{ TokenType::PLUS_ASSIGN, "+=" });

				a += 1;
			}
			else
			{
				CheckToken(tokens, token);
				tokens.push_back(Token{ TokenType::PLUS, "+" });
			}

			break;
		case '-':
			if (a < line.length() && line[a + 1] == '=')
			{
				CheckToken(tokens, token);
				tokens.push_back(Token{ TokenType::MINUS_ASSIGN, "-=" });

				a += 1;
			}
			else
			{
				CheckToken(tokens, token);
				tokens.push_back(Token{ TokenType::MINUS, "-" });
			}

			break;
		case '*':
			if (a < line.length() && line[a + 1] == '=')
			{
				CheckToken(tokens, token);
				tokens.push_back(Token{ TokenType::TIMES_ASSIGN, "*=" });

				a += 1;
			}
			else
			{
				CheckToken(tokens, token);
				tokens.push_back(Token{ TokenType::TIMES, "*" });
			}

			break;
		case '/':
			if (a < line.length() && line[a + 1] == '=')
			{
				CheckToken(tokens, token);
				tokens.push_back(Token{ TokenType::DIVIDE_ASSIGN, "/=" });

				a += 1;
			}
			else
			{
				CheckToken(tokens, token);
				tokens.push_back(Token{ TokenType::DIVIDE, "/" });
			}

			break;
		case '%':
			if (a < line.length() && line[a + 1] == '=')
			{
				CheckToken(tokens, token);
				tokens.push_back(Token{ TokenType::MOD_ASSIGN, "%=" });

				a += 1;
			}
			else
			{
				CheckToken(tokens, token);
				tokens.push_back(Token{ TokenType::MOD, "%" });
			}

			break;
		case '!':
			CheckToken(tokens, token);
			if (a < line.length() && line[a + 1] == '=')
			{
				tokens.push_back(Token{ TokenType::NOT_EQUALS, "!=" });
				a += 1;
			}
			else
			{
				tokens.push_back(Token{ TokenType::NOT, "!" });
			}

			break;
		case '|':
			if (a < line.length() && line[a + 1] == '|')
			{
				CheckToken(tokens, token);
				tokens.push_back(Token{ TokenType::OR, "||" });

				a += 1;
			}

			break;
		case '&':
			if (a < line.length() && line[a + 1] == '&')
			{
				CheckToken(tokens, token);
				tokens.push_back(Token{ TokenType::AND, "&&" });

				a += 1;
			}

			break;
		case '>':
			CheckToken(tokens, token);

			if (a < line.length() && line[a + 1] == '=')
			{
				tokens.push_back(Token{ TokenType::GREATER_EQUAL, "<=" });
				a += 1;
			}
			else
			{
				tokens.push_back(Token{ TokenType::GREATER, ">" });
			}

			break;
		case '<':
			CheckToken(tokens, token);

			if (a < line.length() && line[a + 1] == '=')
			{
				tokens.push_back(Token{ TokenType::SMALLER_EQUAL, "<=" });
				a += 1;
			}
			else
			{
				tokens.push_back(Token{ TokenType::SMALLER, "<" });
			}

			break;
		case '(':
			CheckToken(tokens, token);
			tokens.push_back(Token{ TokenType::OPEN_BRACKET, "(" });
			openBracket += 1;
			break;
		case ')':
			CheckToken(tokens, token);
			tokens.push_back(Token{ TokenType::CLOSE_BRACKET, ")" });
			openBracket -= 1;
			break;
		case '[':
			CheckToken(tokens, token);
			tokens.push_back(Token{ TokenType::OPEN_SQUARE });
			openSquare += 1;
			break;
		case ']':
			CheckToken(tokens, token);
			tokens.push_back(Token{ TokenType::CLOSE_SQUARE });
			openSquare -= 1;
			break;
		case '{':
			CheckToken(tokens, token);
			tokens.push_back(Token{ TokenType::OPEN_CURLY, "{" });
			openCurly += 1;
			break;
		case '}':
			CheckToken(tokens, token);
			tokens.push_back(Token{ TokenType::CLOSE_CURLY, "}" });
			openCurly -= 1;
			break;
		case ',':
			CheckToken(tokens, token);
			tokens.push_back(Token{ TokenType::COMMA, "," });
			break;
		case ';':
			CheckToken(tokens, token);
			tokens.push_back(Token{ TokenType::SEMICOLON, ";" });
			break;
		case ' ':
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

	if (openBracket > 0) {
		throw Error(night::_invalid_expression_, tokens, 0, tokens.size() - 1,
			"closing bracket is missing");
	}
	else if (openBracket < 0) {
		throw Error(night::_invalid_expression_, tokens, 0, tokens.size() - 1,
			"opening bracket is missing");
	}
	else if (openSquare > 0) {
		throw Error(night::_invalid_expression_, tokens, 0, tokens.size() - 1,
			"closing square bracket is missing");
	}
	else if (openSquare < 0) {
		throw Error(night::_invalid_expression_, tokens, 0, tokens.size() - 1,
			"opening square bracket is missing");
	}
	else if (openCurly > 0) {
		throw Error(night::_invalid_expression_, tokens, 0, tokens.size() - 1,
			"closing curly bracket is missing");
	}
	else if (openCurly < 0) {
		throw Error(night::_invalid_expression_, tokens, 0, tokens.size() - 1,
			"opening curly bracket is missing");
	}

	Parser(tokens, false);
}
