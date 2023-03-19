#include "test_parser.hpp"
#include "night_tests.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "error.hpp"

#include <iostream>

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

	std::clog << " - variable initialization without assignment\n";
	lexer.scan_code("var1 int;");

	codes = parse_var(lexer, scope);

	night_assert_expect_bytecode(0, BytecodeType::CONSTANT);
	night_assert_expect_bytecode(1, BytecodeType::ASSIGN);


	std::clog << " - variable initialization with assignment\n";
	lexer.scan_code("var2 int = 2 + 3");

	codes = parse_var(lexer, scope);

	night_assert_expect_bytecode(0, BytecodeType::CONSTANT);
	night_assert_expect_bytecode(1, BytecodeType::CONSTANT);
	night_assert_expect_bytecode(2, BytecodeType::ADD);
	night_assert_expect_bytecode(3, BytecodeType::ASSIGN);


	std::clog << " - variable assign\n";
	lexer.scan_code("var2 = 2;");

	codes = parse_var(lexer, scope);

	night_assert_expect_bytecode(0, BytecodeType::CONSTANT);
	night_assert_expect_bytecode(1, BytecodeType::ASSIGN);


	std::clog << " - variable add assign\n";
	lexer.scan_code("var2 += 2;");

	codes = parse_var(lexer, scope);

	night_assert_expect_bytecode(0, BytecodeType::CONSTANT);
	night_assert_expect_bytecode(1, BytecodeType::ASSIGN);
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
	night_assert_expect_bytecode(2, BytecodeType::END_IF);

	std::clog << " - if statement with statement";
	lexer.scan_code("if (true) { var int; }");

	codes = parse_else(lexer, scope);

	night_assert_expect_bytecode(0, BytecodeType::CONSTANT);
	night_assert_expect_bytecode(0, BytecodeType::IF);
	night_assert_expect_bytecode(1, BytecodeType::CONSTANT);
	night_assert_expect_bytecode(2, BytecodeType::ASSIGN);
	night_assert_expect_bytecode(3, BytecodeType::END_IF);
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
	night_assert_expect_bytecode(1, BytecodeType::END_IF);

	std::clog << " - else statement with statement";
	lexer.scan_code("else { var int; }");

	codes = parse_else(lexer, scope);

	night_assert_expect_bytecode(0, BytecodeType::ELSE);
	night_assert_expect_bytecode(1, BytecodeType::CONSTANT);
	night_assert_expect_bytecode(2, BytecodeType::ASSIGN);
	night_assert_expect_bytecode(3, BytecodeType::END_IF);
}