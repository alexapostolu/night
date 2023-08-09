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
	STRING_LIT,

	VARIABLE,

	BOOL_TYPE,
	CHAR_TYPE,
	INT_TYPE,
	FLOAT_TYPE,
	STRING_TYPE,

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

	bool is_type() const;
};

namespace night
{

std::string to_str(TokenType type);

}