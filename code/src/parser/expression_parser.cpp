#include "parser/expression_parser.hpp"

#include "lexer.hpp"
#include "ast/expression.hpp"
#include "ast/statement.hpp"
#include "parser/statement_parser.hpp"
#include "token.hpp"

#include <vector>
#include <memory>
#include <optional>
#include <stdint.h>
#include <assert.h>

expr::expr_p parse_expr(Lexer& lexer, bool err_on_empty, std::optional<TokenType> const& end_token)
{
	// The root node of the expression.
	expr::expr_p head(nullptr);

	// The previous type allows us to differentiate the subscript operator from
	// an array (used in case OPEN_SQUARE), and the negative operator from the
	// subtract operator (used in case BINARY_OP).
	std::optional<TokenType> previous_token_type;

	while (true)
	{
		// The new node to be constructed.
		expr::expr_p node;

		switch (lexer.eat().type)
		{
		case TokenType::BOOL_LIT:
			node = std::make_shared<expr::Numeric>(lexer.loc, ValueType::BOOL, (int64_t)(lexer.curr().str == "true"));
			break;

		case TokenType::CHAR_LIT:
			assert(lexer.curr().str.length() == 1);
			node = std::make_shared<expr::Numeric>(lexer.loc, ValueType::CHAR, (int64_t)lexer.curr().str[0]);
			break;

		case TokenType::INT_LIT:
			node = std::make_shared<expr::Numeric>(lexer.loc, ValueType::INT, std::stoull(lexer.curr().str));
			break;

		case TokenType::FLOAT_LIT:
			node = std::make_shared<expr::Numeric>(lexer.loc, ValueType::FLOAT, std::stod(lexer.curr().str));
			break;

		case TokenType::STRING_LIT:
			node = parse_string(lexer);
			break;

		case TokenType::TYPE:
		case TokenType::VARIABLE:
			node = parse_variable_or_call(lexer);
			break;

		case TokenType::OPEN_SQUARE:
			node = parse_subscript_or_array(lexer, previous_token_type);
			break;

		case TokenType::UNARY_OP:
			node = std::make_shared<expr::UnaryOp>(lexer.loc, lexer.curr().str);
			break;

		case TokenType::BINARY_OP:
			node = parse_subtract_or_negative(lexer, previous_token_type);
			break;

		case TokenType::OPEN_BRACKET:
			node = parse_bracket(lexer, err_on_empty);
			break;

		default:
		{
			if (err_on_empty && !head)
				throw night::create_fatal_error("found '" + lexer.curr().str + "', expected expression", lexer.loc);

			if (end_token.has_value())
				lexer.curr_check(*end_token);

			return head;
		}
		}

		assert(node);

		if (!head)
			head = node;
		else
			head->insert_node(node, &head);

		previous_token_type = lexer.curr().type;
	}
}

expr::expr_p parse_string(Lexer& lexer)
{
	std::vector<expr::expr_p> str;

	for (char c : lexer.curr().str)
		str.push_back(std::make_shared<expr::Numeric>(lexer.loc, ValueType::CHAR, (int64_t)c));

	return std::make_shared<expr::Array>(lexer.loc, str, true);
}

expr::expr_p parse_variable_or_call(Lexer& lexer)
{
	auto var_name = lexer.curr().str;

	// Parse function call.
	if (lexer.peek().type == TokenType::OPEN_BRACKET)
	{
		lexer.eat();
		return std::make_shared<expr::FunctionCall>(parse_func_call(lexer, var_name));
	}
	
	// Parse array allocation.
	if (lexer.peek().type == TokenType::OPEN_SQUARE)
	{
		lexer.eat();

		std::vector<expr::expr_p> sizes;
		while (true)
		{
			sizes.push_back(parse_expr(lexer, true, TokenType::CLOSE_SQUARE));

			lexer.eat();

			if (lexer.eat().type != TokenType::OPEN_SQUARE)
				break;
		}

		return std::make_shared<expr::Allocate>(lexer.loc, var_name, sizes)
	}
	
	// Parse variable.

	return std::make_shared<expr::Variable>(lexer.loc, var_name);
}

expr::expr_p parse_subscript_or_array(Lexer& lexer, std::optional<TokenType> previous_token_type)
{
	// Parse subscript.
	if (previous_token_type == TokenType::STRING_LIT ||
		previous_token_type == TokenType::VARIABLE)
	{
		auto index_expr = parse_expr(lexer, true);
		lexer.curr_check(TokenType::CLOSE_SQUARE);

		auto node = std::make_shared<expr::BinaryOp>(lexer.loc, "[");
		node->insert_node(index_expr);

		return node;
	}
	
	// Parse array.

	std::vector<expr::expr_p> arr;
	while (true)
	{
		auto elem = parse_expr(lexer, false);

		if (!elem)
		{
			lexer.curr_check(TokenType::CLOSE_SQUARE);
			break;
		}

		arr.push_back(elem);

		if (lexer.curr().type == TokenType::CLOSE_SQUARE)
			break;

		lexer.curr_check(TokenType::COMMA);
	}

	return std::make_shared<expr::Array>(lexer.loc, arr, false);
}

expr::expr_p parse_subtract_or_negative(Lexer& lexer, std::optional<TokenType> previous_token_type)
{
	if (lexer.curr().str == "-" &&
		(!previous_token_type.has_value() ||
			previous_token_type == TokenType::UNARY_OP ||
			previous_token_type == TokenType::BINARY_OP))
		return std::make_shared<expr::UnaryOp>(lexer.loc, lexer.curr().str);

	return std::make_shared<expr::BinaryOp>(lexer.loc, lexer.curr().str);
}

expr::expr_p parse_bracket(Lexer& lexer, bool err_on_empty)
{
	auto node = parse_expr(lexer, err_on_empty);
	node->set_guard();

	lexer.curr_check(TokenType::CLOSE_BRACKET);
	
	return node;
}