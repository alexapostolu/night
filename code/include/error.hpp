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
 * The Error functions take in source_location::current() as a default argument,
 * and if debug flag is set to true, then the source location of the line that
 * threw the error is displayed, allowing the programmer to easily find where
 * the error was thrown in the source code.
 */

#pragma once

#include "token.hpp"

#include <source_location>
#include <vector>
#include <string>

enum class ErrorType
{
	Warning,
	Minor,
	FatalCompile,
	FatalRuntime
};

struct ErrorData
{
	ErrorType type;
	Location location;
	std::source_location source_location;
	std::string message;

	Token token;
};

namespace night {

class error
{
public:
	static error& get();

	bool has_minor_errors() const;
	
	void what();

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

	void create_minor_error(
		std::string const& message,
		Token const& token,
		std::source_location const& s_loc = std::source_location::current()
	) noexcept;

	[[nodiscard]]
	error const& create_fatal_error(
		std::string const& msg,
		Location const& loc,
		std::source_location const& s_loc = std::source_location::current()
	) noexcept;

	[[nodiscard]]
	error const& create_runtime_error(
		std::string const& msg,
		std::source_location const& s_loc = std::source_location::current()
	) noexcept;

public:
	void operator=(error const&) = delete;

private:
	error();

public:
	bool debug_flag;

private:
	std::vector<ErrorData> errors;
	bool has_minor_errors_;
};

}