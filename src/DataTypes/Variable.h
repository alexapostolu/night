#pragma once

#include <string>

#include "Token.h"

struct Variable
{
	TokenType type;
	std::string name;
	std::string value;
};