#include "../include/error.hpp"

#include <fstream>
#include <sstream>
#include <regex>
#include <string>

const std::string CompileError::invalid_syntax = "invalid syntax";
const std::string CompileError::definition_error = "definition error";
const std::string CompileError::type_mismatch = "type mismatch";

CompileError::CompileError(const std::string& debug_file, const int debug_line, const std::string& _type,
	const std::string& _file, const int _line, const std::string& _desc, const std::string& _note,
	const std::string& _link)
	: type(_type), file(_file), line(_line), desc(_desc), note(_note), link(_link)
{
#ifdef _DEBUG
	std::cout << debug_file << '\n' << debug_line << "\n\n";
#endif
	const std::string RESET = "\033[0m";
	const std::string RED = "\033[0;31m";
	const std::string YELLOW = "\033[0;33m";
	const std::string CYAN = "\033[0;36m";
	const std::string WHITE = "\033[0;37m";
	const std::string b_RED = "\033[0;1;31m";
	const std::string b_WHITE = "\033[0;1;37m";
	const std::string u_RED = "\033[0;4;31m";
	const std::string u_WHITE = "\033[0;4;37m";
	const std::string bu_RED = "\033[0;1;4;31m";
	const std::string bu_WHITE = "\033[0;1;4;37m";

	std::ifstream code(file);
	assert(code.is_open() && "shit"); // I blame the user for this one

	for (int a = 0; a < line; ++a)
		getline(code, code_line);

	for (int a = file.length() - 1; a >= 0; --a)
	{
		if (file[a] == '\\' || file[a] == '/')
		{
			file = file.substr(a + 1);
			break;
		}
	}

	int start = -1;
	for (std::size_t a = 0; a < desc.length(); ++a)
	{
		if (desc[a] == '\'' && start == -1)
		{
			start = a + 1;
		}
		else if (desc[a] == '\'' && start != -1)
		{
			if (std::regex_match(desc.substr(start, a - start), std::regex("[0-9]+")) ||
				std::regex_match(desc.substr(start, a - start), std::regex("([0-9]+)(\\.[0-9]+)?")))
			{
				desc.insert(start, YELLOW);
				desc.insert(a + YELLOW.length(), WHITE);
				a += YELLOW.length() + WHITE.length();
			}
			else if (std::regex_match(desc.substr(start, a - start), std::regex("[a-zA-Z_][a-zA-Z_0-9]*")))
			{
				desc.insert(start, CYAN);
				desc.insert(a + CYAN.length(), WHITE);
				a += CYAN.length() + WHITE.length();
			}
			else
			{
				note.insert(start, b_WHITE);
				note.insert(a + b_WHITE.length(), WHITE);
				a += b_WHITE.length() + WHITE.length();
			}

			start = -1;
		}
	}

	start = -1;
	for (std::size_t a = 0; a < note.length(); ++a)
	{
		if (note[a] == '\'' && start == -1)
		{
			start = a + 1;
		}
		else if (note[a] == '\'' && start != -1)
		{
			if (std::regex_match(note.substr(start, a - start), std::regex("[0-9]+")) ||
				std::regex_match(note.substr(start, a - start), std::regex("([0-9]+)(\\.[0-9]+)?")))
			{
				note.insert(start, YELLOW);
				note.insert(a + YELLOW.length(), WHITE);
				a += YELLOW.length() + WHITE.length();
			}
			else if (std::regex_match(note.substr(start, a - start), std::regex("[a-zA-Z_][a-zA-Z_0-9]*")))
			{
				note.insert(start, CYAN);
				note.insert(a + CYAN.length(), WHITE);
				a += CYAN.length() + WHITE.length();
			}
			else
			{
				note.insert(start, b_WHITE);
				note.insert(a + b_WHITE.length(), WHITE);
				a += b_WHITE.length() + WHITE.length();
			}

			start = -1;
		}
	}
}

std::string CompileError::what() const
{
	const std::string RESET = "\033[0m";
	const std::string RED = "\033[0;31m";
	const std::string YELLOW = "\033[0;33m";
	const std::string CYAN = "\033[0;36m";
	const std::string WHITE = "\033[0;37m";
	const std::string b_RED = "\033[0;1;31m";
	const std::string b_WHITE = "\033[0;1;37m";
	const std::string u_RED = "\033[0;4;31m";
	const std::string u_WHITE = "\033[0;4;37m";
	const std::string bu_RED = "\033[0;1;4;31m";
	const std::string bu_WHITE = "\033[0;1;4;37m";

	std::stringstream output;

	output << b_RED << "[ compile error ] -> " << u_RED << type << '\n';
	output << WHITE << file << " (" << line << ")\n\n";

	output << WHITE << desc << "\n\n\n";

	output << "  " << b_WHITE << "| " << line << ":   " << WHITE << code_line << "\n\n\n";

	output << WHITE << note << '\n';
	if (link != "")
		output << WHITE << "\nfor more information, please visit:\n" << CYAN << "https://github.io/night/" << link << '\n';

	output << RESET;
	return output.str();
}

FrontError::FrontError(const std::string& _msg)
{
	msg = _msg;
}

std::string FrontError::what() const
{
	return msg;
}

#undef BackError

BackError::BackError(const std::string& _file, const int _line, const std::string& _msg)
	: file(_file), line(_line)
{
	msg = _msg;

	for (int a = file.length() - 1; a >= 0; --a)
	{
		if (file[a] == '\\' || file[a] == '/')
		{
			file = file.substr(a + 1);
			break;
		}
	}
}

std::string BackError::what() const
{
	return file + " (" + std::to_string(line) + "): error - " + msg + '\n';
}