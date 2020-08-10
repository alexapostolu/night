#pragma once

#include <exception>
#include <sstream>
#include <string>
#include <vector>

#include "Token.h"

const std::string WHITE = "\033[38;5;253m";
const std::string BLUE  = "\033[38;5;39m";
const std::string RED   = "\033[38;5;160m";
const std::string I_RED = "\033[38;5;196m";
const std::string U_RED = "\033[38;5;160;4m";
const std::string RESET = "\033[0m";

namespace night {

const std::string _undefined_token_ = "token is not defined";
const std::string _token_redefinition_ = "token is redefined";
const std::string _invalid_expression_ = "expression is invalid";
const std::string _invalid_if_statement_ = "if statement is invalid";
const std::string _invalid_grammar_ = "language grammar is invalid";

}

class Error
{
friend void Parser(std::vector<Token>&, bool, bool);

private:
	std::string error;
	std::vector<Token> code;
	unsigned int start, end;
	std::string desc;

public:
	Error(const std::string& _error, const std::vector<Token>& _code,
		unsigned int _start, unsigned int _end, const std::string& _desc)
		: error(_error), code(_code), start(_start), end(_end), desc(_desc)
	{
		for (std::size_t a = 0; a < code.size(); ++a)
		{
			if (code[a].type == TokenType::STR_VALUE)
				code[a].token = "\"" + code[a].token + "\"";
			else if (code[a].type == TokenType::SYB_VALUE)
				code[a].token = "'" + code[a].token + "'";
		}

		// create syntax highlighting here
	}

	static std::string DevError(const std::exception& e)
	{
		std::stringstream output;
		output << "Unknown Error - " << e.what() << "\n\n";
		output << "Oops! Something went wrong! Please submit an issue on the GitHub page:\n";
		output << "https://github.com/DynamicSquid/Night";

		return output.str();
	}

	std::string what() const
	{
		std::stringstream output;

		output << I_RED << "Error - " << RESET;
		output << U_RED << error << "\n\n" << RESET;

		for (std::size_t a = 0; a < code.size(); ++a)
			output << (a >= start && a <= end ? RED : WHITE) << code[a].token << ' ';

		output << '\n';

		for (std::size_t a = 0; a < code.size(); ++a)
		{
			for (std::size_t b = 0; b < code[a].token.length(); ++b)
				output << RESET << (a < start || a > end ? ' ' : '~');
			output << ((a < start || a > end) || (a == end) ? ' ' : '~');
		}

		output << "\n\n";

		bool inside = false;
		int firstError = 0;
		for (const char& c : desc)
		{
			if (c == '\'')
			{
				inside = !inside;
				firstError += 1;
				output << WHITE << c << RESET;
				continue;
			}

			output << (inside ? (firstError > 2 ? BLUE : RED) : WHITE) << c << RESET;
		}

		return output.str();
	}
};