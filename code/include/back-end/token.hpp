#pragma once

#include "error.hpp"

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

	LET,

	// types
	BOOL_T, INT_T, FLOAT_T, STR_T,

	IF, ELIF, ELSE,

	LOOP,

	FN,
	RETURN,

	EOL, _EOF
};

struct Token
{
	Location loc;

	TokenType type;
	std::string data;

	constexpr bool is_value() const
	{
		return type == TokenType::BOOL_L  || type == TokenType::INT_L ||
			   type == TokenType::FLOAT_L || type == TokenType::STR_L ||
			   type == TokenType::VAR     || type == TokenType::OPEN_SQUARE;
	}

	constexpr bool is_operator() const
	{
		return type == TokenType::UNARY_OP || type == TokenType::BINARY_OP;
	}

	static Token const _EOL;
	static Token const _EOF;
};
