#include "front-end/front-end.hpp"
#include "error.hpp"
#include "cmakedef.hpp"

#include <iostream>
#include <vector>
#include <string>

int main(int argc, char* argv[])
{
	std::vector<std::string_view> const argv_s(argv, argv + argc);
	std::string const more_info = "for more info, run: night --help\n";

	if (argv_s.size() == 2)
	{
		if (argv_s[1][0] == '-')
		{
			if (argv_s[1] == "--help")
			{
				std::cout << "usage: night <file>|<options>\n"
					<< "options:\n"
					<< "    --help     displays this message\n"
					<< "    --version  displays the version\n";
			}
			else if (argv_s[1] == "--version")
			{
				std::cout << "night v"
					<< night_VERSION_MAJOR << '.'
					<< night_VERSION_MINOR << '.'
					<< night_VERSION_PATCH << '\n';
			}
			else
			{
				std::cout << "unknown option: " << argv[1] << '\n' << more_info;
			}
		}
		else
		{
			try {
				front_end(argv[1]);
			}
			catch (night::error const& e) {
				std::cout << e.msg();
			}
			catch (std::exception const& e) {
				std::cout << "oh no! we've come across an unexpected error:\n\n    " << e.what() <<
					"\n\nplease submit an issue on the github page:\nhttps://github.com/dynamicsquid/night\n";
			}
		}
	}

	if (argc == 2 || (argc == 3 && argv[2][0] == '-'))
	{
		if (argv_s[2] == "-debug")
			night::error::debug_flag = true;

	}
	else
	{
		std::cout << "invalid number of arguments\n" << more_info;
	}
}
