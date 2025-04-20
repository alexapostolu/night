#pragma once

#include "ntest.hpp"
#include "lexer/lexer.hpp"
#include "parser/ast/expression_operator.hpp"
#include "parser/expression_parser.hpp"

#include <ostream>

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

	case TokenType::ASSIGN:           return os << "TokenType::ASSIGN";
	case TokenType::ASSIGN_OPERATOR:  return os << "TokenType::ASSIGN_OPERATOR";

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

std::string test_expression_parser_basic()
{
	std::string file_name = create_test_file(
		"= 2 + 3"
	);

	Lexer lexer(file_name);

	expr::expr_p expr = parse_expr(lexer, false);
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

std::string test_expression_parser_ending_token()
{
	std::string file_name = create_test_file(
		"= 2 + 3 }"
	);

	Lexer lexer(file_name);

	expr::expr_p expr = parse_expr(lexer, false);

	night_assert_notnull(expr);
	night_assert_eq(lexer.curr().type, TokenType::CLOSE_CURLY);

	return "";
}

std::string test_expression_parser_order_of_operations()
{
	std::string file_name = create_test_file(
		"= 2 * (3 + 4 / 5)"
	);

	Lexer lexer(file_name);

	expr::expr_p expr = parse_expr(lexer, false);
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
