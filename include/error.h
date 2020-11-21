#pragma once

#include <iostream>
#include <cassert>
#include <string>

class Error
{
public:
	Error(const std::string& _message);
	Error(const std::string& _file, int _line, const std::string& _message);

public:
	std::string what() const;

public:
	std::string file;
	int line;
	std::string message;
};

// MSVC specific macro; it shouldn't be defined in release builds
#ifdef _DEBUG
	// similar to the assert macro, but also returns a value to satisfy compiler warnings
	#define assert_rtn(con, rtn) assert(con); return rtn
#else
	inline void AssertError()
	{
		std::cerr << "Oops! Something unexpected has happened! Please submit an issue on the GitHub page.\n";
		exit(1);
	}

	#define assert(con) AssertError()
	#define assert_rtn(con, rtn) AssertError()

	// just in case I forget to remove all the debugging print statements :)
	#define clog wtf are you doing
#endif