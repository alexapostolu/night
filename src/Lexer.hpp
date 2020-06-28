#pragma once

#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include "Token.hpp"
#include "Parser.h"

void check(std::vector<Token>& tokens, std::string& token)
{
	if (token == "bool") {
		tokens.push_back(Token{ TokenTypes::BOOL_TYPE, "bool" });
		token = "";
	}
	else if (token == "char") {
		tokens.push_back(Token{ TokenTypes::CHAR_TYPE, "char" });
		token = "";
	}
	else if (token == "int") {
		tokens.push_back(Token{ TokenTypes::INT_TYPE, "int" });
		token = "";
	}
	else if (token == "false" || token == "true") {
		tokens.push_back(Token{ TokenTypes::BOOL_VALUE, token });
		token = "";
	}
	else if (token.length() == 3 && token[0] == '\'' && token[2] == '\'') {
		tokens.push_back(Token{ TokenTypes::CHAR_VALUE, std::string(1, token[1]) });
		token = "";
	}
	else if (std::regex_match(token, std::regex("[0-9]+"))) {
		tokens.push_back(Token{ TokenTypes::INT_VALUE, token });
		token = "";
	}
	else if (token == "print") {
		tokens.push_back(Token{ TokenTypes::PRINT, "print" });
		token = "";
	}
	else {
		tokens.push_back(Token{ TokenTypes::VARIABLE, token });
		token = "";
	}
}

void Lexer(const std::string& line)
{
	std::vector<Token> tokens;
	std::string token = "";

	for (std::size_t a = 0; a < line.length(); ++a) {
		if (line[a] == '=') {
			check(tokens, token);
			tokens.push_back(Token{ TokenTypes::ASSIGNMENT, "=" });
		}
		else if (line[a] == '+') {
			check(tokens, token);
			tokens.push_back(Token{ TokenTypes::PLUS, "+" });
		}
		else if (line[a] == '-') {
			check(tokens, token);
			tokens.push_back(Token{ TokenTypes::MINUS, "-" });
		}
		else if (line[a] == '*') {
			check(tokens, token);
			tokens.push_back(Token{ TokenTypes::TIMES, "*" });
		}
		else if (line[a] == '/') 
			if (line[a + 1] == '/') { break; }
			check(tokens, token);
			tokens.push_back(Token{ TokenTypes::DIVIDE, "/" });
		}
		else if (line[a] == '%') {
			check(tokens, token);
			tokens.push_back(Token{ TokenTypes::MOD, "%" });
		}
		else if (line[a] == '!') {
			check(tokens, token);
			tokens.push_back(Token{ TokenTypes::NOT, "!" });
		}
		else if (line[a] == '&') {
			if (line[a + 1] == '&')
			{
				check(tokens, token);
				tokens.push_back(Token{ TokenTypes::AND, "&&" });
			}
			else if (line[a - 1] != '&')
			{
				std::cout << "Error - undefined symbol '&'";
				exit(0);
			}
		}
		else if (line[a] == '|') {
			if (line[a + 1] == '|')
			{
				check(tokens, token);
				tokens.push_back(Token{ TokenTypes::AND, "||" });
			}
			else if (line[a - 1] != '|')
			{
				std::cout << "Error - undefined symbol '|'";
				exit(0);
			}
		}
		else if (line[a] == '(') {
			check(tokens, token);
			tokens.push_back(Token{ TokenTypes::OPEN_BRACKET, "(" });
		}
		else if (line[a] == ')') {
			check(tokens, token);
			tokens.push_back(Token{ TokenTypes::CLOSE_BRACKET, ")" });
		}
		else if (line[a] == ';') {
			check(tokens, token);
			tokens.push_back(Token{ TokenTypes::SEMICOLON, ";" });
		}
		else {
			if ((line[a] == ' ' || line[a] == ';') && token != "")
			{
				check(tokens, token);
			}
			else
			{
				if (line[a] != ' ')
					token += line[a];
			}
		}
	}

	for (std::size_t a = 0; a < tokens.size(); ++a) {
		if (tokens[a].token == "" || tokens[a].token == " " || tokens[a].token == "\0")
			tokens.erase(tokens.begin() + a);
	}

	Parser(tokens);
}
