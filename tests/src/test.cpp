#include "expression_parser_units.hpp"

#include <iostream>

static std::string const clear = "\033[0m";
static std::string const red = "\033[31m";
static std::string const red_bold = "\033[1;31m";
static std::string const green = "\033[32m";
static std::string const green_bold = "\033[1;32m";
static std::string const yellow = "\033[33m";
static std::string const cyan = "\033[36m";

static int failed = 0;

#define night_test(function)												\
	std::cout << #function << ' ';											\
																			\
	if (std::string err = function(); err.empty())							\
		std::cout << green << "Passed.\n" << clear;							\
	else																	\
		{ std::cout << red << "Failed.\n" << yellow << err << clear << '\n'; failed++; }

int main()
{
	std::cout << cyan << "Running unit tests...\n\n" << clear;

	night_test(test_expression_parser_basic);
	night_test(test_expression_parser_order_of_operations);

	if (failed)
	{
		std::cout << red_bold << '\n' << failed << (failed == 1 ? " unit test " : " unit tests ") << "failed.\n" << clear;
		return 1;
	}
	else
	{
		std::cout << green_bold << "\nAll unit tests passed.\n" << clear;
	}
}
