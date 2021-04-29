#pragma once

#include "token.hpp"

#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

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

struct Lexer
{
	std::fstream *const code_file;
	Location loc;

	std::string code_line;
	std::size_t i;
	
	Token curr, peek;

	std::unordered_map<char, std::map<char, TokenType> > const symbols;
};

Lexer lexer_create(std::string const& file_name, bool const main_file);
void lexer_new_line(Lexer const& lexer);
Token lexer_eat(Lexer& lexer, bool const next_line);
Token lexer_peek(Lexer& lexer, bool const next_line);
Token lexer_curr(Lexer& lexer, bool const next_line);