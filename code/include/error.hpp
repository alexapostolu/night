#pragma once

#include "lexer.hpp"
#include <string>
#include <vector>
#include <source_location>

#define NIGHT_CREATE_WARNING(msg)			  night::error::get().create_warning(msg, __FILE__, __LINE__);
#define NIGHT_CREATE_MINOR(msg, line, col)	  night::error::get().create_minor_error(msg, line, col, __FILE__, __LINE__);
#define NIGHT_CREATE_FATAL(msg)				  night::error::get().create_fatal_error(msg, lexer);//, __FILE__, __LINE__);
#define NIGHT_CREATE_FATAL_LEXER(msg)		  night::error::get().create_fatal_error(msg, *this, __FILE__, __LINE__);

namespace night {

struct fatal_error
{
	std::string const& what() const noexcept;
	std::string msg;
};


class error
{
public:
	error(error const&) = delete;

public:
	static error& get();

	void create_warning(std::string const& msg, std::string const& file, int line) noexcept;
	void create_minor_error(std::string const& msg, int line, int col, std::string const& debug_file, int debug_line) noexcept;
	//fatal_error create_fatal_error(std::string const& msg, Lexer const& lexer, std::string const& file, int line) noexcept;
	fatal_error create_fatal_error(std::string const& msg, Lexer const& lexer, const std::source_location& loc = std::source_location::current()) noexcept;

public:
	void operator=(error const&) = delete;

private:
	error();

public:
	bool debug_flag;

private:
	std::vector<std::string> warnings,
							 minor_error;
};

}