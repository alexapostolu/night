#include "error.hpp"

#include <fstream>
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

void night::create_warning(std::string const& msg, Location const& loc, std::source_location const& s_loc) noexcept
{
	error::get().minor_errors.push_back(format_error_msg("warning", msg, loc, s_loc));
}

void night::create_minor_error(std::string const& msg, Location const& loc, std::source_location const& s_loc) noexcept
{
	error::get().minor_errors.push_back(format_error_msg("minor error", msg, loc, s_loc));
}

night::error const& night::create_fatal_error(
	std::string const& msg, Location const& loc,
	std::source_location const& s_loc) noexcept
{
	error::get().fatal_error_msg = format_error_msg("fatal error", msg, loc, s_loc);
	return error::get();
}

night::error const& night::create_runtime_error(
	std::string const& msg,
	std::source_location const& s_loc) noexcept
{
	error::get().fatal_error_msg = "[ runtime error ]\n";

	if (error::get().debug_flag)
		error::get().fatal_error_msg += std::string() + s_loc.file_name() + " " + std::to_string(s_loc.line()) + '\n';

	error::get().fatal_error_msg += '\n' + msg + '\n';
	return error::get();
}

bool night::error::has_minor_errors() const
{
	return !minor_errors.empty();
}

std::string night::format_error_msg(
	std::string const& type,
	std::string const& msg,
	Location const& loc,
	std::source_location const& s_loc) noexcept
{
	std::string base = "[ " + type + " ]\n" +
						loc.file + " (" + std::to_string(loc.line) + ":" + std::to_string(loc.col) + ")\n";

	if (error::get().debug_flag)
		base += std::string() + s_loc.file_name() + " " + std::to_string(s_loc.line()) + '\n';

	base += '\n' + msg + "\n\n";

	std::string error_line;
	std::ifstream error_file(loc.file);
	for (int i = 0; i < loc.line; ++i)
		std::getline(error_file, error_line);

	base += "    " + error_line + "\n";

	for (int i = 0; i < loc.col; ++i)
		base += " ";
	base += "    ^\n\n";
	
	return base;
}