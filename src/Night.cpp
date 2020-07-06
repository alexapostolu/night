#include <fstream>
#include <string>

#include "CleanLine.h"
#include "ExtractLine.h"

#include "Error.h"

int main()
{
	std::ifstream codeFile("source.night");
	if (codeFile.is_open())
	{
		std::string fileLine = "", codeLine = "";
		while (getline(codeFile, fileLine))
			CleanLine(fileLine, codeLine);

		ExtractLine(codeLine);

		codeFile.close();
	}
	else
	{
		error("source file 'source.night' not found");
	}
}
