#pragma once

#include "../lib/string.h"
#include "./token.h"

struct Variable
{
	TokenType type;
	night::string name;
	night::string value;
};