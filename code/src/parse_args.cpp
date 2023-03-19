#include "parse_args.hpp"
#include "error.hpp"
#include "version.hpp"

#include <iostream>
#include <vector>
#include <string>

std::string parse_args(std::vector<std::string_view> const& args)
{
	std::string more_info = "for more info, type:\n"
							"    night --help\n\n";

	// make sure to use SPACES not tabs when formatting console output
	// different consoles will format tabs differently!
	// fuck 8 space tabs fuck 8 space tabs fuck 8 space tabs so much
	// 4 space tab superiority
	std::string help = "usage:\n"
					   "    night <file>\n"
					   "    night <file> <flag..>\n"
					   "    night <option>\n"
					   "flags:\n"
					   "    -b           generates a bytecode file for each source file\n"
					   "    -d           shows debug info for compiler source code (for developers)\n"
					   "options:\n"
					   "    --help       displays this message\n"
					   "    --version    displays the version\n\n";

	if (args.size() == 1)
	{
		std::cout << "you need to type some arguments!\n\n" << more_info;
		return "";
	}

	if (args.size() == 2 && args[1].find("--") == 0)
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