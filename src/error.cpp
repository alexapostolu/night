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
	const ErrorType& _type,
	const Location& _loc,
	const std::string& _msg,
	const std::string& _fix,
	const Learn& _link
)
	: loc(_loc), msg(_msg), fix(_fix)
{
#ifdef _DEBUG // Visual Studio specific macro
	std::cout << debug_loc.file << '\n' << debug_loc.line << "\n\n";
#endif

	switch (_type)
	{
	case ErrorType::PREPROCESSOR:
		type = "preprocessor";
		break;
	case ErrorType::COMPILE:
		type = "compile";
		break;
	case ErrorType::RUNTIME:
		type = "runtime";
		break;
	}

	switch (_link)
	{
	case Learn::VARIABLES:
		link = "learn.html#variables";
		break;
	case Learn::ARRAYS:
		link = "learn.html#arrays";
		break;
	case Learn::CONDITIONALS:
		link = "learn.html#conditionals";
		break;
	case Learn::LOOPS:
		link = "learn.html#loops";
		break;
	case Learn::FUNCTIONS:
		link = "learn.html#functions";
		break;
	case Learn::TYPE_CHECKING:
		link = "learn.html#type-checking";
		break;
	}

	std::ifstream code(loc.file);
	assert(code.is_open(), "shit"); // i blame the user for this one

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
}

std::string night::error::what() const
{
	std::stringstream output;

	output << b_RED << "[ error ] -> " << u_RED << type << '\n';
	output << WHITE << loc.file << " (" << loc.line << ")\n\n";

	output << WHITE << msg << "\n\n\n";

	output << "  " << b_WHITE << "| " << loc.line << ":   " << WHITE << line << "\n\n\n";

	output << WHITE << fix << '\n';
	if (link != "")
		output << WHITE << "\nfor more information, please visit:\n" << CYAN << "https://dynamicsquid.github.io/night/" << link << '\n';

	output << RESET;
	return output.str();
}