#pragma once

#include <fstream>
#include <algorithm>
#include <string>

#include "Lexer.hpp"
#include "Runtime.hpp"
#include "Output.hpp"

#include "DataTypes/Error.hpp"

void ExtractCode(char* f_arg) /*ExtractCode Function, takes char*
 f_arg as the argument*/
{
	std::ifstream code(f_arg);
	if (!code.is_open())
		throw "source file no open";

	std::string fileLine, codeLine = "";
	while (getline(code, fileLine))
	{
		std::size_t commentPos = fileLine.find("//");
		if (commentPos != std::string::npos)
			fileLine.erase(fileLine.begin() + commentPos, fileLine.end());

		codeLine += fileLine;
	}

	int curlyBracket = 0;
	for (std::size_t a = 0; a < codeLine.length(); ++a)
	{
		if (codeLine[a] == '{')
			curlyBracket += 1;
		else if (codeLine[a] == '}')
			curlyBracket -= 1;

		if ((codeLine[a] == ';' || codeLine[a] == '}') && curlyBracket == 0)
		{
			std::string temp = codeLine.substr(0, a + 1);
			codeLine.erase(codeLine.begin(), codeLine.begin() + a + 1);

			Lexer(temp);// , false);

			a = -1;
		}
	}

	if (curlyBracket < 0)
		throw "";
	else if (curlyBracket > 0)
		throw "";

	codeLine.erase(remove_if(codeLine.begin(), codeLine.end(), ::isspace), codeLine.end());

	std::vector<Token> temp;
	if (codeLine != "") {
		throw Error(night::_invalid_grammar_, temp, 0, 0,
			"missing semicolon");
	}

	Parser(temp, true);
	//Lexer("", true);

	PrintOutput();
}
