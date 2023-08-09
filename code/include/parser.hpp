#pragma once

#include "lexer.hpp"
#include "bytecode.hpp"
#include "parser_scope.hpp"
#include "interpreter.hpp"
#include "ast/ast.hpp"
#include "ast/expression.hpp"
#include "value_type.hpp"

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
	bool* curly_enclosed = nullptr);

// lexer
//   start: first token of statement
//   end:   last token of statement
std::shared_ptr<AST> parse_stmt(
	Lexer& lexer);

// three cases:
//   variable init
//   variable assign
//   function call
std::shared_ptr<AST> parse_var(Lexer& lexer);

// lexer
//   start: variable type
//   end: semicolon
// examples:
//   my_var int;
//   my_var int = [expression];
VariableInit parse_var_init(Lexer& lexer, std::string const& var_name);

// caller's responsibility to check current token of lexer
// lexer
//   start: assignment
//   end:   first token after expression
// examples:
//   my_var = [expression];
//   my_var += [expression];
VariableAssign parse_var_assign(Lexer& lexer, std::string const& var_name);

FunctionCall parse_func_call(Lexer& lexer, std::string const& func_name);

Conditional parse_if(Lexer& lexer);

While parse_while(Lexer& lexer);

For parse_for(Lexer& lexer);

Function parse_func(Lexer& lexer);

Return parse_return(Lexer& lexer);

// if expr is null, it is the callers responsibility to handle,
// or set err_on_empty to true to display an error message in that event
// lexer
//   start: token before expression
//   end: first token after expression
// turns tokens into AST
// 'bracket' is for recursive call only
std::shared_ptr<expr::Expression> parse_expr(Lexer& lexer, bool err_on_empty);