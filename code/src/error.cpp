#include "error.hpp"

std::string const& night::fatal_error::what() const noexcept
{
	return msg;
}

night::error::error()
	: debug_flag(false) {}

night::error& night::error::get()
{
	static error instance;
	return instance;
}

void night::error::create_warning(std::string const& msg, std::string const& file, int line) noexcept
{
	warnings.push_back(msg);
}

void night::error::create_minor_error(std::string const& msg, int line, int col, std::string const& debug_file, int debug_line) noexcept
{
	minor_error.push_back(msg);
}

night::fatal_error night::error::create_fatal_error(std::string const& msg, Lexer const& lexer, std::string const& file, int line) noexcept
{
	if (debug_flag)
		return fatal_error{ "[ error ]\n" + file + '\n' + std::to_string(line) + '\n' + msg + '\n' };
	return fatal_error{ "[ error ]\n" + lexer.file_name + " (" + std::to_string(lexer.line) + ":" + std::to_string(lexer.i) + ")\n" + msg + '\n'};
}