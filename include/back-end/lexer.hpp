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

class Lexer
{
public:
	Lexer(
		std::string const& file_name,
		bool		const  main_file
	);

public:
	Token eat(bool const next_line);
	void eat(TokenType const& expect_type, bool const next_line,
		std::string const& stmt_format, std::string const& stmt_learn);
	Token peek(bool const next_line);

	Token get_curr(bool const next_line);
	Location get_loc() const;
	
private:
	bool new_line();

private:
	std::fstream code_file;
	Location loc;

	std::string code_line;
	std::size_t i;
	
	Token curr, next;

	static std::unordered_map<char, std::map<char, TokenType> > const symbols;
	static std::unordered_map<std::string, TokenType> const keywords;
};