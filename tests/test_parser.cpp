#include "test_parser.hpp"
#include "night_tests.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "error.hpp"

#include <iostream>

#define night_assert_expect_bytecode(i, _type) night_assert("expected " + bytecode_to_str(_type), codes[(i)].type == (_type));

void test_parser()
{
	std::clog << "testing parser\n\n";

	test_parse_var();
}

void test_parse_var()
{
	std::clog << "testing parse_var\n";

	Lexer lexer;
	Scope scope;
	bytecodes_t codes;

	std::clog << " - variable declaration\n";
	lexer.scan_code("var1 bool;");

	codes = parse_var(lexer, scope);

	night_assert_expect_bytecode(0, BytecodeType::CONSTANT);
	night_assert_expect_bytecode(1, BytecodeType::BOOL_ASSIGN);


	std::clog << " - variable initialization\n";
	lexer.scan_code("var2 int = 2 + 3");

	codes = parse_var(lexer, scope);

	night_assert_expect_bytecode(0, BytecodeType::CONSTANT);
	night_assert_expect_bytecode(1, BytecodeType::CONSTANT);
	night_assert_expect_bytecode(2, BytecodeType::ADD);
	night_assert_expect_bytecode(3, BytecodeType::INT_ASSIGN);


	std::clog << " - variable assignment\n";
	lexer.scan_code("var2 = 2;");

	codes = parse_var(lexer, scope);

	night_assert_expect_bytecode(0, BytecodeType::CONSTANT);
	night_assert_expect_bytecode(1, BytecodeType::INT_ASSIGN);


	std::clog << " - variable add assignment\n";
	lexer.scan_code("var2 += 2;");

	codes = parse_var(lexer, scope);

	night_assert_expect_bytecode(0, BytecodeType::CONSTANT);
	night_assert_expect_bytecode(1, BytecodeType::ADD_ASSIGN);
}

void test_parse_if()
{
	std::clog << "testing parse_if\n";

	Lexer lexer;
	Scope scope;
	bytecodes_t codes;

	std::clog << " - if statement\n";
	lexer.scan_code("if (true) {}");

	codes = parse_else(lexer, scope);

	night_assert_expect_bytecode(0, BytecodeType::CONSTANT);
	night_assert_expect_bytecode(1, BytecodeType::IF);
	night_assert("bytecode 'if' has the wrong identifier", codes[1].val = 1);


	std::clog << " - if statement with body";
	lexer.scan_code("if (true) { var int; }");

	codes = parse_else(lexer, scope);

	night_assert_expect_bytecode(0, BytecodeType::CONSTANT);
	night_assert_expect_bytecode(0, BytecodeType::IF);
	night_assert_expect_bytecode(1, BytecodeType::CONSTANT);
	night_assert_expect_bytecode(2, BytecodeType::INT_ASSIGN);
}

void test_parse_else()
{
	std::clog << "testing parse_else\n";

	Lexer lexer;
	Scope scope;
	bytecodes_t codes;

	std::clog << " - else statement\n";
	lexer.scan_code("else {}");

	codes = parse_else(lexer, scope);

	night_assert_expect_bytecode(0, BytecodeType::ELSE);

	std::clog << " - else statement with statement";
	lexer.scan_code("else { var int; }");

	codes = parse_else(lexer, scope);

	night_assert_expect_bytecode(0, BytecodeType::ELSE);
	night_assert_expect_bytecode(1, BytecodeType::CONSTANT);
	night_assert_expect_bytecode(2, BytecodeType::INT_ASSIGN);
}

void test_parse_for()
{
	std::clog << "testing parse_for\n";

	Lexer lexer;
	Scope scope;
	bytecodes_t codes;

	std::clog << " - for statement\n";
	lexer.scan_code("for (int i = 0; i < 5; i += 1) {}");

	codes = parse_else(lexer, scope);

	night_assert_expect_bytecode(0, BytecodeType::FOR);
}

void test_parse_while();
void test_parse_rtn();