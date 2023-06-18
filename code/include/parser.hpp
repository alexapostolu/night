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
void generate_codes_var(bytecodes_t& codes, Lexer& lexer, Scope& scope);
void generate_codes_if(bytecodes_t& codes, Lexer& lexer, Scope& scope, bool is_elif);
void generate_codes_else(bytecodes_t& codes, Lexer& lexer, Scope& scope);
void generate_codes_for(bytecodes_t& codes, Lexer& lexer, Scope& scope);
void generate_codes_while(bytecodes_t& codes, Lexer& lexer, Scope& scope);
bytecodes_t parse_func(Lexer& lexer, Scope& scope);
bytecodes_t parse_rtn(Lexer& lexer, Scope& scope);

BytecodeType token_var_type_to_bytecode(std::string const& type);

void number_to_bytecode(std::string const& s_num, bytecodes_t& codes);
void number_to_bytecode(int num, bytecodes_t& codes);

// lexer
//   start: variable type
//   end: semicolon
// examples:
//   my_var int;
//   my_var int = [expression];
void generate_codes_var_init(bytecodes_t& codes, Lexer& lexer, Scope& scope,
	std::string const& var_name);

// lexer
//   start: assignment
//   end: if require_semicolon is true, semicolon, else, first token after expression
// examples:
//   my_var = [expression];
//   my_var += [expression];
void generate_codes_var_assign(bytecodes_t& codes, Lexer& lexer, Scope& scope,
	std::string const& var_name, bool require_semicolon);

// token starts at open bracket
// ends at close bracket
void parse_comma_sep_stmts(Lexer& lexer, Scope& scope, bytecodes_t& codes);

// lexer
//   start: first token of expression
//   end: first token after expression
// turns tokens into AST
// 'bracket' is for recursive call only
// if return value is null, expression is empty
// caller's responsibility to handle null return values
expr_p parse_toks_expr(Lexer& lexer, Scope& scope, std::string const& err_msg = "",
	bool bracket = false);

// generates bytecode from the expression pointer
void generate_codes_expr(bytecodes_t& codes, expr_p const& expr);
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