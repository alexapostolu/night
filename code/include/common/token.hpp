#pragma once

#include <string>

struct Location
{
	std::string file;
	int line, col;
};

enum class TokenType
{
	VARIABLE_INIT,
	PARAMETERS,
	BODY,
	EXPR,

	OPEN_BRACKET,
	CLOSE_BRACKET,
	OPEN_SQUARE,
	CLOSE_SQUARE,
	OPEN_CURLY,
	CLOSE_CURLY,
	COLON,
	SEMICOLON,
	COMMA,

	UNARY_OPERATOR,
	BINARY_OPERATOR,

	BOOL_LIT,
	CHAR_LIT,
	INT_LIT,
	FLOAT_LIT,
	STRING_LIT,

	VARIABLE,

	TYPE,

	IF,
	ELIF,
	ELSE,

	FOR,
	WHILE,

	DEF,
	VOID,
	RETURN,

	END_OF_FILE
};

struct Token
{
	TokenType type;
	std::string str;

	Location loc;
};

namespace night {

std::string to_str(TokenType type);

}