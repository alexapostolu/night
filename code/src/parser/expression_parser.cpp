#include "parser/expression_parser.hpp"

#include "lexer/lexer.hpp"
#include "parser/ast/expression.hpp"
#include "parser/ast/expression_operator.hpp"
#include "parser/ast/statement.hpp"
#include "parser/statement_parser.hpp"
#include "common/token.hpp"
#include "common/debug.hpp"

#include <iostream>
#include <vector>
#include <memory>
#include <optional>
#include <stdint.h>
#include <assert.h>
#include <unordered_map>
#include <unordered_set>

/*
 * Points to the function to parse the token, and defines the next expected
 * tokens to form valid expression grammar.
 */
struct TokenParser
{
	expr::expr_p (*parser)(Lexer&, std::optional<TokenType> const&);
	std::unordered_set<TokenType> next_allowed;
};

/*
 * For each expression token, stores the next allowed tokens that will make a
 * valid expression.
 * 
 * Used by the expression parser to enforce proper grammar.
 */
std::unordered_set<TokenType> literals_next_allowed_tokens{
	TokenType::BINARY_OPERATOR,
	TokenType::CLOSE_BRACKET,
	TokenType::CLOSE_SQUARE,
	TokenType::COMMA
};

std::unordered_set<TokenType> typevar_next_allowed_tokens{
	TokenType::BINARY_OPERATOR,
	TokenType::OPEN_BRACKET,
	TokenType::CLOSE_BRACKET,
	TokenType::OPEN_SQUARE,
	TokenType::CLOSE_SQUARE
};

std::unordered_set<TokenType> open_square_next_allowed_tokens{
	TokenType::BOOL_LIT,
	TokenType::CHAR_LIT,
	TokenType::INT_LIT,
	TokenType::FLOAT_LIT,
	TokenType::STRING_LIT,
	TokenType::TYPE,
	TokenType::VARIABLE,
	TokenType::OPEN_BRACKET,
	TokenType::UNARY_OPERATOR
};

std::unordered_set<TokenType> unary_operator_next_allowed_tokens{
	TokenType::BOOL_LIT,
	TokenType::CHAR_LIT,
	TokenType::INT_LIT,
	TokenType::FLOAT_LIT,
	TokenType::STRING_LIT,
	TokenType::TYPE,
	TokenType::VARIABLE,
	TokenType::OPEN_BRACKET,
	TokenType::OPEN_SQUARE,
	TokenType::UNARY_OPERATOR
};

std::unordered_set<TokenType> binary_operator_next_allowed_tokens{
	TokenType::BOOL_LIT,
	TokenType::CHAR_LIT,
	TokenType::INT_LIT,
	TokenType::FLOAT_LIT,
	TokenType::STRING_LIT,
	TokenType::TYPE,
	TokenType::VARIABLE,
	TokenType::OPEN_BRACKET,
	TokenType::OPEN_SQUARE
};

std::unordered_set<TokenType> open_bracket_next_allowed_tokens{
	TokenType::BOOL_LIT,
	TokenType::CHAR_LIT,
	TokenType::INT_LIT,
	TokenType::FLOAT_LIT,
	TokenType::STRING_LIT,
	TokenType::TYPE,
	TokenType::VARIABLE,
	TokenType::OPEN_BRACKET,
	TokenType::OPEN_SQUARE,
	TokenType::UNARY_OPERATOR
};

std::unordered_set<TokenType> close_bracket_next_allowed_tokens{
	TokenType::BINARY_OPERATOR,
	TokenType::OPEN_SQUARE
};

std::unordered_set<TokenType> close_square_next_allowed_tokens{
	TokenType::BINARY_OPERATOR,
	TokenType::OPEN_BRACKET,
	TokenType::OPEN_SQUARE
};

std::unordered_map<TokenType, TokenParser> token_parser_map{
	{ TokenType::BOOL_LIT, { parse_bool ,literals_next_allowed_tokens } },
	{ TokenType::CHAR_LIT, { parse_char, literals_next_allowed_tokens } },
	{ TokenType::INT_LIT, { parse_int, literals_next_allowed_tokens } },
	{ TokenType::FLOAT_LIT, { parse_float, literals_next_allowed_tokens } },
	{ TokenType::STRING_LIT, { parse_string, literals_next_allowed_tokens } },

	{ TokenType::TYPE, { parse_typevar, typevar_next_allowed_tokens } },
	{ TokenType::VARIABLE, { parse_typevar, typevar_next_allowed_tokens } },

	{ TokenType::OPEN_SQUARE, { parse_open_square, open_square_next_allowed_tokens } },
	{ TokenType::UNARY_OPERATOR, { parse_unary_operator, unary_operator_next_allowed_tokens } },
	{ TokenType::BINARY_OPERATOR, { parse_binary_operator, binary_operator_next_allowed_tokens } },

	{ TokenType::OPEN_BRACKET, { parse_open_bracket, open_bracket_next_allowed_tokens } },

	{ TokenType::CLOSE_BRACKET, { nullptr, close_bracket_next_allowed_tokens } },
	{ TokenType::CLOSE_SQUARE, { nullptr, close_square_next_allowed_tokens } }
};

