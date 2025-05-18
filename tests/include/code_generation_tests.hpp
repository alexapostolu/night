#pragma once

#include "ntest.hpp"
#include "lexer/lexer.hpp"
#include "parser/statement_parser.hpp"
#include "parser/statement_scope.hpp"
#include "parser/code_gen.hpp"
#include "interpreter/interpreter.hpp"
#include "common/bytecode.hpp"
#include "common/error.hpp"


std::string test_code_gen_expression_basic()
{
	std::string file_name = create_test_file(
		"= 2 + 3"
	);

	Lexer lexer(file_name);

	expr::expr_p expr = parse_expr(lexer, false);
	
	StatementScope scope;
	expr->type_check(scope);
	(void)expr->optimize(scope); // Unnecessary for this unit test, but consistent with intended usage
	bytecodes_t bytes = expr->generate_codes();

	int i = 0;
	bytecode_t expected[] = {
		ByteType_sINT8, 2, 0, 0, 0, 0, 0, 0, 0,
		ByteType_sINT8, 3, 0, 0, 0, 0, 0, 0, 0,
		BytecodeType_ADD_I };

	night_assert_eq(bytes.size(), sizeof(expected) / sizeof(expected[0]));

	for (auto const& byte : bytes)
		night_assert_eq(byte, expected[i++]);

	return "";
}

std::string test_code_gen_variable_init()
{
	std::string file_name = create_test_file(
		"my_var int = 2 + 3;"
	);

	std::vector<stmt_p> statements = parse_file(file_name);

	bytecodes_t bytes = code_gen(statements);

	int i = 0;
	bytecode_t expected[] = {
		ByteType_sINT8, 5, 0, 0, 0, 0, 0, 0, 0,
		ByteType_uINT8, 0, 0, 0, 0, 0, 0, 0, 0,
		ByteType_STORE };

	night_assert_eq(bytes.size(), sizeof(expected) / sizeof(expected[0]));

	for (auto const& byte : bytes)
		night_assert_eq(byte, expected[i++]);

	return "";
}