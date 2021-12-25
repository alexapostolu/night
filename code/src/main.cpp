#include "front-end/front-end.hpp"
#include "error.hpp"
#include "cmakedef.hpp"

#include <iostream>
#include <stdexcept>

int main(int argc, char* argv[])
{
	std::string const more_info = "for more info, run: night --help\n";

	if (argc != 2)
	{
		std::cout << "invalid number of arguments\n" << more_info;
		return;
	}

	if (argv[1][0] == '-')
	{
		if (!(std::strcmp(argv[1], "--help") && std::strcmp(argv[1], "-h")))
		{
			std::cout << "usage: night <file>|<options>\n"
					  << "options:\n"
					  << "    --help     displays this message and exit\n"
					  << "    --version  displays night's current version\n";
			return;
		}
		if (!(std::strcmp(argv[1], "--version") && std::strcmp(argv[1], "-v")))
		{
			std::cout << "night v"
					  << night_VERSION_MAJOR << '.'
					  << night_VERSION_MINOR << '.'
					  << night_VERSION_PATCH << '\n';
			return;
		}

		std::cout << "unknown option: " << argv[1] << '\n' << more_info;
		return;
	}

	try {
		front_end(argv[1]);
		return 0;
	}
	catch (night::error const& e) {
		std::cout << e.what();
	}
	catch (std::exception const& e) {
		std::cout << "Uh oh! We've come across an unexpected error:\n\n    " << e.what() <<
			"\n\nPlease submit an issue on the GitHub page:\nhttps://github.com/dynamicsquid/night\n";
	}

	return 1;
}
