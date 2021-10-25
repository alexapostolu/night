#pragma once

#include <stdexcept>
#include <cassert>
#include <string>

#define NIGHT_PREPROCESS_ERROR(msg) \
	night::error(__FILE__, __LINE__, night::error_preprocess, {}, msg, {})

struct Location
{
	std::string file;
	int line, col;
};

namespace night {

const char format_array[] = "arrays must be in this format: `[elem1, elem2]`";
const char format_subscript[] = "subscript operators must be in this format: `array[index]`";
const char format_init[] = "variable initializations must be in this format: `let variable = expression`";
const char format_if[] = "if conditionals must be in this format: `if (condition) {}`";
const char format_elif[] = "elif conditionals must be in this format: `elif (condition) {}`";
const char format_else[] = "else conditionals must be in this format: `else {}`";
const char format_while[] = "while loops must be in this format: `while (condition) {}`";
const char format_for[] = "for loops must be in this format: `for (iterator : range) {}`";
const char format_fn[] = "function definitions must be in this format: `fn function(parameters) {}`";
const char format_call[] = "function calls must be in this format: `function(parameters)`";
const char format_method[] = "method calls must be in this format: `object.method(parameters)`";

const char error_preprocess[] = "preprocessor";
const char error_compile[] = "compile";
const char error_runtime[] = "runtime";

class error
{
public:
	error(
		std::string_view debug_file,
		int const debug_line,

		std::string const& _type,
		Location const& _loc,

		std::string const& _msg,
		std::string const& _fix
	);

public:
	std::string what() const;

private:
	std::string type;

	Location loc;
	std::string line;

	std::string msg;
	std::string fix;

private:
	static constexpr char RESET[] = "\033[0m";
	static constexpr char RED[] = "\033[0;31m";
	static constexpr char YELLOW[] = "\033[0;33m";
	static constexpr char CYAN[] = "\033[38;5;45m";
	static constexpr char WHITE[] = "\033[0;37m";
	static constexpr char b_RED[] = "\033[0;1;31m";
	static constexpr char b_WHITE[] = "\033[0;1;37m";
	static constexpr char u_RED[] = "\033[0;4;31m";
	static constexpr char u_WHITE[] = "\033[0;4;37m";
	static constexpr char bu_RED[] = "\033[0;1;4;31m";
	static constexpr char bu_WHITE[] = "\033[0;1;4;37m";
};

struct error_wrapper
{
	const std::string type;
};

} // namespace night
