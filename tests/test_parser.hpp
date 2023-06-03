#pragma once

#include "bytecode.hpp"
#include "scope.hpp"
#include "lexer.hpp"

void test_parser();
void test_parse_var();
void test_parse_if();
void test_parse_else();
void test_parse_for();
void test_parse_while();
void test_parse_rtn();

struct Wrap
{
	static void test(std::string const& msg, std::string const& code);
	static void expect(bytecode_t type);
	static void expect(BytecodeType type);

private:
	static Scope scope;
	static Lexer lexer;

	static int i;
	static bytecodes_t& codes;
};