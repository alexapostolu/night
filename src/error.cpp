#include "../include/error.h"

#include <string>

Error::Error(const std::string& _message)
	: file("source"), message(_message) {}

Error::Error(const std::string& _file, int _line, const std::string& _message)
	: file(_file), line(_line), message(_message) {}


std::string Error::what() const
{
	return file == "source"
		? "error - " + message
		: file + " (" + std::to_string(line) + "): error - " + message + "\n";
}