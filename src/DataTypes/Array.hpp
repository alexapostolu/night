#pragma once

#include <string>
#include <vector>

#include "Token.hpp"

struct Array
{
	std::string name;
	std::vector<Token> elements;
};
