#pragma once

#include <string>
#include <vector>

#include "Token.h"

struct Function
{
	std::string name;
	std::vector<Variable> parameters;
	std::vector<Token> functionCode;
};
