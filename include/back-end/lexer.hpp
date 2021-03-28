#pragma once

#include "token.hpp"

#include <string>
#include <vector>

void ReplaceEscape(
	std::string& token,
	const std::string& str,
	const char ch
);

// matches token with keyword
void FindKeyword(
	const Location& loc,

	std::vector<Token>& tokens,
	std::string& token
);

std::vector<Token> Lexer(
	const Location& loc,
	const std::string& file_ine
);