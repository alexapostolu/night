#pragma once

#include "lexer.hpp"

class Parser
{
public:
	Parser(Lexer const& lexer);
};

void parse_file