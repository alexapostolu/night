#include "../include/error.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

night::error::error(
	std::string_view debug_file, int const debug_line, std::string const& _type,
	Location const& _loc, std::string const& _msg, std::string const& _fix)
	: loc(_loc), type(_type), msg(_msg), fix(_fix)
{
#ifdef _DEBUG // Visual Studio specific macro
	std::cout << debug_file << '\n' << debug_line << "\n\n";
#endif

	if (type == night::error_preprocess)
		return;

	std::ifstream code(loc.file);
	assert(code.is_open());
	for (int a = 0; a < loc.line; ++a)
		getline(code, line);

	// remove absolute file path so only local path is displayed
	for (int a = loc.file.length() - 1; a >= 0; --a)
	{
		if (loc.file[a] == '\\' || loc.file[a] == '/')
		{
			loc.file = loc.file.substr(a + 1);
			break;
		}
	}

	bool been = false;
	std::size_t pos = msg.find("`");
	while (pos != std::string::npos)
	{
		msg.replace(pos, 1, been ? WHITE : CYAN);
		been = !been;

		pos = msg.find("`", pos + 1);
	}

	been = false;
	pos = fix.find("`");
	while (pos != std::string::npos)
	{
		fix.replace(pos, 1, been ? WHITE : CYAN);
		been = !been;

		pos = fix.find("`", pos + 1);
	}
}

std::string night::error::what() const
{
	std::stringstream output;

	output << b_RED << "[ " << type << b_RED << " error ]\n";

	if (type == night::error_preprocess)
	{
		output << '\n' << WHITE << msg << "\n";
	}
	else
	{
		output << WHITE << loc.file << " (" << loc.line << ")\n\n";
		output << WHITE << msg << "\n\n";
		output << "  " << b_WHITE << "| " << loc.line << ":   " << line << "\n\n";
		output << WHITE << fix << '\n';
	}

	output << RESET;

	return output.str();
}