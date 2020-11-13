#pragma once

#include <iostream>
#include <cassert>
#include <string>

// MSVC specific macro; it shouldn't be defined in release builds
#ifdef _DEBUG
	// similar to the assert macro but also returns a value to suppress compiler warnings
	#define assert_rtn(con, rtn) assert(con); return rtn
#else
	#define assert(con) std::cerr << "Oops! Something unexpected has happened! " \
		"Please submit an issue on the GitHub page.\n"; exit(1)
	#define assert_rtn(con, rtn) std::cerr << "Oops! Something unexpected has happened! " \
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