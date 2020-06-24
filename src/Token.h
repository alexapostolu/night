#pragma once

enum class TokenTypes
{
	BOOL,
	CHAR,
	INT,
	VARIABLE,
	ASSIGNMENT,
	INT_VALUE,
	BOOL_VALUE,
	CHAR_VALUE,
	PLUS,
	MINUS,
	TIMES,
	DIVIDE,
	OPEN_BRACKET,
	CLOSE_BRACKET,
	SEMICOLON
};

struct Token
{
	TokenTypes type;
	std::string token;
};
