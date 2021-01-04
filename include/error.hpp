#pragma once

#include <iostream>
#include <cassert>
#include <string>

#ifndef _DEBUG
	inline void AssertError()
	{
		std::clog << "Oops! Something unexpected has happened! Please submit an issue on the GitHub page.\n";
		exit(1);
	}

	#undef  assert
	#define assert(con) AssertError()
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