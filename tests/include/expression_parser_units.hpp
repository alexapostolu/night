#pragma once

#include "ntest.hpp"
#include "lexer/lexer.hpp"
#include "parser/ast/expression_operator.hpp"
#include "parser/expression_parser.hpp"

#include <ostream>

std::ostream& operator<<(std::ostream& os, expr::UnaryOpType type)
{
	switch (type) {
	case expr::UnaryOpType::NEGATIVE:	return os << "UnaryOpType::NEGATIVE";
	case expr::UnaryOpType::NOT:		return os << "UnaryOpType::NOT";
	default: return os << "UNKNOWN";
	}
}

std::ostream& operator<<(std::ostream & os, expr::BinaryOpType type)
{
	switch (type) {
	case expr::BinaryOpType::ADD:             return os << "BinaryOpType::ADD";
	case expr::BinaryOpType::SUB:             return os << "BinaryOpType::SUB";
	case expr::BinaryOpType::MULT:            return os << "BinaryOpType::MULT";
	case expr::BinaryOpType::DIV:             return os << "BinaryOpType::DIV";
	case expr::BinaryOpType::MOD:             return os << "BinaryOpType::MOD";
	case expr::BinaryOpType::LESSER:          return os << "BinaryOpType::LESSER";
	case expr::BinaryOpType::GREATER:         return os << "BinaryOpType::GREATER";
	case expr::BinaryOpType::LESSER_EQUALS:   return os << "BinaryOpType::LESSER_EQUALS";
	case expr::BinaryOpType::GREATER_EQUALS:  return os << "BinaryOpType::GREATER_EQUALS";
	case expr::BinaryOpType::EQUALS:          return os << "BinaryOpType::EQUALS";
	case expr::BinaryOpType::NOT_EQUALS:      return os << "BinaryOpType::NOT_EQUALS";
	case expr::BinaryOpType::AND:             return os << "BinaryOpType::AND";
	case expr::BinaryOpType::OR:              return os << "BinaryOpType::OR";
	case expr::BinaryOpType::SUBSCRIPT:       return os << "BinaryOpType::SUBSCRIPT";
	default: return os << "UNKNOWN";
	}
}

std::ostream& operator<<(std::ostream& os, TokenType type) {
	switch (type) {
	case TokenType::EXPR:             return os << "TokenType::EXPR";

	case TokenType::OPEN_BRACKET:     return os << "TokenType::OPEN_BRACKET";
	case TokenType::CLOSE_BRACKET:    return os << "TokenType::CLOSE_BRACKET";
	case TokenType::OPEN_SQUARE:      return os << "TokenType::OPEN_SQUARE";
	case TokenType::CLOSE_SQUARE:     return os << "TokenType::CLOSE_SQUARE";
	case TokenType::OPEN_CURLY:       return os << "TokenType::OPEN_CURLY";
	case TokenType::CLOSE_CURLY:      return os << "TokenType::CLOSE_CURLY";
	case TokenType::COLON:            return os << "TokenType::COLON";
	case TokenType::SEMICOLON:        return os << "TokenType::SEMICOLON";
	case TokenType::COMMA:            return os << "TokenType::COMMA";

	case TokenType::UNARY_OPERATOR:   return os << "TokenType::UNARY_OPERATOR";
	case TokenType::BINARY_OPERATOR:  return os << "TokenType::BINARY_OPERATOR";

	case TokenType::BOOL_LIT:         return os << "TokenType::BOOL_LIT";
	case TokenType::CHAR_LIT:         return os << "TokenType::CHAR_LIT";
	case TokenType::INT_LIT:          return os << "TokenType::INT_LIT";
	case TokenType::FLOAT_LIT:        return os << "TokenType::FLOAT_LIT";
	case TokenType::STRING_LIT:       return os << "TokenType::STRING_LIT";

	case TokenType::VARIABLE:         return os << "TokenType::VARIABLE";

	case TokenType::TYPE:             return os << "TokenType::TYPE";

	case TokenType::IF:               return os << "TokenType::IF";
	case TokenType::ELIF:             return os << "TokenType::ELIF";
	case TokenType::ELSE:             return os << "TokenType::ELSE";

	case TokenType::FOR:              return os << "TokenType::FOR";
	case TokenType::WHILE:            return os << "TokenType::WHILE";

	case TokenType::DEF:              return os << "TokenType::DEF";
	case TokenType::VOID:             return os << "TokenType::VOID";
	case TokenType::RETURN:           return os << "TokenType::RETURN";

	case TokenType::END_OF_FILE:      return os << "TokenType::END_OF_FILE";

	default: return os << "UNKNOWN";
	}
}

