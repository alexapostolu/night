#pragma once

#include <string>
#include <vector>

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

	void create_warning(std::string const& msg) noexcept;
	void create_minor_error(std::string const& msg) noexcept;
	fatal_error create_fatal_error(std::string const& msg) noexcept;

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