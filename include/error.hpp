#pragma once

#include <iostream>
#include <string>

class Error
{
public:
	virtual std::string what() const = 0;

protected:
	std::string msg;
};

class FrontError
	: public Error
{
public:
	FrontError(const std::string& _msg);

	std::string what() const;
};

class BackError
	: public Error
{
public:
	BackError(const std::string& _file, const int _line, const std::string& _msg);

	/*
	BackError(
		const std::string& type,

		const std::string& _file,
		const int _line,

		const int _start,
		const int _end,
		
		const std::string& _desc,
		const std::string& _note,

		const std::string& _link = {}
	);
	*/

	std::string what() const;

private:
	std::string file;
	const int line;
};

#ifndef _DEBUG
	#undef  assert
	#define assert(con) std::cout << "Uh oh! Something unexpected has happened! Please submit an issue on the GitHub page:\n" \
								  << "github.com/dynamicsquid/night\n";														  \
                        exit(1)
#else
	#include <cassert>

	// makes it easier for debugging purposes as it shows where exactly the
	// error is thrown
	#define BackError(file, line, msg) BackError(__FILE__, __LINE__, std::to_string(line) + ' ' + msg)
#endif