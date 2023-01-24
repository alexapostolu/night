#pragma once

#include "lexer.hpp"
#include "bytecode.hpp"
#include "scope.hpp"
#include "expression.hpp"
#include "type.hpp"

bytecodes_t parse_stmts(Lexer& lexer, Scope& scope);
bytecodes_t parse_stmt(Lexer& lexer, Scope& scope);

bytecodes_t parse_var(Lexer& lexer, Scope& scope);
bytecodes_t parse_if(Lexer& lexer, Scope& scope, bool is_elif);
bytecodes_t parse_else(Lexer& lexer, Scope& scope);
bytecodes_t parse_for(Lexer& lexer, Scope& scope);
bytecodes_t parse_while(Lexer& lexer, Scope& scope);
bytecodes_t parse_rtn(Lexer& lexer, Scope& scope);

expr_p parse_expr_toks(Lexer& lexer, Scope& scope, bool bracket = false);
ValueType parse_expr(expr_p const& expr, bytecodes_t& bytes);
void parse_expr_single(expr_p& head, expr_p const& val);

ExprUnaryType  str_to_unary_type(std::string_view str);
ExprBinaryType str_to_binary_type(std::string_view str);