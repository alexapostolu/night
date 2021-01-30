#include "../include/error.hpp"

#include <string>

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

/*
BackError::BackError(const std::string& _type, const std::string& _file, const int _line, const int start, const int end,
	const std::string& _desc, const std::string& _note, const std::string& _link)
	: _type(_type), file(_file), line(_line), start(_start), end(_end), desc(_desc), note(_note), link(_link)
{
	std::ifstream code(file);
	assert(code.is_open() && "shit"); // I blame the user for this one

	std::cout << "error - " << type << '\n';
	std::cout << file << " | " << line << " | " << start << "\n\n";

	std::cout << desc << "\n\n";

	std::cout << line << ": " << code_line << "\n\n";
	for (int a = start; a <= end; ++a)
		std::cout << '~';

	std::cout << note << '\n'
	if (link != "")
		std::cout << "for more information, visit: github.io/night/" << link << '\n';
}
*/

std::string BackError::what() const
{
	return file + " (" + std::to_string(line) + "): error - " + msg + '\n';
}