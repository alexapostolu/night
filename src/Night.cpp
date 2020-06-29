#include <fstream>
#include <iostream>
#include <string>

#include "ExtractLine.h"

int main()
{
	std::ifstream code("source.night");
	if (code.is_open())
	{
		std::string line;
		while (getline(code, line))
			ExtractLine(line);
	}
	else
	{
		std::cout << "Error - file 'source.ngt' not found\n";
		exit(0);
	}
}
