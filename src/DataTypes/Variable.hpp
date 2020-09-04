#pragma once

#include <string>

#include "Token.hpp"

enum class VariableType
{
	BIT, SYB,
	INT, DEC,
	STR
};

struct Variable
{
	TokenType type; // VariableType
	std::string name;
	std::string value;
};
