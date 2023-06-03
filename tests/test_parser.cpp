#include "test_parser.hpp"
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


	Wrap::test("variable declaration",
		"var1 bool;");
	Wrap::expect(BytecodeType::BOOL);   Wrap::expect(0);
	Wrap::expect(BytecodeType::ASSIGN); Wrap::expect(0);


	Wrap::test("variable initialization",
		"var2 int32 = 2;");
	Wrap::expect(BytecodeType::S_INT4); Wrap::expect(2);
	Wrap::expect(BytecodeType::ASSIGN); Wrap::expect(1);


	Wrap::test("variable assignment",
		"var2 = 5;");
	Wrap::expect(BytecodeType::S_INT4); Wrap::expect(5);
	Wrap::expect(BytecodeType::ASSIGN); Wrap::expect(1);


	Wrap::test("variable add assignment",
		"var2 += 7;");
	Wrap::expect(BytecodeType::S_INT4);   Wrap::expect(7);
	Wrap::expect(BytecodeType::VARIABLE); Wrap::expect(1);
	Wrap::expect(BytecodeType::ADD);
	Wrap::expect(BytecodeType::ASSIGN);   Wrap::expect(1);


	Wrap::test("expressions",
		"var3 int16 = var1 + var2 * 7");
	Wrap::expect(BytecodeType::VARIABLE); Wrap::expect(1);
	Wrap::expect(BytecodeType::S_INT2);   Wrap::expect(7);
	Wrap::expect(BytecodeType::MULT);
	Wrap::expect(BytecodeType::VARIABLE); Wrap::expect(0);
	Wrap::expect(BytecodeType::ADD);
	Wrap::expect(BytecodeType::ASSIGN);   Wrap::expect(2);
}

void test_parse_if()
{
	std::clog << "testing parse_if\n";

	Wrap::test("if statement",
		"if (true) {}");
	Wrap::expect(BytecodeType::BOOL);
	Wrap::expect(1);
	Wrap::expect(BytecodeType::IF);
	Wrap::expect

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

void test_parse_while() {}
void test_parse_rtn() {}

void Wrap::test(std::string const& msg, std::string const& code)
{
	std::clog << " - " << msg << '\b';

	lexer.scan_code(code);

	i = 0;
	codes = parse_var(lexer, scope);
}

void Wrap::expect(bytecode_t value)
{
	std::clog << " . " << codes[i] << " == " << value;
	if (codes[i] != value)
		std::clog << " . . assertion failed\n";
	else
		std::clog << " . . assertion passed\n";

	++i;
}

void Wrap::expect(BytecodeType value)
{
	std::clog << " . " << bytecode_to_str(codes[i]) << " == " << bytecode_to_str(value);
	if (codes[i] != (bytecode_t)value)
		std::clog << " . . assertion failed\n";
	else
		std::clog << " . . assertion passed\n";

	++i;
}