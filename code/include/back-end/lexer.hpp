#pragma once

#include "token.hpp"

#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

// one lexer for each file
class Lexer
{
public:
	Lexer(std::string_view file_name, const bool main_file);

public:
	Token eat(const bool go_to_next_line);
	Token peek(const bool go_to_next_line);

	Token get_curr() const noexcept;
	Location get_loc() const noexcept;
	
private:
	bool next_token(const bool go_to_next_line) noexcept;
	bool next_line() noexcept;

	// when a string token has been scanned, this function is called to replace
	// escape strings with escape characters
	void replace_escape_chars(std::string& token) const noexcept;

private:
	std::fstream code_file;
	Location loc;

	std::string code_line;
	std::size_t i;
	
	Token curr;

	static std::unordered_map<char, std::vector<std::pair<char, TokenType> > > const symbols;
	static std::unordered_map<std::string, TokenType> const keywords;
};
