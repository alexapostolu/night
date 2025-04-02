#include "ntest.hpp"
#include "expression_parser_units.hpp"
#include "code_generation_tests.hpp"

#include <iostream>

int main()
{
	std::cout << cyan << "Running unit tests...\n\n" << clear;

	night_test(test_expression_parser_basic);
	night_test(test_expression_parser_ending_token);
	night_test(test_expression_parser_order_of_operations);

	night_test(test_code_gen_expression_basic);
	night_test(test_code_gen_variable_init);

	ntest::clean_test_files();

	int ret = ntest::display_summary();
	return ret;
}
