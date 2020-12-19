#pragma once

#include <iostream>
#include <cassert>
#include <string>

#ifndef _DEBUG
	inline void AssertError()
	{
		std::cerr << "Oops! Something unexpected has happened! Please submit an issue on the GitHub page.\n";
		exit(1);
	}

	#undef  assert
	#define assert(con) AssertError()

	// just in case I forget to remove all the debugging print statements
	#define clog wtf are you doing
#endif

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