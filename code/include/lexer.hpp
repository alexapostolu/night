#pragma once

#include "token.hpp"
#include "error.hpp"

#include <fstream>
#include <string>

struct Location;

class Lexer
{
public:
	Lexer();
	Lexer(std::string const& _file_name);
	~Lexer();

public:
	Token const& eat();
	Token const& curr() const;
	Token const& expect(TokenType type, std::string const& err = "\n");

	// used for testing
	void scan_code(std::string const& code);

private:
	Token eat_string();
	Token eat_keyword();
	Token eat_number();
	Token eat_symbol();

	// returns false when it reaches end of file
	bool new_line();
	Token eat_new_line();

public:
	Location loc;

private:
	std::fstream file;
	std::string file_line;

	Token curr_tok;
};