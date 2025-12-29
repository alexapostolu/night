#pragma once

#include "lexer/lexer.hpp"
#include "ast/expression.hpp"

#include <optional>
#include <unordered_set>

/*
 * If there is no expression, it is the callers responsibility to handle, or
 * set 'err_on_empty' to true to display an error message in that event.
 * 
 * If the ending token is unspecified, it is the callers responsibility to
 * check if it is valid.
 * 
 * Lexer
 *   start: first token before expression
 *   end: last token after expression
 */
expr::expr_p parse_expr(
	Lexer& lexer,
	bool err_on_empty,
	std::optional<TokenType> const& end_token = std::nullopt
);

expr::expr_p parse_expr_s(
	Lexer& lexer,
	bool err_on_empty,
	std::optional<TokenType> const& end_token = std::nullopt
);

expr::expr_p parse_string(Lexer& lexer);

/*
 * Lexer starts at type/variable.
 */
expr::expr_p parse_variable_or_call(
	Lexer& lexer
);

expr::expr_p parse_bool(Lexer& lexer, std::optional<TokenType> const& previous_token_type);
expr::expr_p parse_char(Lexer& lexer, std::optional<TokenType> const& previous_token_type);
expr::expr_p parse_int(Lexer& lexer, std::optional<TokenType> const& previous_token_type);
expr::expr_p parse_float(Lexer& lexer, std::optional<TokenType> const& previous_token_type);
expr::expr_p parse_string(Lexer& lexer, std::optional<TokenType> const& previous_token_type);

expr::expr_p parse_typevar(Lexer& lexer, std::optional<TokenType> const& previous_token_type);

expr::expr_p parse_open_square(Lexer& lexer, std::optional<TokenType> const& previous_token_type);

expr::expr_p parse_unary_operator(Lexer& lexer, std::optional<TokenType> const& previous_token_type);
expr::expr_p parse_binary_operator(Lexer& lexer, std::optional<TokenType> const& previous_token_type);

expr::expr_p parse_open_bracket(Lexer& lexer, std::optional<TokenType> const& previous_token_type);
