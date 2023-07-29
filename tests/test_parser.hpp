#pragma once

#include "bytecode.hpp"
#include "parser_scope.hpp"
#include "lexer.hpp"

class Lexer;

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
	static ParserScope scope;
	static Lexer lexer;

	static int i;
	static bytecodes_t& codes;
};