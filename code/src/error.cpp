#include "error.hpp"

std::string const& night::fatal_error::what() const noexcept
{
	return msg;
}

night::error::error()
	: debug_flag(false) {}

night::error& night::error::get()
{
	static error instance;
	return instance;
}

void night::error::create_warning(std::string const& msg) noexcept
{
	warnings.push_back("");
}

void night::error::create_minor_error(std::string const& msg) noexcept
{
	minor_error.push_back("");
}

night::fatal_error night::error::create_fatal_error(std::string const& msg) noexcept
{
	return fatal_error{ msg };
}