#pragma once

#include <iostream>
#include <string>

// stores user output
std::string storeOutput(const std::string& outputStr = "")
{
	static std::string output = "";
	
	for (std::size_t a = 0; a < outputStr.length(); ++a)
	{
		output += (a < outputStr.length() - 1 && outputStr[a] == '\\' && outputStr[a++ + 1] == 'n' ?
			'\n' : outputStr[a]);
	}

	return output;
}

// prints it out in the end
void printOutput(const std::string& output)
{
	std::cout << output;
}