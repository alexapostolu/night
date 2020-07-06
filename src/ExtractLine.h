#pragma once

#include <string>

#include "Lexer.h"

#include "Error.h"

void ExtractLine(std::string& codeLine)
{
	int openCurly = 0;

	std::string line = "";
	for (std::size_t a = 0; a < codeLine.length(); ++a)
	{
		line += (codeLine[a] == '\t' ? ' ' : codeLine[a]);

		if (codeLine[a] == '{')
			openCurly += 1;
		else if (codeLine[a] == '}')
			openCurly -= 1;
		
		if ((codeLine[a] == ';' && openCurly == 0) || (codeLine[a] == '}' && openCurly == 0))
		{
			Lexer(line);
			line = "";
		}
	}

	if (openCurly > 0)
		error("symbol '}' not found");
	else if (openCurly < 0)
		error("symbol '{' not found");
	else if (line != "")
		error("symbol ';' or '}' not found");
}
