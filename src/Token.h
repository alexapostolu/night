#pragma once

#include <string>

enum class TokenType
{
	BOOL_TYPE,
	CHAR_TYPE,
	INT_TYPE,
	STRING_TYPE,

	VARIABLE,

	ASSIGNMENT,

	BOOL_VALUE,
	CHAR_VALUE,
	INT_VALUE,
	STRING_VALUE,

	PLUS,
	MINUS,
	TIMES,
	DIVIDE,
	MOD,

	NOT,
	OR,
	AND,

	OPEN_BRACKET,
	CLOSE_BRACKET,

	OPEN_CURLY,
	CLOSE_CURLY,

	IF,

	PRINT,

	SEMICOLON
};

struct Token
{
	TokenType type;
	std::string token;
};
