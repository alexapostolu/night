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
	if (newline != std::string::npos)
	{
		output[newline] = '\n';
		output.erase(output.begin() + newline + 1);
	}

	std::cout << output;
}