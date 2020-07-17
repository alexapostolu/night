#include <fstream>
#include <iostream>
#include <string>

#include "CleanLine.h"
#include "ExtractLine.h"

#include "Squid.h"
#include "Output.h"

int main()
{
	std::ifstream code("source.night");
	if (code.is_open())
	{
		std::string fileLine = "", codeLine = "";
		while (getline(code, fileLine))
			CleanLine(fileLine, codeLine);

		try {
			ExtractLine(codeLine);
		}
		catch (const squid& s) {
			std::cout << s.what() << '\n';
			return 1;
		}
		catch (int excNum) {
			SquidError(excNum);
			return 1;
		}
		catch (...) {
			SquidError(0);
			return 1;
		}

		printOutput(storeOutput());

		code.close();
		return 0;
	}
	else
	{
		SquidError(1);
		return 1;
	}
}