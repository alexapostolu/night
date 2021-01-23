#pragma once

#include "../error.hpp"

void FrontEnd(const int argc)
{
	if (argc != 2)
		throw FrontError("invalid command line arguments; only pass in the file name as an argument");
}