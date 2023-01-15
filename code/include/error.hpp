#pragma once

#include <string>
#include <vector>

#define NIGHT_CREATE_WARNING(msg) night::error::get().create_warning(msg, __FILE__, __LINE__);
#define NIGHT_CREATE_MINOR(msg)	  night::error::get().create_minor_error(msg, __FILE__, __LINE__);
#define NIGHT_CREATE_FATAL(msg)   night::error::get().create_fatal_error(msg, __FILE__, __LINE__);

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
	void create_minor_error(std::string const& msg, std::string const& file, int line) noexcept;
	fatal_error create_fatal_error(std::string const& msg, std::string const& file, int line) noexcept;

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