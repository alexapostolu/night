#pragma once

#include "lexer.hpp"
#include "ast/expression_operator.hpp"
#include "parser/expression_parser.hpp"

#include <fstream>
#include <filesystem>
#include <iostream>

#define night_assert(expr, err) if (!(expr)) return (err);

int files = 0;

std::string create_test_file(std::string const& code)
{
	std::string file_name = std::to_string(files);
	std::string content = code;

	std::ofstream file(file_name);
	file << content;

	files++;
	file.close();

	return file_name;
}

void delete_file(const std::string& filePath) {
	try {
		std::filesystem::remove(filePath);
	}
	catch (const std::filesystem::filesystem_error& e) {
		std::cerr << "Filesystem error: " << e.what() << "\n";
	}
}

std::string test_expression_parser_basic()
{
	std::string file_name = create_test_file(
		"= 2 + 3"
	);

	Lexer lexer(file_name);

	expr::expr_p expr = parse_expr(lexer, false);
	auto binary_op = std::dynamic_pointer_cast<expr::BinaryOp>(expr);

	night_assert(
		binary_op && binary_op->get_type() == expr::BinaryOpType::ADD,
		"Expected expression to have binary operator ADD as its head.");

	auto lhs = std::dynamic_pointer_cast<expr::Numeric>(binary_op->get_lhs());
	auto rhs = std::dynamic_pointer_cast<expr::Numeric>(binary_op->get_rhs());

	night_assert(
		lhs && rhs,
		"Expected left and right hand side expressions to be Numeric.");

	night_assert(
		std::holds_alternative<int64_t>(lhs->get_val()) && std::holds_alternative<int64_t>(lhs->get_val()),
		"Expected left and right hand side expressions to be int64_t.");

	//lexer.~Lexer();
	//delete_file(file_name);
	return "";
}

std::string test_expression_parser_order_of_operations()
{
	std::string file_name = create_test_file(
		"= 2 * (3 + 4 * 5)"
	);

	Lexer lexer(file_name);

	expr::expr_p expr = parse_expr(lexer, false);
	auto binary_op = std::dynamic_pointer_cast<expr::BinaryOp>(expr);

	night_assert(
		binary_op && binary_op->get_type() == expr::BinaryOpType::MULT,
		"Expected expression to have binary operator MULT as its head.");


	auto lhs = std::dynamic_pointer_cast<expr::Numeric>(binary_op->get_lhs());
	auto rhs = std::dynamic_pointer_cast<expr::BinaryOp>(binary_op->get_rhs());

	night_assert(
		lhs && std::holds_alternative<int64_t>(lhs->get_val()),
		"Expected left hand side to be Numeric.");

	night_assert(
		rhs && rhs->get_type() == expr::BinaryOpType::ADD,
		"Expected right hand side to be ADD.");

	//lexer.~Lexer();
	//delete_file(file_name);
	return "";
}
