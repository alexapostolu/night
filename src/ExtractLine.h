#pragma once

#include <string>

#include "Lexer.h"

#include "Output.h"

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
		throw _missing_close_curly_;
	else if (openCurly < 0)
		throw _missing_open_curly_;
	else if (line != "")
		throw _missing_semicolon_;
}