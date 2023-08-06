/*
 * this file contains internal functions for debugging,
 * and should NOT impact production
 */

#pragma once

#include "bytecode.hpp"

#include <iostream>
#include <source_location>
#include <stdexcept>
#include <sstream>

namespace debug
{

template <typename T>
[[nodiscard]]
std::runtime_error unhandled_case(T val, std::source_location const& s_loc = std::source_location::current())
{
	std::stringstream s;
	s << "    [unhandled case]\n"
	  << "    " << s_loc.file_name() <<'\n'
	  << "    " << s_loc.function_name() << '\n'
	  << "    line: " << s_loc.line() << "\n    " << val << '\n';

	throw std::runtime_error(s.str());
}

template <typename T>
void log(T out) { std::cout << out; }

template <typename T>
void logn(T out) { std::cout << out << '\n'; }

// prints out all the bytecodes
void log_codes(bytecodes_t const& codes);

}