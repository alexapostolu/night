#pragma once

#include <iostream>
#include <string>

#ifdef _DEBUG
	#define assert(con, msg) if (con) { std::cerr << __FILE__ << '\n' << __LINE__ << '\n' << msg << '\n'; exit(1); }
	
#else
	#define assert(msg) std::cerr << "Oops! Something unexpected has happened! " \
		"Please submit an issue on the GitHub page.\n"; exit(1)
#endif

class Error
{
public:
	Error(const std::string& _message)
		: file("source"), message(_message) {}

	Error(const std::string& _file, int _line, const std::string& _message)
		: file(_file), line(_line), message(_message) {}

public:
	std::string what() const
	{
		return file == "source"
			? "error - " + message
			: file + " (" + std::to_string(line) + "): error - " + message + "\n";
	}

public:
	std::string file;
	int line;
	std::string message;
};