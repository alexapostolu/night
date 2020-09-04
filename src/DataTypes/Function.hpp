#pragma once

#include <string>
#include <vector>

#include "Token.hpp"
#include "Variable.hpp"

struct Function
{
	TokenType type;
	std::string name;
	std::vector<Variable> parameters;
	std::vector<Token> code;
};
