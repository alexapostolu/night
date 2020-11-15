#pragma once

#include "token.h"
#include "error.h"

#include <string>
#include <vector>
#include <unordered_map>

// matches token with keyword
void FindKeyword(const std::string& file, int line, const std::unordered_map<std::string, TokenType>& keywords,
	std::vector<Token>& tokens, std::string& token);

// scans code
std::vector<Token> Lexer(const std::string& file, int line, const std::string& fileLine);