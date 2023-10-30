#include "parse_args.hpp"

#include "error.hpp"
#include "version.hpp"

#include <iostream>
#include <vector>
#include <string>

std::string parse_args(int argc, char* argv[])
{
	std::string const more_info = "for more info, type:\n"
								  "    night --help\n\n";

	std::string const help = "usage:\n"
							 "    night <file>\n"
							 "    night <file> <flag..>\n"
							 "    night <option>\n"
							 "flags:\n"
							 "    -d           shows debug info for compiler source code (for developers)\n"
							 "options:\n"
							 "    --help       displays this message\n"
							 "    --version    displays the version\n\n";

	std::vector<std::string> args(argv, argv + argc);

	if (argc == 1)
	{
		std::cout << "you need to type some arguments!\n\n" << more_info;
		return "";
	}

	if (argc == 2 && args[1].find("--") == 0)
	{
		if (args[1] == "--help")
		{
			std::cout << help;
		}
		else if (args[1] == "--version")
		{
			std::cout << "night v"
					  << NIGHT_vMAJOR << '.'
					  << NIGHT_vMINOR << '.'
					  << NIGHT_vPATCH << "\n\n";
		}
		else
		{
			std::cout << "unknown option: " << args[1] << '\n' << more_info;
		}

		return "";
	}

	std::string run_file;

	for (std::size_t i = 1; i < args.size(); ++i)
	{
		if (args[i].length() >= 6 && args[i].substr(args[i].length() - 6) == ".night")
		{
			if (!run_file.empty())
			{
				std::cout << "you can not run more than one file at the same time!\n";
				return "";
			}

			run_file = args[i];
		}
		else if (args[i] == "-d")
		{
			night::error::get().debug_flag = true;
		}
		else
		{
			std::cout << "unknown option: " << args[i] << '\n' << more_info;
			return "";
		}
	}

	return run_file;
}