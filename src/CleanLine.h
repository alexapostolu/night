#pragma once

#include <string>

void CleanLine(std::string& fileLine, std::string& codeLine)
{
	// removes comments
	std::size_t commentIndex = fileLine.find("//");
	if (commentIndex != std::string::npos)
		fileLine.erase(fileLine.begin() + commentIndex, fileLine.end());

	if (fileLine.empty())
		return;

	// removes whitespace
	for (std::size_t a = fileLine.length() - 1; fileLine[a] == ' ' && a >= 0; --a)
	{
		fileLine.erase(fileLine.begin() + a);

		if (a == 0)
			break;
	}

	codeLine += fileLine;
}
