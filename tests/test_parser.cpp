#include "test_parser.hpp"
#include "../code/include/lexer.hpp"
#include "../code/include/parser.hpp"
#include "../code/include/bytecode.hpp"
#include "../code/include/error.hpp"

#include <iostream>

void test_parser()
{
	std::clog << "testing parser\n\n";

	test_parse_var();
}

void test_parse_var()
{
	std::clog << "testing parse_var\n";


	Test::test("variable declaration",
		"var0 bool;");
	Test::expect(BytecodeType::BOOL);   Test::expect(0);
	Test::expect(BytecodeType::U_INT1); Test::expect(0);
	Test::expect(BytecodeType::STORE);


	Test::test("variable initialization",
		"var1 int32 = 2;");
	Test::expect(BytecodeType::S_INT4); Test::expect(2);
	Test::expect(BytecodeType::U_INT1); Test::expect(1);
	Test::expect(BytecodeType::STORE);


	Test::test("variable assignment",
		"var1 = 5;");
	Test::expect(BytecodeType::S_INT4); Test::expect(5);
	Test::expect(BytecodeType::U_INT1); Test::expect(1);
	Test::expect(BytecodeType::STORE);


	Test::test("variable add assignment",
		"var1 += 7;");
	Test::expect(BytecodeType::S_INT4);   Test::expect(7);
	Test::expect(BytecodeType::LOAD);	  Test::expect(1);
	Test::expect(BytecodeType::ADD);
	Test::expect(BytecodeType::STORE);    Test::expect(1);


	Test::test("expressions",
		"var2 int16 = var0 + var1 * 7");
	Test::expect(BytecodeType::LOAD);	  Test::expect(0);
	Test::expect(BytecodeType::S_INT2);   Test::expect(7);
	Test::expect(BytecodeType::MULT);  
	Test::expect(BytecodeType::LOAD);	  Test::expect(1);
	Test::expect(BytecodeType::ADD);
	Test::expect(BytecodeType::U_INT1);	  Test::expect(2);
	Test::expect(BytecodeType::STORE);	  Test::expect(2);
}

void test_parse_conditional()
{
	std::clog << "testing parse_if\n";


	Test::test("if statement",
		"if (true) { var float8; }");
	Test::expect(BytecodeType::BOOL, 1);
	Test::expect(BytecodeType::JUMP_IF_FALSE, 6);
	Test::expect(BytecodeType::FLOAT8, 0);
	Test::expect(BytecodeType::STORE, 0);
	Test::expect(BytecodeType::JUMP, 0);

	Test::test("if and elif statement",
		"if (true) {} elif (true) { var float8; }");
	Test::expect(BytecodeType::BOOL, 1);
	Test::expect(BytecodeType::JUMP_IF_FALSE, 2);
	Test::expect(BytecodeType::JUMP, 10);
	Test::expect(BytecodeType::BOOL, 1);
	Test::expect(BytecodeType::JUMP_IF_FALSE, 6);
	Test::expect(BytecodeType::FLOAT8, 0);
	Test::expect(BytecodeType::STORE, 0);
	Test::expect(BytecodeType::JUMP, 0);

	Test::test("if, elif, and else statement",
		"if (true) {} elif (true) {} else { var char; }");
	Test::expect(BytecodeType::BOOL, 1);
	Test::expect(BytecodeType::JUMP_IF_FALSE, 2);
	Test::expect(BytecodeType::JUMP, 10);
	Test::expect(BytecodeType::BOOL, 1);
	Test::expect(BytecodeType::JUMP_IF_FALSE, 2);
	Test::expect(BytecodeType::JUMP, 4);
	Test::expect(BytecodeType::CHAR1, 0);
	Test::expect(BytecodeType::STORE, 0);
}

void test_parse_for()
{
}

void test_parse_while() {}
void test_parse_rtn() {}

void test_generate_func()
{
	Test::test("function without paramters",
		"def func1() {}");
}

void Test::test(std::string const& msg, std::string const& code)
{
	std::clog << " - " << msg << '\b';

	lexer.scan_code(code);

	i = 0;
	codes = parse_stmt(lexer, scope)->generate_codes();
}

void Test::expect(bytecode_t value)
{
	std::clog << " . " << codes[i] << " == " << value;
	if (codes[i] != value)
		std::clog << " . . assertion failed\n";
	else
		std::clog << " . . assertion passed\n";

	++i;
}

void Test::expect(BytecodeType value, int val)
{
	std::clog << " . " << night::to_str(codes[i]) << " == " << night::to_str(value);
	if (codes[i] != (bytecode_t)value)
		std::clog << " . . assertion failed\n";
	else
		std::clog << " . . assertion passed\n";

	++i;

	if (val != -1)
		expect(val);
}