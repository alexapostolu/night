#include <fstream>
#include <iostream>
#include <string>

#include "Lexer.h"

int main()
{
	std::ifstream code("source.night");
	if (code.is_open())
	{
		std::string line;
		while (getline(code, line))
		{
			Lexer(line);
		}
	}
	else
	{
		std::cout << "Error - cannot open file 'source.night'";
		exit(0);
	}
}
