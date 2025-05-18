#pragma once

#include "ntest.hpp"
#include "lexer/lexer.hpp"
#include "parser/statement_parser.hpp"
#include "parser/expression_parser.hpp"
#include "parser/code_gen.hpp"
#include "parser/statement_scope.hpp"
#include "interpreter/interpreter.hpp"
#include "common/error.hpp"

#include <iostream>
#include <sstream>
#include <string>

#include <iostream>
#include <string>
#include <cstdio>
#include <fstream>

std::string test_predefined_function_conversions()
{
	std::string file_name = create_test_file(
		"print(char(97));"
		"print(char(\"9\"));"
		"print(int(true));"
		"print(int('a'));"
		"print(int(3.14));"
		"print(int(\"7\"));"
		"print(float(true));"
		"print(float('a'));"
		"print(float(4));"
		"print(float(\"2.71\"));"
		"print(\"a\" + str('b'));"
		"print(\"a\" + str(3));"
		"print(\"a\" + str(3.14));"
	);

	auto statements = parse_file(file_name);
	auto bytecodes = code_gen(statements);

	char out[64];
	out[0] = '\0';

	InterpreterScope scope;
	interpret_bytecodes(scope, bytecodes, true, out);

	night_assert_eq(std::string(out),
		"a"
		"9"
		"1"
		"97"
		"3"
		"7"
		"1f"
		"97f"
		"4f"
		"2.7100000381469727f"
		"ab"
		"a3"
		"a3.140000"
	);

	return "";
}