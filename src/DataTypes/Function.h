<<<<<<< HEAD
#pragma once

#include <string>
#include <vector>

#include "Token.h"
#include "Variable.h"

struct Function
{
	TokenType type;
	std::string name;
	std::vector<Variable> parameters;
	std::vector<Token> code;
=======
#pragma once

#include <string>
#include <vector>

#include "Token.h"
#include "Variable.h"

struct Function
{
	TokenType type;
	std::string name;
	std::vector<Variable> parameters;
	std::vector<Token> code;
>>>>>>> test1
};