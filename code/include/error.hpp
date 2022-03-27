#pragma once

#include <stdexcept>
#include <cassert>
#include <string>

#define NIGHT_PREPROCESS_ERROR(msg) \
	night::error(__FILE__, __LINE__, night::error_preprocess, {}, msg, {})

struct Location
{
	std::string file;
	std::size_t line, col;
};

namespace night {

std::string const format_array = "arrays must be in this format: `[elem1, elem2]`";
std::string const format_subscript = "subscript operators must be in this format: `array[index]`";
std::string const format_init = "variable initializations must be in this format: `let variable = expression`";
std::string const format_if = "if conditionals must be in this format: `if (condition) {}`";
std::string const format_elif = "elif conditionals must be in this format: `elif (condition) {}`";
std::string const format_else = "else conditionals must be in this format: `else {}`";
std::string const format_loop = "loops must be in this format: `loop (initialization, range, condition) {}`";
std::string const format_fn = "function definitions must be in this format: `fn function(parameters) {}`";
std::string const format_call = "function calls must be in this format: `function(parameters)`";
std::string const format_method = "method calls must be in this format: `object.method(parameters)`";

std::string const error_preprocess = "preprocessor";
std::string const error_compile = "compile";
std::string const error_runtime = "runtime";

class error
{
public:
	error(
		std::string_view debug_file,
		int const        debug_line,

		std::string const& _type,
		Location	const& _loc,

		std::string const& _msg,
		std::string const& _fix
	);

public:
	std::string what() const;

public:
	static bool debug_flag;

private:
	std::string type;

	Location loc;
	std::string line;

	std::string msg;
	std::string fix;

private:
	const std::string RESET = "\033[0m";
	const std::string RED = "\033[0;31m";
	const std::string YELLOW = "\033[0;33m";
	const std::string CYAN = "\033[38;5;45m";
	const std::string WHITE = "\033[0;37m";
	const std::string b_RED = "\033[0;1;31m";
	const std::string b_WHITE = "\033[0;1;37m";
	const std::string u_RED = "\033[0;4;31m";
	const std::string u_WHITE = "\033[0;4;37m";
	const std::string bu_RED = "\033[0;1;4;31m";
	const std::string bu_WHITE = "\033[0;1;4;37m";
};

} // namespace night
