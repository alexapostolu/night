#include "debug.hpp"
#include "statement_scope.hpp"
#include "bytecode.hpp"

#include <iostream>
#include <stack>

void debug::log_codes(bytecodes_t const& codes)
{
	return;

	std::clog << "[printing bytecodes]\n";
	std::clog << "[end of bytecodes]";
}