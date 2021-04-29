#pragma once

#include "token.hpp"
#include "../error.hpp"

#include <vector>
#include <string>

bool find_num_types(
	const VariableTypeContainer& container
);

// splits a 1D array of tokens into a 2d array based on individual statements
std::vector<std::vector<Token> > SplitCode(
	const std::vector<Token>& tokens
);

std::string get_types_as_str(
	const ValueContainer& types
);

struct Location
{
	std::string file;
	int line, col;
};