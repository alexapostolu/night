#pragma once

#include <string>
#include <vector>
#include <source_location>
#include <stdexcept>
#include <sstream>

#define NIGHT_CREATE_WARNING(msg)			  night::error::get().create_warning(msg, lexer.loc);
#define NIGHT_CREATE_MINOR(msg)				  night::error::get().create_minor_error(msg, lexer.loc);
#define NIGHT_CREATE_FATAL(msg)				  night::error::get().create_fatal_error(msg, lexer.loc);
#define NIGHT_CREATE_FATAL_LEXER(msg)		  night::error::get().create_fatal_error(msg, loc);

struct Location
{
	std::string file;
	int line, col;
};

namespace night {

template <typename T>
inline std::runtime_error const& unhandled_case(T val, std::source_location const& s_loc = std::source_location::current())
{
	std::stringstream s;
	s << s_loc.file_name() << '\n' + s_loc.function_name() << '\n' << val;

	return std::runtime_error(s.str());
}


class error
{
public:
	static error& get();

	std::string what() const;

	void create_warning(std::string const& msg, Location const& loc,
		std::source_location const& s_loc = std::source_location::current()) noexcept;

	void create_minor_error(std::string const& msg, Location const& loc,
		std::source_location const& s_loc = std::source_location::current()) noexcept;

	error const& create_fatal_error(std::string const& msg, Location const& loc,
		std::source_location const& s_loc = std::source_location::current()) noexcept;

public:
	void operator=(error const&) = delete;

private:
	error();
	std::string format_error_msg(std::string const& msg, Location const& loc, std::source_location const& s_loc) const noexcept;

public:
	bool debug_flag;

private:
	std::vector<std::string> warnings,
							 minor_errors;

	std::string fatal_error_msg;
};

}