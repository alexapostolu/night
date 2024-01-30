#pragma once

#include "../code/include/bytecode.hpp"
#include "../code/include/parser_scope.hpp"
#include "../code/include/lexer.hpp"

#include <string>

void test_parser();
void test_parse_var();
void test_parse_conditional();
void test_parse_for();
void test_parse_while();
void test_generate_func();
void test_parse_rtn();

struct Test
{
	static void test(std::string const& msg, std::string const& code);

	static void expect(bytecode_t type);
	static void expect(BytecodeType type, int val = -1);

private:
	static StatementScope scope;
	static Lexer lexer;

	static int i;
	static bytecodes_t codes;
};