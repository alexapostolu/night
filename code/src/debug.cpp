#include "debug.hpp"
#include "statement_scope.hpp"
#include "bytecode.h"

#include <iostream>
#include <stack>

void debug::log_codes(bytes_t const& codes)
{
	return;

	std::clog << "[printing bytecodes]\n";
	std::clog << "[end of bytecodes]";
}