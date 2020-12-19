#include "../include/error.hpp"

#include <string>

Error::Error(const std::string& _message)
	: file("source"), message(_message) {}

Error::Error(const std::string& _file, int _line, const std::string& _message)
	: file(_file), line(_line), message(_message)
{
	for (int a = file.length() - 1; a >= 0; --a)
	{
		if (file[a] == '\\' || file[a] == '/')
		{
			file = file.substr(a + 1);
			break;
		}
	}
}

std::string Error::what() const
{
	return file == "source"
		? "error - " + message
		: file + " (" + std::to_string(line) + "): error - " + message + "\n";
}