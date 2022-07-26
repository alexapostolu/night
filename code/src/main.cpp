#include "parse_file.hpp"
#include "error.hpp"
#include "cmakedef.hpp"

#include <iostream>
#include <vector>
#include <string>

int main(int argc, char* argv[])
{
	std::vector<std::string_view> const argv_s(argv, argv + argc);
	std::string const more_info = "for more info, run: night --help\n";
	
	if (argv_s.size() == 2 && argv_s[1].find("--") == 0)
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

		return 0;
	}

	std::string run_file;

	for (std::size_t i = 1; i < argv_s.size(); ++i)
	{
		if (argv_s[i].find(".night"))
		{
			if (!run_file.empty())
			{
				std::cout << "can not run more than one file at the same time\n";
				return 0;
			}

			run_file = argv_s[i];
		}
		else if (argv_s[i] == "-")
		{
			auto arg = argv_s[i].substr(1);

			if (arg == "debug")
			{
				night::error::debug_flag = true;
			}
			else
			{
				std::cout << "unknown option: " << argv_s[i] << '\n' << more_info;
			}
		}
		else
		{
			std::cout << "unknown option: " << argv_s[i] << '\n' << more_info;
		}
	}
	
	try {
		parse_file(run_file);
	}
	catch (night::error const& e) {
		e.display_errors();
	}
	catch (std::exception const& e) {
		std::cout << "oh no! we've come across an unexpected error:\n\n    " << e.what() <<
			"\n\nplease submit an issue on the github page:\nhttps://github.com/dynamicsquid/night\n";
	}
}