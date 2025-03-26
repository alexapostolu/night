#include "ntest.hpp"
#include "expression_parser_units.hpp"

#include <iostream>

int main()
{
	std::cout << cyan << "Running unit tests...\n\n" << clear;

	night_test(test_expression_parser_basic);
	night_test(test_expression_parser_ending_token);
	night_test(test_expression_parser_order_of_operations);

	ntest::clean_test_files();

	int ret = ntest::display_summary();
	return ret;
}
