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
	std::string s;

	for (auto const& msg : minor_errors)
		s += msg + '\n';

	s += '\n' + fatal_error_msg + '\n';

	return s;
}

void night::error::create_warning(std::string const& msg, Location const& loc, std::source_location const& s_loc) noexcept
{
	minor_errors.push_back(format_error_msg("warning", msg, loc, s_loc));
}

void night::error::create_minor_error(std::string const& msg, Location const& loc, std::source_location const& s_loc) noexcept
{
	minor_errors.push_back(format_error_msg("minor error", msg, loc, s_loc));
}

night::error const& night::error::create_fatal_error(
	std::string const& msg, Location const& loc,
	std::source_location const& s_loc) noexcept
{
	fatal_error_msg = format_error_msg("fatal error", msg, loc, s_loc);
	return *this;
}

bool night::error::has_minor_errors() const
{
	return !minor_errors.empty();
}

std::string night::error::format_error_msg(
	std::string const& type,
	std::string const& msg,
	Location const& loc,
	std::source_location const& s_loc) const noexcept
{
	std::string base = "[ " + type + " ]\n" +
						loc.file + " (" + std::to_string(loc.line) + ":" + std::to_string(loc.col) + ")\n";

	if (debug_flag)
		base += std::string() + s_loc.file_name() + " " + std::to_string(s_loc.line()) + '\n';
	
	base += '\n' + msg + '\n';
	return base;
}