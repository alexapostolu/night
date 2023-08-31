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

AST_Block parse_file(std::string const& main_file);

// lexer
//   start: first token of statements
//   end:   first token of next statements (the reason is parse_if() has to know the next token for if-elif-else chain)
// curly_enclosed
//   if true, throws an error if statements are not enclosed in curly brackets
//     functions are the only statement type that require curly brackets
AST_Block parse_stmts(
	Lexer& lexer,
	bool requires_curly);

// lexer
//   start: first token of statement
//   end:   first token of next statement
std::shared_ptr<AST> parse_stmt(
	Lexer& lexer);

// three cases:
//   variable init
//   variable assign
//   function call
std::shared_ptr<AST> parse_var(Lexer& lexer);

// lexer:
//   start: variable type
//   end: semicolon
// examples:
//   my_var int;
//   my_var int = [expression];
VariableInit parse_var_init(Lexer& lexer, std::string const& var_name);

// lexer:
//   start: assignment operator
//   end: first token of next statement
// examples:
//   my_var = [expression];
//   for (;; my_var += 1) {}
VariableAssign parse_var_assign(Lexer& lexer, std::string const& var_name);

ArrayMethod parse_array_method(Lexer& lexer, std::string const& var_name);

// lexer
//   start: open brakcet
//   end: closing bracket
expr::FunctionCall parse_func_call(Lexer& lexer, std::string const& func_name);

Conditional parse_if(Lexer& lexer);
While parse_while(Lexer& lexer);
For parse_for(Lexer& lexer);
Function parse_func(Lexer& lexer);
Return parse_return(Lexer& lexer);

// if expr is null, it is the callers responsibility to handle,
// or set err_on_empty to true to display an error message in that event
// lexer
//   start: first token of expression
//   end: first token after expression
// turns tokens into AST
expr::expr_p parse_expr(Lexer& lexer, bool err_on_empty);