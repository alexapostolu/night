#include "parser/expression_parser.hpp"

#include "lexer/lexer.hpp"
#include "parser/ast/expression.hpp"
#include "parser/ast/expression_operator.hpp"
#include "parser/ast/statement.hpp"
#include "parser/statement_parser.hpp"
#include "common/token.hpp"

#include <vector>
#include <memory>
#include <optional>
#include <stdint.h>
#include <assert.h>

expr::expr_p parse_expr(Lexer& lexer, bool err_on_empty, std::optional<TokenType> const& end_token)
{
	// The root node of the expression.
	expr::expr_p head;

	// The previous type allows us to differentiate the subscript operator from
	// an array (used in case OPEN_SQUARE), and the negative operator from the
	// subtract operator (used in case BINARY_OP).
	std::optional<TokenType> previous_token_type;

	expr::expr_p new_node;

	while (true)
	{
		// The new node to be constructed.
		expr::expr_p new_node;
		
		switch (lexer.eat().type)
		{
		case TokenType::BOOL_LIT:
			parse_check_value(previous_token_type, lexer);
			new_node = std::make_shared<expr::Numeric>(lexer.loc, Type::BOOL, (int64_t)(lexer.curr().str == "true"));
			break;

		case TokenType::CHAR_LIT:
			parse_check_value(previous_token_type, lexer);
			assert(lexer.curr().str.length() == 1);
			new_node = std::make_shared<expr::Numeric>(lexer.loc, Type::CHAR, (int64_t)lexer.curr().str[0]);
			break;

		case TokenType::INT_LIT:
			parse_check_value(previous_token_type, lexer);
			new_node = std::make_shared<expr::Numeric>(lexer.loc, Type::INT, std::stoll(lexer.curr().str));
			break;

		case TokenType::FLOAT_LIT:
			parse_check_value(previous_token_type, lexer);
			new_node = std::make_shared<expr::Numeric>(lexer.loc, Type::FLOAT, std::stod(lexer.curr().str));
			break;

		case TokenType::STRING_LIT:
			parse_check_value(previous_token_type, lexer);
			new_node = parse_string(lexer);
			break;

		case TokenType::TYPE:
		case TokenType::VARIABLE:
			parse_check_value(previous_token_type, lexer);
			new_node = parse_variable_or_call(lexer);
			break;

		case TokenType::OPEN_SQUARE:
			parse_check_open_square(previous_token_type, lexer);
			new_node = parse_subscript_or_array(lexer, previous_token_type);
			break;

		case TokenType::UNARY_OPERATOR:
			parse_check_unary_operator(previous_token_type, lexer);
			new_node = std::make_shared<expr::UnaryOp>(lexer.loc, lexer.curr().str);
			break;

		case TokenType::BINARY_OPERATOR:
			parse_check_binary_operator(previous_token_type, lexer);
			new_node = parse_subtract_or_negative(lexer, previous_token_type);
			break;

		case TokenType::OPEN_BRACKET:
			parse_check_open_bracket(previous_token_type, lexer);
			new_node = parse_bracket(lexer, err_on_empty);
			previous_token_type = TokenType::CLOSE_BRACKET;
			break;

		default:
		{
			if (err_on_empty && !head)
				throw night::error::get().create_fatal_error("Found '" + lexer.curr().str + "', expected expression", lexer.loc);

			if (end_token.has_value())
				lexer.curr_is(*end_token);

			parse_check_expression_ending(previous_token_type, lexer);

			return head;
		}
		}
		
		assert(new_node);

		if (!head)
			head = new_node;
		else
			head->insert_node(new_node, &head);

		previous_token_type = lexer.curr().type;
	}
}

expr::expr_p parse_string(Lexer& lexer)
{
	std::vector<expr::expr_p> str;

	for (char c : lexer.curr().str)
		str.push_back(std::make_shared<expr::Numeric>(lexer.loc, Type::CHAR, (int64_t)c));

	return std::make_shared<expr::Array>(lexer.loc, str, true);
}

expr::expr_p parse_variable_or_call(Lexer& lexer)
{
	auto var_name = lexer.curr().str;
	Token var = lexer.curr();

	// Parse function call.
	if (lexer.peek().type == TokenType::OPEN_BRACKET)
	{
		lexer.eat();
		return std::make_shared<expr::FunctionCall>(parse_func_call(lexer, var));
	}
	
	// Parse array allocation.
	if (lexer.peek().type == TokenType::OPEN_SQUARE &&
		(var_name == "int" || var_name == "float"))
	{
		lexer.eat();

		std::vector<expr::expr_p> sizes;
		while (true)
		{
			sizes.push_back(parse_expr(lexer, true, TokenType::CLOSE_SQUARE));

			if (lexer.peek().type == TokenType::OPEN_SQUARE)
				lexer.eat();
			else
				break;
		}
		
		return std::make_shared<expr::Allocate>(lexer.loc, Type(var_name).prim, sizes);
	}
	
	// Parse variable.

	return std::make_shared<expr::Variable>(lexer.loc, var_name);
}

