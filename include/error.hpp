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

	// for better error messages
	/*
	BackError(
		const std::string& _file,
		const int _line,
		
		const std::string& _desc,
		const std::string& _note
	);
	*/

	std::string what() const;

private:
	std::string file;
	int line;

	// for better error messages
	// const std::string note;
};

#ifndef _DEBUG
	#undef  assert
	#define assert(con) std::cout << "Uh oh! Something unexpected has happened! Please submit an issue on the GitHub page:\n" \
								  << "github.com/dynamicsquid/night\n";														  \
                        exit(1)
#else
	// makes it easier for debugging purposes as it shows where exactly the
	// error is thrown
	#include <cassert>
	#define BackError(file, line, msg) BackError(__FILE__, __LINE__, std::to_string(line) + ' ' + msg)
#endif