expr::expr_p setup_expression_parser(std::string const& code, bool err_on_empty = false,
	std::optional<TokenType> const& end_token = std::nullopt)
{
	std::string file_name = create_test_file(code);
	Lexer lexer(file_name);

	return parse_expr(lexer, err_on_empty, end_token);
}

std::string test_expression_parser_basic()
{
	std::string file_name = create_test_file(
		"= 2 + 3;"
	);

	Lexer lexer(file_name);

	expr::expr_p expr = parse_expr(lexer, false, TokenType::SEMICOLON);
	night_assert_notnull(expr);

	auto add = std::dynamic_pointer_cast<expr::BinaryOp>(expr);
	night_assert_notnull(add);
	night_assert_eq(add->get_type(), expr::BinaryOpType::ADD);

	auto two = std::dynamic_pointer_cast<expr::Numeric>(add->get_lhs());
	night_assert_notnull(two);
	night_assert_tr(std::holds_alternative<int64_t>(two->get_val()));
	night_assert_eq(std::get<int64_t>(two->get_val()), 2);
	
	auto three = std::dynamic_pointer_cast<expr::Numeric>(add->get_rhs());
	night_assert_notnull(three);
	night_assert_tr(std::holds_alternative<int64_t>(three->get_val()));
	night_assert_eq(std::get<int64_t>(three->get_val()), 3);

	return "";
}

std::string test_expression_parser_negative_or_subtract()
{
	std::string file_name = create_test_file(
		"= -1 - 2"
	);

	Lexer lexer(file_name);

	expr::expr_p expr = parse_expr(lexer, false);

	auto subtract = std::dynamic_pointer_cast<expr::BinaryOp>(expr);
	night_assert_notnull(subtract);
	night_assert_eq(subtract->get_type(), expr::BinaryOpType::SUB);

	auto negative = std::dynamic_pointer_cast<expr::UnaryOp>(subtract->get_lhs());
	night_assert_notnull(negative);
	night_assert_eq(negative->get_type(), expr::UnaryOpType::NEGATIVE);

	auto one = std::dynamic_pointer_cast<expr::Numeric>(negative->get_expr());
	night_assert_notnull(one);
	night_assert_tr(std::holds_alternative<int64_t>(one->get_val()));
	night_assert_eq(std::get<int64_t>(one->get_val()), 1);

	auto two = std::dynamic_pointer_cast<expr::Numeric>(subtract->get_rhs());
	night_assert_notnull(two);
	night_assert_tr(std::holds_alternative<int64_t>(two->get_val()));
	night_assert_eq(std::get<int64_t>(two->get_val()), 2);

	return "";
}

std::string test_expression_parser_subscript()
{
	std::string file_name = create_test_file(
		"= -arr[1][2] + 3"
	);

	Lexer lexer(file_name);

	expr::expr_p expr = parse_expr(lexer, false);

	auto add = std::dynamic_pointer_cast<expr::BinaryOp>(expr);
	night_assert_notnull(add);
	night_assert_eq(add->get_type(), expr::BinaryOpType::ADD);

	// Tests subscript binary operator has higher precedence than unary operator.
	// This is important because most other binary operators have lower precedence
	// than unary operators.
	auto negative = std::dynamic_pointer_cast<expr::UnaryOp>(add->get_lhs());
	night_assert_notnull(negative);
	night_assert_eq(negative->get_type(), expr::UnaryOpType::NEGATIVE);

	auto sub2 = std::dynamic_pointer_cast<expr::BinaryOp>(negative->get_expr());
	night_assert_notnull(sub2);
	night_assert_eq(sub2->get_type(), expr::BinaryOpType::SUBSCRIPT);

	auto two = std::dynamic_pointer_cast<expr::Numeric>(sub2->get_lhs());
	night_assert_notnull(two);
	night_assert_tr(std::holds_alternative<int64_t>(two->get_val()));
	night_assert_eq(std::get<int64_t>(two->get_val()), 2);

	auto sub1 = std::dynamic_pointer_cast<expr::BinaryOp>(sub2->get_rhs());
	night_assert_notnull(sub1);
	night_assert_eq(sub1->get_type(), expr::BinaryOpType::SUBSCRIPT);

	auto one = std::dynamic_pointer_cast<expr::Numeric>(sub1->get_lhs());
	night_assert_notnull(one);
	night_assert_tr(std::holds_alternative<int64_t>(one->get_val()));
	night_assert_eq(std::get<int64_t>(one->get_val()), 1);

	auto arr = std::dynamic_pointer_cast<expr::Variable>(sub1->get_rhs());
	night_assert_notnull(arr);

	auto three = std::dynamic_pointer_cast<expr::Numeric>(add->get_rhs());
	night_assert_notnull(three);
	night_assert_tr(std::holds_alternative<int64_t>(three->get_val()));
	night_assert_eq(std::get<int64_t>(three->get_val()), 3);

	return "";
}

