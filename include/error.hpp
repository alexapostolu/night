#pragma once

#include "../include/back-end/utils.hpp"

#include <iostream>
#include <stdexcept>
#include <cassert>
#include <string>
#include <vector>

#define NIGHT_PREPROCESS_ERROR(msg, link) \
	night::error(Location{ __FILE__, __LINE__ }, night::error_preprocess, {}, msg, {}, link)

#define NIGHT_COMPILE_ERROR(msg, fix, link, loc) \
	night::error(Location{ __FILE__, __LINE__ }, night::error_compile, loc, msg, fix, link)

#define NIGHT_RUNTIME_ERROR(msg, fix, link) \
	night::error(Location{ __FILE__, __LINE__ }, night::error_runtime, stmt.loc, msg, fix, link)

namespace night {

const std::string error_preprocess = "preprocessor";
const std::string error_compile = "compile";
const std::string error_runtime = "runtime";

const std::string learn_learn = "learn.html"; // you really screwed up the syntax if you get linked to this

const std::string learn_run = "";
const std::string learn_include = "learn.html#including-files";

const std::string learn_variables = "learn.html#variables";
const std::string learn_arrays = "learn.html#arrays";
const std::string learn_operators = "learn.html#operators";
const std::string learn_conditionals = "learn.html#conditional";
const std::string learn_loops = "learn.html#loops";
const std::string learn_functions = "learn.html#functions";
const std::string learn_classes = "learn.html#classes";
const std::string learn_type_checking = "learn.html#type-checking";

class error
{
public:
	error(
		const Location& debug_loc,

		const std::string& _type,
		const Location& _loc,

		const std::string& _msg,

		const std::string& _fix,
		const std::string& _link
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

struct error_wrapper
{
	const std::string type;
};

} // namespace night