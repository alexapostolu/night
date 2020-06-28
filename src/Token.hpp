#pragma once

#include <string>

enum class TokenTypes
{
	BOOL_TYPE,      // 0
	CHAR_TYPE,      // 1
	INT_TYPE,       // 2

	VARIABLE,       // 3

	ASSIGNMENT,     // 4

	PLUS,           // 5
	MINUS,          // 6
	TIMES,          // 7
	DIVIDE,         // 8
	MOD,            // 9

	NOT,            // 10
	OR,             // 11
	AND,            // 12

	OPEN_BRACKET,   // 13
	CLOSE_BRACKET,  // 14

	BOOL_VALUE,     // 15
	CHAR_VALUE,     // 16
	INT_VALUE,      // 17

	SEMICOLON,      // 18
	PRINT           // 19
};

struct Token
{
	TokenTypes type;
	std::string token;
};
