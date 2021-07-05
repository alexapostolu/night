#include "../include/error.hpp"
#include "../include/back-end/utils.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <regex>
#include <string>

night::error::error(
	const Location& debug_loc,
	const std::string& _type,
	const Location& _loc,
	const std::string& _msg,
	const std::string& _fix,
	const std::string& _link
)
	: loc(_loc), msg(_msg), fix(_fix)
{
#ifdef _DEBUG // Visual Studio specific macro
	std::cout << debug_loc.file << '\n' << debug_loc.line << "\n\n";
#endif

	if (type == night::error_preprocess)
		return;

	std::ifstream code(loc.file);
	assert(code.is_open(), "what kinda bullshit runtime file shenanigans are you doing?");

	for (int a = 0; a < loc.line; ++a)
		getline(code, line);

	for (int a = loc.file.length() - 1; a >= 0; --a)
	{
		if (loc.file[a] == '\\' || loc.file[a] == '/')
		{
			loc.file = loc.file.substr(a + 1);
			break;
		}
	}

	bool been = false;
	for (int a = 0; a < msg.length(); ++a)
	{
		if (msg[a] == '`')
		{
			msg.erase(a);
			msg.insert(a, been ? WHITE : CYAN);
			been = !been;
		}
	}
}

std::string night::error::what() const
{
	std::stringstream output;

	output << b_RED << "[ error ] -> " << u_RED << type << '\n';

	if (type == night::error_preprocess)
	{
		output << '\n' << WHITE << msg << "\n";
	}
	else
	{
		output << WHITE << loc.file << " (" << loc.line << ")\n\n";

		output << WHITE << msg << "\n\n\n";

		output << "  " << b_WHITE << "| " << loc.line << ":   " << WHITE << line << "\n\n\n";

		output << WHITE << fix << '\n';
		if (link != "")
			output << WHITE << "\nfor more information, please visit:\n" << CYAN << "https://dynamicsquid.github.io/night/" << link << '\n';
	}

	output << RESET;
	return output.str();
}