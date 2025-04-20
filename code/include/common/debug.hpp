#pragma once

#include <source_location>
#include <stdexcept>
#include <sstream>

template <typename T>
concept Printable = requires(T t, std::stringstream ss) { ss << t; };

namespace debug
{

template <Printable T>
[[nodiscard]]
std::runtime_error unhandled_case(T type, std::source_location const& s_loc = std::source_location::current())
{
	std::stringstream s;
	s << "    [Unhandled Case]\n"
	  << "    " << s_loc.file_name() << ':' << s_loc.line() << '\n'
	  << "    " << s_loc.function_name() << '\n'
	  << "    " << type << '\n';

	return std::runtime_error(s.str());
}

}