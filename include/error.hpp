#pragma once

#include <iostream>
#include <stdexcept>
#include <string>

#ifndef _DEBUG
	#undef  assert
	#define assert(con) std::cout << "Uh oh! Something unexpected has happened! Please submit an issue on the GitHub page:\n" \
								  << "github.com/dynamicsquid/night\n";														  \
                        exit(1)
#else
	#include <cassert>
#endif

class Error
{
public:
	virtual std::string what() const = 0;

	static std::string UnexpectedError(
		const std::exception& e
	);

protected:
	std::string msg;
};

class FrontEndError
{
public:
	std::string what() const;
};

class CompileError
	: public Error
{
public:
	CompileError(
		const std::string& debug_file,
		const int debug_line,

		const std::string& type,

		const std::string& _file,
		const int _line,

		const std::string& _desc,
		const std::string& _note,

		const std::string& _link = {}
	);

	std::string what() const;

public:
	static const std::string invalid_syntax;
	static const std::string invalid_grammar;
	static const std::string definition_error;
	static const std::string type_mismatch;

private:
	std::string type;

	std::string file;
	int line;

	std::string code_line;

	std::string desc;
	std::string note;

	std::string link;
};

class RuntimeError
	: public Error
{
public:
	RuntimeError(
		const std::string& debug_file,
		const int debug_line,

		const std::string& type,

		const std::string& _file,
		const int _line,

		const std::string& _desc,
		const std::string& _note,

		const std::string& _link = {}
	);

	std::string what() const;

public:
	static const std::string invalid_expression;
	static const std::string out_of_range;
	static const std::string type_mismatch;

private:
	std::string type;

	std::string file;
	int line;

	std::string code_line;

	std::string desc;
	std::string note;

	std::string link;
};