#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "Token.h"
#include "Variable.h"

#include "Lexer.h"
#include "Transfer.h"

int main()
{
	std::vector<Token> tokens;
	std::vector<Variable> variables;

	std::ifstream code("code.nt");
	if (code.is_open())
	{
		std::string line;
		while (getline(code, line))
		{
			Lexer(tokens, line);
			Transfer(tokens, variables);

			tokens.clear();
		}
	}
	else
	{
		std::cout << "Error - File 'code.nt' not found";
		exit(0);
	}

	for (std::size_t a = 0; a < tokens.size(); ++a)
	{
		//std::cout << "TOKEN (" << (int)tokens[a].type << "): " << tokens[a].token << "\n";
		//std::cout << tokens[a].token << ' ';
	}

	for (std::size_t a = 0; a < variables.size(); ++a)
	{
		std::cout << variables[a].type << ' ' << variables[a].name << ' ' <<
			variables[a].value << '\n';
	}
}
