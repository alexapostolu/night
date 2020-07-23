#pragma once

#include <string>
#include <vector>

#include "Token.h"

struct Function
{
	std::string name;

	std::vector<Token> functionCode;
};