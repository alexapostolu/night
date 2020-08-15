#pragma once

#include <string>

namespace {

std::string output = "";

}

void StoreOutput(const std::string& text)
{
	output += text;
}

void PrintOutput()
{
	std::size_t newline = output.find("\\n");
	while (newline != std::string::npos)
	{
		output[newline] = '\n';
		output.erase(output.begin() + newline + 1);

		newline = output.find("\\n");
	}

	std::cout << output;
}