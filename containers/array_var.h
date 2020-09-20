#pragma once

#include "../lib/string.h"
#include "../lib/array.h"

#include "./token.h"

struct Array
{
	TokenType type;
	night::string name;
	night::array<Token> elems;
};