static Primitive token_int_to_type(Token const& token)
{
	assert(token.type == TokenType::INT_LIT);

	if (token.str == "int8") return Primitive::INT8;
	if (token.str == "int16") return Primitive::INT16;
	if (token.str == "int32") return Primitive::INT32;
	if (token.str == "int64") return Primitive::INT64;
	if (token.str == "uint8") return Primitive::uINT8;
	if (token.str == "uint16") return Primitive::uINT16;
	if (token.str == "uint32") return Primitive::uINT32;
	if (token.str == "uint64") return Primitive::uINT64;

	throw debug::unhandled_case(token.str);
}

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
		
		Token const& curr = lexer.eat();
		
		if (!token_parser_map.contains(curr.type) || curr.type == TokenType::CLOSE_BRACKET || curr.type == TokenType::CLOSE_SQUARE)
		{
			if (!end_token.has_value() || curr.type == end_token)
				return head;

			if (err_on_empty || head)
				throw night::error::get().create_fatal_error(
					"Found invalid token '" + lexer.curr().str + "' in expression", lexer.loc);
		}

		if (previous_token_type.has_value() &&
			!token_parser_map.at(*previous_token_type).next_allowed.contains(curr.type))
		{
			throw night::error::get().create_fatal_error(
				"Unexpected token '" + lexer.curr().str + "' after token '" + night::to_str(*previous_token_type) + "'", lexer.loc);
		}

		new_node = token_parser_map.at(curr.type).parser(lexer, previous_token_type);
		assert(new_node);

		if (!head)
			head = new_node;
		else
			head->insert_node(new_node, &head);

		previous_token_type = lexer.curr().type;
	}
}

expr::expr_p parse_bool(Lexer& lexer, std::optional<TokenType> const& previous_token_type)
{
	return std::make_shared<expr::Numeric>(lexer.loc, Primitive::BOOL, (int64_t)(lexer.curr().str == "true"));
}

expr::expr_p parse_char(Lexer& lexer, std::optional<TokenType> const& previous_token_type)
{
	assert(lexer.curr().str.length() == 1);
	return std::make_shared<expr::Numeric>(lexer.loc, Primitive::CHAR, (int64_t)lexer.curr().str[0]);
}

expr::expr_p parse_int(Lexer& lexer, std::optional<TokenType> const& previous_token_type)
{
	return std::make_shared<expr::Numeric>(lexer.loc, Primitive::INT, std::stoll(lexer.curr().str));
}

expr::expr_p parse_float(Lexer& lexer, std::optional<TokenType> const& previous_token_type)
{
	return std::make_shared<expr::Numeric>(lexer.loc, Primitive::FLOAT, std::stod(lexer.curr().str));
}

expr::expr_p parse_string(Lexer& lexer, std::optional<TokenType> const& previous_token_type)
{
	std::vector<expr::expr_p> str;

	for (char c : lexer.curr().str)
		str.push_back(std::make_shared<expr::Numeric>(lexer.loc, Primitive::CHAR, (int64_t)c));

	return std::make_shared<expr::Array>(lexer.loc, str, true);
}

expr::expr_p parse_typevar(Lexer& lexer, std::optional<TokenType> const& previous_token_type)
{
	Token variable = lexer.curr();

	// Parse function call.
	if (lexer.peek().type == TokenType::OPEN_BRACKET)
	{
		lexer.eat();
		return std::make_shared<expr::FunctionCall>(parse_func_call(lexer, variable));
	}

	// Parse array allocation.
	if (lexer.peek().type == TokenType::OPEN_SQUARE && variable.type == TokenType::TYPE)
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

		return std::make_shared<expr::Allocate>(lexer.loc, Type(variable.str).get_prim(), sizes);
	}

	// Parse variable.

	return std::make_shared<expr::Variable>(lexer.loc, variable.str);
}

expr::expr_p parse_open_square(Lexer& lexer, std::optional<TokenType> const& previous_token_type)
{
	// Parse subscript.
	if (previous_token_type == TokenType::STRING_LIT ||
		previous_token_type == TokenType::VARIABLE ||
		previous_token_type == TokenType::CLOSE_SQUARE)
	{
		auto index_expr = parse_expr(lexer, true, TokenType::CLOSE_SQUARE);
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

expr::expr_p parse_unary_operator(Lexer& lexer, std::optional<TokenType> const& previous_token_type)
{
	 return std::make_shared<expr::UnaryOp>(lexer.loc, lexer.curr().str);
}

expr::expr_p parse_binary_operator(Lexer& lexer, std::optional<TokenType> const& previous_token_type)
{
	if (lexer.curr().str == "-" &&
		(!previous_token_type.has_value() ||
			previous_token_type == TokenType::UNARY_OPERATOR ||
			previous_token_type == TokenType::BINARY_OPERATOR))
		return std::make_shared<expr::UnaryOp>(lexer.loc, lexer.curr().str);

	return std::make_shared<expr::BinaryOp>(lexer.loc, lexer.curr().str); 
}

expr::expr_p parse_open_bracket(Lexer& lexer, std::optional<TokenType> const& previous_token_type)
{
	auto node = parse_expr(lexer, true, TokenType::CLOSE_BRACKET);
	node->set_guard();

	lexer.curr_is(TokenType::CLOSE_BRACKET);

	return node;
}
