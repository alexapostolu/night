#pragma once

#include "../lib/string.h"
#include "../lib/array.h"

#include "./variable.h"
#include "./token.h"

struct Function
{
	TokenType type;
	night::string name;
	night::array<Variable> params;
	night::array<Token> code;
	Token returnVal;
};