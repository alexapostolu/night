#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "Token.h"
#include "Lexer.h"

int main()
{
	std::vector<Token> tokens;

	std::ifstream code("code.nt");
	if (code.is_open())
	{
		std::string line;
		while (getline(code, line))
		{
			Lexer(tokens, line);
		}
	}
	else
	{
		std::cout << "Error - File 'code.nt' not found";
		exit(0);
	}

	for (std::size_t a = 0; a < tokens.size(); ++a)
	{
		std::cout << "TOKEN (" << (int)tokens[a].type << "): " << tokens[a].token << "\n";
		//std::cout << tokens[a].token << ' ';
	}
}
