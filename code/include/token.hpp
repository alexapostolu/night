#pragma once

#include <string>

enum class TokenType
{
	END_OF_FILE
};

struct Token
{
	TokenType type;
	std::string val;
};