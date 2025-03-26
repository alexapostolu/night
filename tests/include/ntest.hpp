#pragma once

#include <sstream>
#include <string>

#define night_test(function)												\
	std::cout << #function << ' ';											\
																			\
	if (std::string err = function(); err.empty())							\
		std::cout << green << "Passed.\n" << clear;							\
	else																	\
		{ std::cout << red << "Failed.\n"									\
					<< yellow << err << clear << "\n\n"; tests_failed++; }

#define night_assert_notnull(expr)											\
	if (!(expr))															\
		return "(" + std::to_string(__LINE__) + ") " + #expr + " == NULL";

#define night_assert_eq(res, exp) {										\
        if ((res) != (exp)) {											\
			std::stringstream ss;										\
			ss << "(" << __LINE__ << ") " << (res) << " != " << (exp);	\
			return ss.str();											\
        }																\
    }

#define night_assert_tr(expr) night_assert_eq(expr, true)

extern std::string const clear;
extern std::string const red;
extern std::string const red_bold;
extern std::string const green;
extern std::string const green_bold;
extern std::string const yellow;
extern std::string const cyan;

extern int tests_failed;

std::string create_test_file(std::string const& code);

void delete_file(std::string const& name);

namespace ntest {

void clean_test_files();

int display_summary();

}
