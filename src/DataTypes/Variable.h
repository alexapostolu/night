<<<<<<< HEAD
#pragma once

#include <string>

#include "Token.h"

struct Variable
{
	TokenType type;
	std::string name;
	std::string value;
=======
#pragma once

#include <string>

#include "Token.h"

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
>>>>>>> test1
};