#pragma once

#include <vector>

#include "Token.h"
#include "VariableParser.h"

void Transfer(const std::vector<Token>& tokens, std::vector<Variable>& variables)
{
	if ((int)tokens[0].type <= 2 && tokens[1].type == TokenTypes::VARIABLE &&
		tokens[2].type == TokenTypes::SEMICOLON && tokens.size() == 3)
	{
		VariableDeclaration(tokens, variables);
	}
	else if ((int)tokens[0].type <= 2 && tokens[1].type == TokenTypes::VARIABLE &&
		tokens[2].type == TokenTypes::ASSIGNMENT && tokens[tokens.size() - 1].type == 
		TokenTypes::SEMICOLON)
	{
		for (std::size_t a = 3; a < tokens.size() - 1; ++a)
		{
			if ((int)tokens[a].type != 3 && ((int)tokens[a].type < 5 || (int)tokens[a].type > 13))
			{
				std::cout << "Error";
				exit(0);
			}
		}

		VariableInitialization(tokens, variables);
	}
}
