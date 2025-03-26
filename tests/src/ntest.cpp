#include "ntest.hpp"

#include <fstream>
#include <filesystem>
#include <string>
#include <iostream>

std::string const clear = "\033[0m";
std::string const red = "\033[31m";
std::string const red_bold = "\033[1;31m";
std::string const green = "\033[32m";
std::string const green_bold = "\033[1;32m";
std::string const yellow = "\033[33m";
std::string const cyan = "\033[36m";

static int test_files = 0;
int tests_failed = 0;

std::string create_test_file(std::string const& code)
{
	std::string file_name = std::to_string(test_files);
	std::string content = code;

	std::ofstream file(file_name);
	file << content;

	test_files++;
	file.close();

	return file_name;
}

void delete_file(std::string const& name) {
	try {
		std::filesystem::remove(name);
	}
	catch (std::filesystem::filesystem_error const& e) {
		std::cerr << "Filesystem error: " << e.what() << "\n";
	}
}

void ntest::clean_test_files()
{
	for (int i = 0; i < test_files; ++i)
		delete_file(std::to_string(i));
}

int ntest::display_summary()
{
	if (tests_failed)
	{
		std::cout << red_bold << '\n' << tests_failed << (tests_failed == 1 ? " unit test " : " unit tests ") << "failed.\n" << clear;
		return 1;
	}
	else
	{
		std::cout << green_bold << "\nAll unit tests passed.\n" << clear;
		return 0;
	}
}
