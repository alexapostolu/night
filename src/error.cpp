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
BackError::BackError(const std::string& _file, const int _line, const std::string& _desc, const std::string& _note)
	: file(_file), line(_line), desc(_desc), note(_note)
{
	std::ifstream code(file);
	assert(file.is_open() && "shit");

	std::string line;
	for (int a = 0; a < line; ++a)
		getline(code, line);

	std::cout
		<< file
		<< line
		<< desc
		<< code
		<< note
		<< '\n';
}
*/

std::string BackError::what() const
{
	return file + " (" + std::to_string(line) + "): error - " + msg + '\n';
}