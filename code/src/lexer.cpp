#include "lexer.hpp"

#include <fstream>
#include <iostream>

Lexer::Lexer(std::string_view file_name)
{
	std::fstream file(file_name.data());
	if (!file.is_open())
		std::cout << "file not open\n";
}