#pragma once

#include "token.hpp"

#include <string>
#include <vector>

// matches token with keyword
void FindKeyword(
	const std::string& file,
	const int          line,

	std::vector<Token>& tokens,
	std::string& token
);

std::vector<Token> Lexer(
	const std::string& file,
	const int          line,

	const std::string& fileLine
);