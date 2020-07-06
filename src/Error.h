#pragma once

#include <iostream>
#include <string>

void error(const std::string& message)
{
	std::cout << "Error - " << message << '\n';
	exit(0);
}
