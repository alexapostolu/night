/*
 * this file contains internal functions for debugging,
 * and should NOT impact production
 * (if they do i fucked up)
 */

#pragma once

#include "bytecode.hpp"

#include <source_location>
#include <stdexcept>
#include <sstream>

namespace debug
{

template <typename T>
void throw_unhandled_case(T val, std::source_location const& s_loc = std::source_location::current())
{
	std::stringstream s;
	s << s_loc.file_name() << '\n' + s_loc.function_name() << '\n' << val;

	throw std::runtime_error(s.str());
}

// prints out all the bytecodes
void log_codes(bytecodes_t const& codes);

}