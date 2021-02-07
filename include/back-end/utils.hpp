#pragma once

#include "token.hpp"
#include "../error.hpp"

#include <vector>

bool find_type(
	const std::vector<VariableType>& container,
	const VariableType& type
);

bool find_num_types(
	const std::vector<VariableType>& container
);

// splits a 1D array of tokens into a 2d array based on individual statements
std::vector<std::vector<Token> > SplitCode(
	const std::vector<Token>& tokens
);

std::string get_var_types_as_str(
	const std::vector<VariableType>& types
);