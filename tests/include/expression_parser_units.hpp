#pragma once

#include "lexer.hpp"
#include "ast/expression_operator.hpp"
#include "parser/expression_parser.hpp"

#include <fstream>
#include <filesystem>
#include <iostream>

#define night_assert(expr, err) if (!(expr)) return (err);

static int files = 0;

std::string create_test_file(std::string const& code)
{
	std::string file_name = std::to_string(files);
	std::string content = "= 2 + 3";

	std::ofstream file(file_name);
	file << content;

	files++;

	return file_name;
}

std::string expression_parser_tests()
{
	std::string file_name = create_test_file(
		"= 2 + 3"
	);

	Lexer lexer(file_name);

	expr::expr_p expr = parse_expr(lexer, false);

	night_assert(
		std::dynamic_pointer_cast<expr::BinaryOp>(expr),
		"Expected expression to have binary operator PLUS as its head.\n"
		"2 + 3");

	return "";
}
