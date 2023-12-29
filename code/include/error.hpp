#pragma once

#include <source_location>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <string>

struct Location
{
	std::string file;
	int line, col;
};

namespace night {

class error
{
public:
	static error& get();

	bool has_minor_errors() const;
	std::string what() const;

public:
	void operator=(error const&) = delete;

private:
	error();

public:
	bool debug_flag;

	std::vector<std::string> minor_errors;
	std::string fatal_error_msg;
};



void create_warning(std::string const& msg, Location const& loc,
	std::source_location const& s_loc = std::source_location::current()) noexcept;

void create_minor_error(std::string const& msg, Location const& loc,
	std::source_location const& s_loc = std::source_location::current()) noexcept;

[[nodiscard]]
error const& create_fatal_error(
	std::string const& msg, Location const& loc,
	std::source_location const& s_loc = std::source_location::current()) noexcept;

[[nodiscard]]
error const& create_runtime_error(
	std::string const& msg,
	std::source_location const& s_loc = std::source_location::current()) noexcept;

std::string format_error_msg(
	std::string const& type,
	std::string const& msg,
	Location const& loc,
	std::source_location const& s_loc
) noexcept;

}