#include "error.hpp"

#include <source_location>
#include <string>

night::error::error()
	: debug_flag(false) {}

night::error& night::error::get()
{
	static error instance;
	return instance;
}

std::string night::error::what() const
{
	std::string s = fatal_error_msg + "\n";

	for (auto const& msg : minor_errors)
		s += msg + '\n';

	for (auto const& msg : warnings)
		s += msg + '\n';

	return s;
}

void night::error::create_warning(std::string const& msg, Location const& loc, std::source_location const& s_loc) noexcept
{
	warnings.push_back(format_error_msg(msg, loc, s_loc));
}

void night::error::create_minor_error(std::string const& msg, Location const& loc, std::source_location const& s_loc) noexcept
{
	minor_errors.push_back(format_error_msg(msg, loc, s_loc));
}

night::error const& night::error::create_fatal_error(
	std::string const& msg, Location const& loc,
	std::source_location const& s_loc) noexcept
{
	fatal_error_msg = format_error_msg(msg, loc, s_loc);
	return *this;
}

std::string night::error::format_error_msg(
	std::string const& msg, Location const& loc,
	std::source_location const& s_loc) const noexcept
{
	std::string base = "[ error fatal ]\n" +
						loc.file + " (" + std::to_string(loc.line) + ":" + std::to_string(loc.col) + ")\n";

	if (debug_flag)
		base += std::string() + s_loc.file_name() + " " + std::to_string(s_loc.line()) + '\n';
	
	base += '\n' + msg;
	return base;
}