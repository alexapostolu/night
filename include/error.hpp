#pragma once

#include "../include/back-end/utils.hpp"

#include <iostream>
#include <stdexcept>
#include <cassert>
#include <string>
#include <vector>

#define NIGHT_COMPILE_ERROR(msg, fix, link) night::error(Location{ __FILE__, __LINE__ }, ErrorType::COMPILE, loc, msg, fix, link)
#define NIGHT_RUNTIME_ERROR(msg, fix, link) night::error(Location{ __FILE__, __LINE__ }, ErrorType::RUNTIME, loc, msg, fix, link)

enum class ErrorType {
	PREPROCESSOR,
	COMPILE,
	RUNTIME
};

enum class Learn {
	LEARN, // you really screwed up the syntax if you get linked to this
	VARIABLES,
	ARRAYS,
	CONDITIONALS,
	LOOPS,
	FUNCTIONS,
	TYPE_CHECKING
};

namespace night {

class error
{
public:
	error(
		const Location& debug_loc,

		const ErrorType& _type,
		const Location& _loc,

		const std::string& _msg,

		const std::string& _fix,
		const Learn& _link
	);

public:
	std::string what() const;

private:
	std::string type;

	Location loc;
	std::string line;

	std::string msg;

	std::string fix;
	std::string link;

private:
	const std::string RESET = "\033[0m";
	const std::string RED = "\033[0;31m";
	const std::string YELLOW = "\033[0;33m";
	const std::string CYAN = "\033[0;36m";
	const std::string WHITE = "\033[0;37m";
	const std::string b_RED = "\033[0;1;31m";
	const std::string b_WHITE = "\033[0;1;37m";
	const std::string u_RED = "\033[0;4;31m";
	const std::string u_WHITE = "\033[0;4;37m";
	const std::string bu_RED = "\033[0;1;4;31m";
	const std::string bu_WHITE = "\033[0;1;4;37m";
};

} // namespace night