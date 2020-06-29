#pragma once

#include <iostream>
#include <string>

#include "Lexer.h"

void ExtractLine(std::string& line)
{
	// removes comments
	std::size_t commentIndex = line.find("//");
	if (commentIndex != std::string::npos)
		line.erase(line.begin() + commentIndex, line.end());

	if (line.empty())
		return;

	// removes whitespace
	for (std::size_t a = line.length() - 1; line[a] == ' ' && a >= 0; --a)
	{
		line.erase(line.begin() + a);

		if (a == 0)
			break;
	}

	// checks for semicolons

	if (!line.empty() && line.back() != ';')
	{
		std::cout << "Error - missing semicolon\n";
		exit(0);
	}

	for (std::size_t a = 0; a < line.length(); ++a)
	{
		if (line[a] == ';' && a == line.length() - 1)
		{
			Lexer(line);
		}
		else if (line[a] == ';' && a != line.length() - 1)
		{
			Lexer(line.substr(0, a + 1));
			line.erase(line.begin(), line.begin() + a + 1);

			a = -1;
		}
	}
}
