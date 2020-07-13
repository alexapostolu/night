#include <fstream>
#include <string>

#include "CleanLine.h"
#include "ExtractLine.h"

#include "Output.h"

int main()
{
	std::ifstream code("source.night");
	if (code.is_open())
	{
		std::string fileLine = "", codeLine = "";
		while (getline(code, fileLine))
			CleanLine(fileLine, codeLine);

		ExtractLine(codeLine);

		printOutput(storeOutput(""));

		code.close();
		return 0;
	}
	else
	{
		return error("file 'source.night' not found");
	}
}