std::string test_expression_parser_subscript_invalid()
{
	try {
		expr::expr_p expr = setup_expression_parser("= arr[1]2]");
		night_assert_tr(false);
	}
	catch (night::error const&) { }

	try {
		expr::expr_p expr = setup_expression_parser("= arr[1 2]");
		night_assert_tr(false);
	}
	catch (night::error const&) { }

	try {
		expr::expr_p expr = setup_expression_parser("= arr[1");
		night_assert_tr(false);
	}
	catch (night::error const&) { }

	return "";
}

std::string test_expression_parser_valid_ending_token()
{
	std::string file_name = create_test_file(
		"[ 2 + 3 ]"
	);

	Lexer lexer(file_name);

	expr::expr_p expr = parse_expr(lexer, false, TokenType::CLOSE_SQUARE);

	night_assert_notnull(expr);
	night_assert_eq(lexer.curr().type, TokenType::CLOSE_SQUARE);

	return "";
}

std::string test_expression_parser_invalid_ending_token()
{
	std::string file_name = create_test_file(
		"= 2 + 3 +"
	);

	Lexer lexer(file_name);

	bool error_thrown = false;

	try {
		expr::expr_p expr = parse_expr(lexer, false, TokenType::SEMICOLON);
	}
	catch (night::error const&) {
		error_thrown = true;
	}

	night_assert_tr(error_thrown);

	return "";
}

std::string test_expression_parser_order_of_operations()
{
	std::string file_name = create_test_file(
		"= 2 * (3 + 4 / 5);"
	);

	Lexer lexer(file_name);

	expr::expr_p expr = parse_expr(lexer, false, TokenType::SEMICOLON);
	night_assert_notnull(expr);

	auto mult = std::dynamic_pointer_cast<expr::BinaryOp>(expr);
	night_assert_notnull(mult);
	night_assert_eq(mult->get_type(), expr::BinaryOpType::MULT);

	auto two = std::dynamic_pointer_cast<expr::Numeric>(mult->get_lhs());
	night_assert_notnull(two);
	night_assert_tr(std::holds_alternative<int64_t>(two->get_val()));
	night_assert_eq(std::get<int64_t>(two->get_val()), 2);

	auto add = std::dynamic_pointer_cast<expr::BinaryOp>(mult->get_rhs());
	night_assert_notnull(add);
	night_assert_eq(add->get_type(), expr::BinaryOpType::ADD);

	auto three = std::dynamic_pointer_cast<expr::Numeric>(add->get_lhs());
	night_assert_notnull(three);
	night_assert_tr(std::holds_alternative<int64_t>(three->get_val()));
	night_assert_eq(std::get<int64_t>(three->get_val()), 3);

	auto divide = std::dynamic_pointer_cast<expr::BinaryOp>(add->get_rhs());
	night_assert_notnull(divide);
	night_assert_eq(divide->get_type(), expr::BinaryOpType::DIV);

	auto four = std::dynamic_pointer_cast<expr::Numeric>(divide->get_lhs());
	night_assert_notnull(four);
	night_assert_tr(std::holds_alternative<int64_t>(four->get_val()));
	night_assert_eq(std::get<int64_t>(four->get_val()), 4);

	auto five = std::dynamic_pointer_cast<expr::Numeric>(divide->get_rhs());
	night_assert_notnull(five);
	night_assert_tr(std::holds_alternative<int64_t>(five->get_val()));
	night_assert_eq(std::get<int64_t>(five->get_val()), 5);

	return "";
}

std::string test_expression_parser_invalid_expression()
{
	try {
		expr::expr_p expr = setup_expression_parser("= 2 + +", false, TokenType::SEMICOLON);
		night_assert_tr(false);
	}
	catch (night::error const&) { }

	// Test error on empty.

	try {
		expr::expr_p expr = setup_expression_parser("= 2 ;", true, TokenType::SEMICOLON);
	}
	catch (night::error const&) {
		night_assert_tr(false);
	}

	try {
		expr::expr_p expr = setup_expression_parser("= ;", true, TokenType::SEMICOLON);
		night_assert_tr(false);
	}
	catch (night::error const&) { }

	return "";
}
