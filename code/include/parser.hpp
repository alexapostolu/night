#pragma once

#include "lexer.hpp"
#include "bytecode.hpp"
#include "scope.hpp"
#include "expression.hpp"
#include "value.hpp"

#include <string>

// lexer
//   start: one token before statements
//   end:   last token of statements
// curly_enclosed
//   modifies to true if the statements are enclosed with curly braces
//   this is useful to know as a function with statements not enclosed by curly
//     braces will throw a syntax error
bytecodes_t parse_stmts(Lexer& lexer, Scope& scope, bool* curly_enclosed = nullptr);

// lexer
//   start: first token of statement
//   end:   last token of statement
bytecodes_t parse_stmt(Lexer& lexer, Scope& scope);
void parse_var(bytecodes_t& codes, Lexer& lexer, Scope& scope);
bytecodes_t parse_if(Lexer& lexer, Scope& scope, bool is_elif);
bytecodes_t parse_else(Lexer& lexer, Scope& scope);
bytecodes_t parse_for(Lexer& lexer, Scope& scope);
bytecodes_t parse_while(Lexer& lexer, Scope& scope);
bytecodes_t parse_func(Lexer& lexer, Scope& scope);
bytecodes_t parse_rtn(Lexer& lexer, Scope& scope);

BytecodeType token_var_type_to_bytecode(std::string const& type);

void number_to_bytecode(std::string const& s_num, bytecodes_t& codes);

// index of element in map
int find_var_index(var_container const& vars, std::string const& var_name);

// token starts at assign, ends at last statement of assignment
// caller's responsibility to check curr token after function call finishes
void parse_var_assign(Lexer& lexer, Scope& scope, bytecodes_t& codes, std::string const& var_name);

// token starts at open bracket
// ends at close bracket
void parse_comma_sep_stmts(Lexer& lexer, Scope& scope, bytecodes_t& codes);

// turns tokens into AST
// 'bracket' is for recursive call only
// if return value is null, you have problem
// lexer.curr starts token before expr, ends at token after expression
expr_p parse_toks_expr(Lexer& lexer, Scope& scope, bool bracket = false);
ValueType expr_type_check(expr_p const& expr);
// generates bytecode from the expression pointer
// deduces and returns the type of the expression
ValueType parse_expr(expr_p const& expr, bytecodes_t& bytes);
void parse_expr_single(expr_p& head, expr_p const& val);

ExprUnaryType  str_to_unary_type(std::string const& str);
ExprBinaryType str_to_binary_type(std::string const& str);

// type check functions should generally throw minor errors
namespace type_check {

// var already defined
void var_defined(Lexer const& lexer, Scope const& scope, std::string const& var_name);

// var undefined
void var_undefined(Lexer const& lexer, Scope const& scope, std::string const& var_name);

// var type compatable with assignment type
void var_assign_type(Lexer const& lexer, Scope& scope, std::string const& var_name, BytecodeType assign_type);

// var type compatable with expr type
void var_expr_type(Lexer const& lexer, Scope& scope, std::string const& var_name, ValueType expr_type);

}