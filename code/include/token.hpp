#pragma once

#include <string>

enum class TokenType
{
	UNARY_OP,
	BINARY_OP,

	ASSIGN,
	OPEN_BRACKET,
	CLOSE_BRACKET,
	OPEN_SQUARE,
	CLOSE_SQUARE,
	OPEN_CURLY,
	CLOSE_CURLY,
	COLON,
	SEMICOLON,
	COMMA,

	BOOL_LIT,
	CHAR_LIT,
	INT_LIT,
	FLOAT_LIT,
	STR_LIT,

	VARIABLE,

	BOOL_TYPE,
	CHAR_TYPE,
	INT_TYPE,
	STR_TYPE,

	IF,
	ELIF,
	ELSE,

	FOR,
	WHILE,

	RETURN,

	END_OF_FILE
};

struct Token
{
	TokenType type;
	std::string str;
};