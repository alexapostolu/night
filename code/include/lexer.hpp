#pragma once

#include "token.hpp"
#include "error.hpp"

#include <source_location>
#include <fstream>
#include <string>
#include <optional>

struct Location;

class Lexer
{
public:
	Lexer() = default;
	Lexer(std::string const& _file_name);
	~Lexer();

public:
	Token const& eat();
	Token peek();
	Token const& curr() const;

	// Eats token, and checks the type of new current token. Returns current if
	// successful.
	Token const& expect(
		TokenType type,
		std::string const& err = "\n",
		std::source_location const& s_loc = std::source_location::current());

	Token const& curr_is(
		TokenType type,
		std::string const& err_msg = "",
		std::source_location const& s_loc = std::source_location::current());

	// Used for testing.
	void scan_code(std::string const& code);

private:
	Token eat_string();
	Token eat_character();
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
	std::optional<Token> prev_tok;
};