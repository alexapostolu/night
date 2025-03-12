#include "error.hpp"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <source_location>
#include <string>

static std::string const clear = "\033[0m";
static std::string const red = "\033[31m";
static std::string const green = "\033[32m";
static std::string const yellow = "\033[33m";
static std::string const blue = "\033[34m";
static std::string const cyan = "\033[36m";

night::error::error()
	: debug_flag(false), has_minor_errors_(false) {}

night::error& night::error::get()
{
	static error instance;
	return instance;
}

void night::error::what() const
{
	for (auto const& err : errors)
	{
		std::string s = err.message;

		bool inside_quote = false;
		std::size_t pos = 0;
		while ((pos = s.find('\'', pos)) != std::string::npos)
		{
			inside_quote = !inside_quote;

			std::string const& colour = inside_quote ? yellow : clear;

			s.replace(pos, 1, colour);
			pos += colour.length();
		}

		pos = 0;
		while ((pos = s.find("integer", pos)) != std::string::npos) {
			s.replace(pos, strlen("integer"), "\033[36minteger\033[0m");
			pos += strlen("\033[36minteger\033[0m");
		}
		pos = 0;
		while ((pos = s.find("float", pos)) != std::string::npos) {
			s.replace(pos, strlen("float"), "\033[36mfloat\033[0m");
			pos += strlen("\033[36mfloat\033[0m");
		}
		pos = 0;
		while ((pos = s.find("character", pos)) != std::string::npos) {
			s.replace(pos, strlen("character"), "\033[36mcharacter\033[0m");
			pos += strlen("\033[36mcharacter\033[0m");
		}

		std::cout << red << "[ ";

		switch (err.type)
		{
		case ErrorType::Warning:
			std::cout << "warning";
			break;
		case ErrorType::Minor:
			std::cout << "minor error";
			break;
		case ErrorType::FatalCompile:
			std::cout << "fatal compile error";
			break;
		case ErrorType::FatalRuntime:
			std::cout << "fatal runtime error";
			break;
		}

		std::cout << " ]\n" << clear;

		std::cout << err.location.file << " (" << std::to_string(err.location.line) << ":" << std::to_string(err.location.col) << ")\n";

		if (error::get().debug_flag)
			std::cout << err.source_location.file_name() << " " << std::to_string(err.source_location.line()) << '\n';

		std::cout << '\n' << s << "\n\n";

		std::string error_line;
		std::ifstream error_file(err.location.file);
		for (int i = 0; i < err.location.line; ++i)
			std::getline(error_file, error_line);

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