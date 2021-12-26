#pragma once

#include "token.hpp"

#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

// one lexer for each file
class Lexer
{
public:
	Lexer(std::string_view file_name, bool main_file);

public:
	Token eat(bool go_to_next_line);

	Token get_curr() const;
	Location get_loc() const;
	
private:
	bool next_token(bool go_to_next_line);
	bool next_line();

	// when a string token has been scanned, this function is called to replace
	// escape strings with escape characters
	void replace_escape_chars(std::string& token) const;

private:
	std::fstream code_file;
	Location loc;

	std::string code_line;
	std::size_t i;
	
	Token curr;

	static std::unordered_map<char, std::vector<std::pair<char, TokenType> > > const symbols;
	static std::unordered_map<std::string, TokenType> const keywords;
};