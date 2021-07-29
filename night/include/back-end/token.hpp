#pragma once

#include "../error.hpp"

#include <string>

enum class TokenType
{
	UNARY_OP, BINARY_OP,

	OPEN_BRACKET, CLOSE_BRACKET,
	OPEN_SQUARE, CLOSE_SQUARE,
	OPEN_CURLY, CLOSE_CURLY,

	ASSIGN,

	COLON, COMMA,

	// literals
	BOOL_L, INT_L, FLOAT_L, STR_L,

	VAR,

	SET,

	// types
	BOOL_T, INT_T, FLOAT_T, STR_T,

	IF, ELIF, ELSE,
	SWITCH,

	WHILE, FOR,

	FN,
	RETURN,

	IMPORT,

	EOL, _EOF
};

struct Token
{
	Location loc;

	TokenType type;
	std::string data;

	bool is_value() const;

	static Token const _EOL;
	static Token const _EOF;
};