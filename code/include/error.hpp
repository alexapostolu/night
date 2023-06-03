#pragma once

#include <string>
#include <vector>
#include <source_location>
#include <stdexcept>

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

inline std::runtime_error const& unhandled_case(int num, std::source_location const& s_loc = std::source_location::current())
{
	return std::runtime_error(std::string() + s_loc.file_name() + '\n' + s_loc.function_name() + '\n' + std::to_string(num));
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