expr::expr_p parse_subscript_or_array(Lexer& lexer, std::optional<TokenType> previous_token_type)
{
	// Parse subscript.
	if (previous_token_type == TokenType::STRING_LIT ||
		previous_token_type == TokenType::VARIABLE ||
		previous_token_type == TokenType::CLOSE_SQUARE)
	{
		auto index_expr = parse_expr(lexer, true);
		lexer.curr_is(TokenType::CLOSE_SQUARE);

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
			lexer.curr_is(TokenType::CLOSE_SQUARE);
			break;
		}

		arr.push_back(elem);

		if (lexer.curr().type == TokenType::CLOSE_SQUARE)
			break;

		lexer.curr_is(TokenType::COMMA);
	}

	return std::make_shared<expr::Array>(lexer.loc, arr, false);
}

expr::expr_p parse_subtract_or_negative(Lexer& lexer, std::optional<TokenType> previous_token_type)
{
	if (lexer.curr().str == "-" &&
		(!previous_token_type.has_value() ||
			previous_token_type == TokenType::UNARY_OPERATOR ||
			previous_token_type == TokenType::BINARY_OPERATOR))
		return std::make_shared<expr::UnaryOp>(lexer.loc, lexer.curr().str);

	return std::make_shared<expr::BinaryOp>(lexer.loc, lexer.curr().str);
}

expr::expr_p parse_bracket(Lexer& lexer, bool err_on_empty)
{
	auto node = parse_expr(lexer, err_on_empty);
	node->set_guard();

	lexer.curr_is(TokenType::CLOSE_BRACKET);
	
	return node;
}

void parse_check_value(std::optional<TokenType> const& previous_type, Lexer const& lexer)
{
	if (previous_type.has_value() &&
		previous_type != TokenType::UNARY_OPERATOR &&
		previous_type != TokenType::BINARY_OPERATOR &&
		previous_type != TokenType::OPEN_SQUARE &&
		previous_type != TokenType::OPEN_BRACKET)
		throw night::error::get().create_fatal_error("Expected operator before value " + lexer.curr().str + ".", lexer.loc);
}

void parse_check_unary_operator(std::optional<TokenType> const& previous_type, Lexer const& lexer)
{
	if (previous_type.has_value() &&
		previous_type != TokenType::UNARY_OPERATOR &&
		previous_type != TokenType::BINARY_OPERATOR &&
		previous_type != TokenType::OPEN_SQUARE &&
		previous_type != TokenType::OPEN_BRACKET)
		throw night::error::get().create_fatal_error("Expected operator before unary operator " + lexer.curr().str + ".", lexer.loc);
}

void parse_check_binary_operator(std::optional<TokenType> const& previous_type, Lexer const& lexer)
{
	if (previous_type.has_value() && previous_type == TokenType::BINARY_OPERATOR && lexer.curr().str == "-")
		return;

	if (previous_type.has_value() &&
		previous_type != TokenType::BOOL_LIT &&
		previous_type != TokenType::CHAR_LIT &&
		previous_type != TokenType::INT_LIT &&
		previous_type != TokenType::FLOAT_LIT &&
		previous_type != TokenType::STRING_LIT &&
		previous_type != TokenType::VARIABLE &&
		previous_type != TokenType::CLOSE_BRACKET &&
		previous_type != TokenType::CLOSE_SQUARE)
		throw night::error::get().create_fatal_error("Expected value or expression before binary operator " + lexer.curr().str + ".", lexer.loc);
}

void parse_check_open_square(std::optional<TokenType> const& previous_type, Lexer const& lexer)
{
	if (previous_type.has_value() &&
		previous_type != TokenType::BINARY_OPERATOR &&
		previous_type != TokenType::OPEN_BRACKET &&
		previous_type != TokenType::CLOSE_SQUARE &&
		previous_type != TokenType::BOOL_LIT &&
		previous_type != TokenType::CHAR_LIT &&
		previous_type != TokenType::INT_LIT &&
		previous_type != TokenType::FLOAT_LIT &&
		previous_type != TokenType::STRING_LIT &&
		previous_type != TokenType::VARIABLE)
		throw night::error::get().create_fatal_error("Expected binary operator, open bracket or closing square before open square.", lexer.loc);
}

void parse_check_open_bracket(std::optional<TokenType> const& previous_type, Lexer const& lexer)
{
	if (previous_type.has_value() &&
		previous_type != TokenType::UNARY_OPERATOR &&
		previous_type != TokenType::BINARY_OPERATOR)
		throw night::error::get().create_fatal_error("Expected operator before open bracket.", lexer.loc);
}

void parse_check_expression_ending(std::optional<TokenType> const& previous_type, Lexer const& lexer)
{
	if (previous_type.has_value() &&
		previous_type != TokenType::CLOSE_BRACKET &&
		previous_type != TokenType::CLOSE_SQUARE &&
		previous_type != TokenType::BOOL_LIT &&
		previous_type != TokenType::CHAR_LIT &&
		previous_type != TokenType::INT_LIT &&
		previous_type != TokenType::FLOAT_LIT &&
		previous_type != TokenType::STRING_LIT &&
		previous_type != TokenType::VARIABLE)
		throw night::error::get().create_fatal_error("Expected closing bracket, literal or variable at the end of the expression.", lexer.loc);
}
