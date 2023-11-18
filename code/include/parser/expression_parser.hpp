#pragma once

#include "lexer.hpp"
#include "ast/expression.hpp"

#include <optional>

/* If there is no expression, it is the callers responsibility to handle, or
 * set 'err_on_empty' to true to display an error message in that event
 * Lexer
 *   start: first token of expression
 *   end: last token after expression
 */
expr::expr_p parse_expr(
	Lexer& lexer,
	bool err_on_empty,
	std::optional<TokenType> const& end_token = std::nullopt
);

expr::expr_p parse_string(Lexer& lexer);
expr::expr_p parse_variable_or_call(Lexer& lexer);
expr::expr_p parse_subscript_or_array(Lexer& lexer, std::optional<TokenType> previous_token_type);
expr::expr_p parse_subtract_or_negative(Lexer& lexer, std::optional<TokenType> previous_token_type);
expr::expr_p parse_bracket(Lexer& lexer, bool err_on_empty);