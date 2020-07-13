#pragma once

#include <string>

enum class TokenType
{
	BIT_TYPE,
	SYB_TYPE,
	INT_TYPE,
	DEC_TYPE,
	STR_TYPE,
	
	VARIABLE,

	ASSIGNMENT,

	BIT_VALUE,
	SYB_VALUE,
	INT_VALUE,
	DEC_VALUE,
	STR_VALUE,

	PLUS,
	MINUS,
	TIMES,
	DIVIDE,
	MOD,

	NOT,
	OR,
	AND,
	EQUALS,

	OPEN_BRACKET,
	CLOSE_BRACKET,

	OPEN_CURLY,
	CLOSE_CURLY,

	IF,
	ELSE,

	PRINT,

	SEMICOLON
};

std::string strTokens[] = { "BIT_TYPE", "SYB_TYPE",
"INT_TYPE", "DEC_TYPE", "STR_TYPE", "VARIABLE", "ASSIGNMENT", "BIT_VALUE", "SYB_VALUE", "INT_VALUE", "DEC_VALUE", "STR_VALUE", "PLUS", "MINUS", "TIMES", "DIVIDE", "MOD", "NOT", "OR", "AND", "EQUALS", "OPEN_BRACKET", "CLOSE_BRACKET", "OPEN_CURLY", "CLOSE_CURLY", "IF", "ELSE", "PRINT", "SEMICOLON" };

struct Token
{
	TokenType type;
	std::string token;
};