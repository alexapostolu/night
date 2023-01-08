#pragma once

#include "lexer.hpp"
#include "bytecode.hpp"
#include "scope.hpp"
#include "expression.hpp"

bytecodes_t parse_stmts(Lexer& lexer, Scope& scope);

bytecodes_t parse_var(Lexer& lexer, Scope& scope);
bytecodes_t parse_if(Lexer& lexer, Scope& scope);
bytecodes_t parse_for(Lexer& lexer, Scope& scope);
bytecodes_t parse_while(Lexer& lexer, Scope& scope);
bytecodes_t parse_rtn(Lexer& lexer, Scope& scope);

expr_p parse_expr(Lexer& lexer, Scope& scope);

ExprUnaryType  str_to_unary_type(std::string_view str);
ExprBinaryType str_to_binary_type(std::string_view str);