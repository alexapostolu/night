/*
 * There are three errors, minor, fatal and runtime.
 *
 * Minor errors do not impede the lexing, parsing, code generation or interpretation
 * of the program. The rest of the program should carry on with the normal task
 * after a minor error is created. An example would be a type error.
 * 
 * Fatal errors do impede the future program. An example would be an unidentified
 * symbol.
 * 
 * 
 * The Error functions take in source_location::current() as a default argument,
 * and if debug flag is set to true, then the source location of the line that
 * threw the error is displayed, allowing the programmer to easily find where
 * the error was thrown in the source code.
 */

#pragma once

#include <source_location>
#include <stdexcept>
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

void create_warning(
	std::string const& msg,
	Location const& loc,
	std::source_location const& s_loc = std::source_location::current()
) noexcept;

void create_minor_error(
	std::string const& msg,
	Location const& loc,
	std::source_location const& s_loc = std::source_location::current()
) noexcept;

/*
 * Creates a fatal error that should immediately stops the execution of the
 * program. This would be used for errors that make lexing and parsing future
 * tokens impossible. Mainly used in Lexer and Parser.
 */
[[nodiscard]]
error const& create_fatal_error(
	std::string const& msg,
	Location const& loc,
	std::source_location const& s_loc = std::source_location::current()
) noexcept;

/*
 * Creates a fatal error that immediately stops the execution of the program.
 */
[[nodiscard]]
error const& create_runtime_error(
	std::string const& msg,
	std::source_location const& s_loc = std::source_location::current()
) noexcept;

std::string format_error_msg(
	std::string const& type,
	std::string const& msg,
	Location const& loc,
	std::source_location const& s_loc
) noexcept;

}