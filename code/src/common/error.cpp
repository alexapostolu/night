#include "common/error.hpp"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <source_location>
#include <string>

static std::string const clear = "\033[0m";
static std::string const red = "\033[31m";
static std::string const green = "\033[32m";
static std::string const yellow = "\033[93m";
static std::string const blue = "\033[34m";
static std::string const cyan = "\033[96m";

static void colour_code_names(std::string& message, char name_punctuation, std::string const& colour)
{
	bool inside_quote = false;
	std::size_t pos = 0;

	while ((pos = message.find(name_punctuation, pos)) != std::string::npos)
	{
		inside_quote = !inside_quote;

		std::string const& colour = inside_quote ? yellow : clear;

		message.replace(pos, 1, colour);
		pos += colour.length();
	}
}

static void colour_code_types(std::string& message, std::string const& type, std::string const& colour)
{
	std::string colour_coded_type = colour + type + clear;
	std::size_t pos = 0;

	while ((pos = message.find(type, pos)) != std::string::npos)
	{
		message.replace(pos, type.length(), colour_coded_type);
		pos += colour_coded_type.length();
	}
}

static std::string error_type_to_str(ErrorType type)
{
	switch (type) {
	case ErrorType::Warning:	  return "Warning";
	case ErrorType::Minor:		  return "Minor Error";
	case ErrorType::FatalCompile: return "Fatal Compile Error";
	case ErrorType::FatalRuntime: return "Fatal Runtime Error";
	default: throw std::runtime_error("Unhandled Case");
	}
}

static std::string get_line_of_error(std::string const& file_name, int line_number)
{
	std::string line;

	std::ifstream file(file_name);
	for (int i = 0; i < line_number; ++i)
		std::getline(file, line);

	line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));

	return line;
}

night::error::error()
	: debug_flag(false)
	, has_minor_errors_(false) {}

night::error& night::error::get()
{
	static error instance;
	return instance;
}

void night::error::what(bool only_warnings)
{
	for (auto& err : errors)
	{
		if (only_warnings && err.type != ErrorType::Warning)
			continue;

		/* Colour Code Error Message */

		colour_code_names(err.message, '\'', yellow);

		colour_code_types(err.message, "boolean", cyan);
		colour_code_types(err.message, "character", cyan);
		colour_code_types(err.message, "integer", cyan);
		colour_code_types(err.message, "float", cyan);
		colour_code_types(err.message, "string", cyan);

		std::string error_line = get_line_of_error(err.location.file, err.location.line);

		/* Display Error Message */

		// [ Minor Error ]
		std::cout << red << "[ " << error_type_to_str(err.type) << " ]\n" << clear;

		// source.night (10:52)
		std::cout << err.location.file << " (" << err.location.line << ":" << err.location.col << ")\n";

		// night/code/src/ast/statement.cpp 53
		if (error::get().debug_flag)
			std::cout << err.source_location.file_name() << " " << err.source_location.line() << '\n';

		std::cout << '\n' << err.message << "\n\n";

		std::cout << "    " << error_line << "\n";

		int indent_size = 4;
		// -1 because the lexer ends one position after the token when it eats.
		int token_position = err.location.col - 1;

		if (err.token.str.empty())
		{
			for (int i = 0; i < indent_size + token_position; ++i)
				std::cout << ' ';
			std::cout << green << "^\n\n" << clear;
		}
		else
		{
			for (int i = 0; i < indent_size + token_position - err.token.str.empty(); ++i)
				std::cout << ' ';
			std::cout << blue;
			for (int i = 0; i < err.token.str.length(); ++i)
				std::cout << "~";
			std::cout << "\n\n" << clear;
		}
	}
}

void night::error::create_warning(
	std::string const& msg,
	Location const& loc,
	std::source_location const& s_loc) noexcept
{
	error::get().errors.emplace_back(ErrorType::Warning, loc, s_loc, msg);
}

void night::error::create_minor_error(
	std::string const& msg,
	Location const& loc,
	std::source_location const& s_loc) noexcept
{
	error::get().errors.emplace_back(ErrorType::Minor, loc, s_loc, msg, Token{ TokenType{}, "", Location{} });
	has_minor_errors_ = true;
}

void night::error::create_minor_error(
	std::string const& message,
	Token const& token,
	std::source_location const& s_loc) noexcept
{
	error::get().errors.emplace_back(ErrorType::Minor, token.loc, s_loc, message, token);
	has_minor_errors_ = true;
}

night::error const& night::error::create_fatal_error(
	std::string const& msg,
	Location const& loc,
	std::source_location const& s_loc) noexcept
{
	error::get().errors.emplace_back(ErrorType::FatalCompile, loc, s_loc, msg);
	return error::get();
}

night::error const& night::error::create_runtime_error(
	std::string const& msg,
	std::source_location const& s_loc) noexcept
{
	error::get().errors.emplace_back(ErrorType::FatalRuntime, Location{}, s_loc, msg);
	return error::get();
}

bool night::error::has_minor_errors() const
{
	return has_minor_errors_;
}