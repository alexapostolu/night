#pragma once

#include "lexer.hpp"
#include "bytecode.hpp"

class Parser
{
public:
	Parser(Lexer const& lexer, bytecodes_t& bytecodes);
};