#pragma once

#include "lexer.hpp"
#include "bytecode.hpp"
#include "parser_scope.hpp"
#include "interpreter.hpp"
#include "ast/ast.hpp"
#include "ast/expression.hpp"
#include "value.hpp"

#include <memory>
#include <string>

// lexer
//   start: one token before statements
//   end:   last token of statements
// curly_enclosed
//   modifies to true if the statements are enclosed with curly braces
//   this is useful to know as a function with statements not enclosed by curly
//     braces will throw a syntax error
AST_Block parse_stmts(
	Lexer& lexer,
	ParserScope& scope,
	bool* curly_enclosed = nullptr);

// lexer
//   start: first token of statement
//   end:   last token of statement
std::shared_ptr<AST> parse_stmt(
	Lexer& lexer,
	ParserScope& scope);

// three cases:
//   variable init
//   variable assign
//   function call
std::shared_ptr<AST> parse_var(Lexer& lexer, ParserScope& scope);

// lexer
//   start: variable type
//   end: semicolon
// examples:
//   my_var int;
//   my_var int = [expression];
VariableInit parse_var_init(Lexer& lexer, ParserScope& scope, std::string const& var_name);

// caller's responsibility to check current token of lexer
// lexer
//   start: assignment
//   end:   first token after expression
// examples:
//   my_var = [expression];
//   my_var += [expression];
VariableAssign parse_var_assign(Lexer& lexer, ParserScope& scope, std::string const& var_name);

FunctionCall parse_func_call(Lexer& lexer, ParserScope& scope, std::string const& func_name);

Conditional parse_if(Lexer& lexer, ParserScope& scope);

While parse_while(Lexer& lexer, ParserScope& scope);

For parse_for(Lexer& lexer, ParserScope& scope);

Function parse_func(Lexer& lexer, ParserScope& scope);

Return parse_return(Lexer& lexer, ParserScope& scope);

// callers responsibility to handle null return value,
// or give optional parameter to display error messages
// lexer
//   start: first token of expression
//   end: first token after expression
// turns tokens into AST
// 'bracket' is for recursive call only
std::shared_ptr<expr::Expression> parse_expr(Lexer& lexer, ParserScope const& scope,
	std::string const& err_msg_empty = "",
	bool bracket = false);

val::value_t token_var_type_to_val_type(std::string const& type);