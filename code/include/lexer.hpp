#pragma once

#include "token.hpp"

#include <fstream>
#include <string>

class Lexer
{
public:
	Lexer(std::string_view file);

public:
	Token eat();

private:
	Token eat_string();
	Token eat_keyword();
	Token eat_number();
	Token eat_symbol();

	// returns false when it reaches end of file
	bool new_line();
	Token eat_new_line();

private:
	std::fstream file;
	std::string file_line;
	std::size_t i; // file line index
};