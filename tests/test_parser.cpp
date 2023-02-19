#include "test_parser.hpp"
#include "night_tests.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "error.hpp"

#include <iostream>

void test_parser()
{
	std::clog << "testing parser\n\n";


	Lexer lexer("test_file.night");
	Scope global_scope;


	test_parse_var(lexer);

}

void test_parse_var(Lexer& lexer)
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

	night_assert_expect_bytecode(2, BytecodeType::CONSTANT);
	night_assert_expect_bytecode(3, BytecodeType::CONSTANT);
	night_assert_expect_bytecode(4, BytecodeType::ADD);
	night_assert_expect_bytecode(5, BytecodeType::ASSIGN);


	std::clog << " - variable assign\n";
	lexer.scan_code("var2 = 2;");

	codes = parse_var(lexer, scope);

	night_assert_expect_bytecode(2, BytecodeType::CONSTANT);
	night_assert_expect_bytecode(5, BytecodeType::ASSIGN);


	std::clog << " - variable add assign\n";
	lexer.scan_code("var2 += 2;");

	codes = parse_var(lexer, scope);

	night_assert_expect_bytecode(6, BytecodeType::CONSTANT);
	night_assert_expect_bytecode(7, BytecodeType::ASSIGN);
}

void scan_code(std::string const& code)
{

}