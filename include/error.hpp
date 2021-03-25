#pragma once

#include <iostream>
#include <stdexcept>
#include <cassert>
#include <string>

class Error
{
public:
	Error(
		const std::string& debug_file,
		const int debug_line,

		const std::string& type,

		const std::string& _file,
		const int _line,

		const std::string& _desc,

		const std::string& _note = {},
		const std::string& _link = {}
	);

public:
	std::string what() const;

	static std::string UnexpectedError(const std::exception& e);

protected:
	std::string error_type;

	std::string type;

	std::string file;
	int line;

	std::string code_line;

	std::string desc;
	std::string note;

	std::string link;

protected:
	const std::string RESET = "\033[0m";
	const std::string RED = "\033[0;31m";
	const std::string YELLOW = "\033[0;33m";
	const std::string CYAN = "\033[0;36m";
	const std::string WHITE = "\033[0;37m";
	const std::string b_RED = "\033[0;1;31m";
	const std::string b_WHITE = "\033[0;1;37m";
	const std::string u_RED = "\033[0;4;31m";
	const std::string u_WHITE = "\033[0;4;37m";
	const std::string bu_RED = "\033[0;1;4;31m";
	const std::string bu_WHITE = "\033[0;1;4;37m";
};

class FrontEndError
{
public:
	FrontEndError(const std::string& _msg);

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

		const std::string& _note = {},
		const std::string& _link = {}
	);

public:
	static const std::string invalid_syntax;
	static const std::string invalid_grammar;
	static const std::string invalid_definition;
	static const std::string invalid_type;
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

		const std::string& _note = {},
		const std::string& _link = {}
	);

public:
	static const std::string invalid_expression;
	static const std::string out_of_range;
	static const std::string invalid_type;
};