#pragma once

#include "lexer.hpp"
#include "bytecode.hpp"
#include "scope.hpp"
#include "expression.hpp"
#include "value.hpp"

#include <string>

bytecodes_t parse_stmts(Lexer& lexer, Scope& scope);
bytecodes_t parse_stmt(Lexer& lexer, Scope& scope);

bytecodes_t parse_var(Lexer& lexer, Scope& scope);
bytecodes_t parse_if(Lexer& lexer, Scope& scope, bool is_elif);
bytecodes_t parse_else(Lexer& lexer, Scope& scope);
bytecodes_t parse_for(Lexer& lexer, Scope& scope);
bytecodes_t parse_while(Lexer& lexer, Scope& scope);
bytecodes_t parse_rtn(Lexer& lexer, Scope& scope);

// token starts at assign, ends at last statement of assignment
// caller's responsibility to check curr token after function call finishes
void parse_var_assign(Lexer& lexer, Scope& scope, bytecodes_t& codes, std::string const& var_name);

// turns tokens into AST
// 'bracket' is for internal use only
// if return value is null, you have problem
// lexer.curr starts token before expr, ends at token after expression
expr_p parse_expr_toks(Lexer& lexer, Scope& scope, bool bracket = false);
ValueType parse_expr(expr_p const& expr, bytecodes_t& bytes);
void parse_expr_single(expr_p& head, expr_p const& val);

ExprUnaryType  str_to_unary_type(std::string const& str);
ExprBinaryType str_to_binary_type(std::string const& str);

// type check functions should generally throw minor errors
namespace type_check {

// var already defined
void var_defined(Scope const& scope, std::string const& var_name);

// var undefined
void var_undefined(Scope const& scope, std::string const& var_name);

// var type compatable with assignment type
void var_assign_type(Scope& scope, std::string const& var_name, AssignType assign_type);

// var type compatable with expr type
void var_expr_type(Scope& scope, std::string const& var_name, ValueType expr_type);

}