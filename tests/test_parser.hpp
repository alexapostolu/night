#pragma once

#include "lexer.hpp"
#include <string>

void test_parser();
void test_parse_var(Lexer& lexer);
void scan_code(std::string const& code);