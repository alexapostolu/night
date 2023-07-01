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
		"if (true) { var float8; }");
	Wrap::expect(BytecodeType::BOOL);   Wrap::expect(1);
	Wrap::expect(BytecodeType::IF);     Wrap::expect(4);
	Wrap::expect(BytecodeType::FLOAT8); Wrap::expect(0);
	Wrap::expect(BytecodeType::ASSIGN); Wrap::expect(0);

	Wrap::test("if and elif statement",
		"if (true) {} elif { var float8; }");
	Wrap::expect(BytecodeType::BOOL);	Wrap::expect(1);

	Wrap::expect(BytecodeType::BOOL);   Wrap::expect(1);
	Wrap::expect(BytecodeType::IF);     Wrap::expect(4);
	Wrap::expect(BytecodeType::FLOAT8); Wrap::expect(0);
	Wrap::expect(BytecodeType::ASSIGN); Wrap::expect(0);
}

void test_parse_else()
{
	std::clog << "testing parse_else\n";

	
	Wrap::test("else statement",
		"else { var char8; }");
	Wrap::expect(BytecodeType::ELSE);   Wrap::expect(4);
	Wrap::expect(BytecodeType::CHAR1);  Wrap::expect(0);
	Wrap::expect(BytecodeType::ASSIGN); Wrap::expect(0);
}

void test_parse_for()
{
	std::clog << "testing parse_for\n";

	Wrap::test("for statement with 3 conditions and body",
		"for (i int8 = 0; i < 5; i += 1) { var int8; }");
	Wrap::expect(3); Wrap::expect(BytecodeType::FOR); Wrap::expect(4);
	Wrap::expect(BytecodeType::S_INT8); Wrap::expect(0);
	Wrap::expect(BytecodeType::ASSIGN); Wrap::expect(0);


	Wrap::test("for statement with 2 conditions",
		"for (i int8 = 0; i < 5;) { }");
	Wrap::expect(2); Wrap::expect(BytecodeType::FOR); Wrap::expect(0);
}

void test_parse_while() {}
void test_parse_rtn() {}

void test_generate_func()
{
	Wrap::test("function without paramters",
		"def func1() {}");
	Wrap::expect(BytecodeType::)
}

void Wrap::test(std::string const& msg, std::string const& code)
{
	std::clog << " - " << msg << '\b';

	lexer.scan_code(code);

	i = 0;
	codes = parse_stmt(lexer, scope);
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