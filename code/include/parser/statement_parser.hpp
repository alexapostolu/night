#pragma once

#include "lexer.hpp"
#include "ast/statement.hpp"
#include "ast/expression.hpp"

#include <vector>
#include <string>
#include <tuple>

/* The starting function for parsing a file.
 */
std::vector<stmt_p> parse_file(std::string const& main_file);

/* Parses a body of statements.
 * Lexer
 *   start: first token of statements
 *   end: first token of next statement
 * 
 * @param contains_return_stmt will be set to true if the statement block
 *   contains a return statement. This is for non-void functions to check if
 *   their body contains a returns statement.
 */
std::vector<stmt_p> parse_stmts(Lexer& lexer, bool requires_curly, bool* contains_return_stmt = nullptr);

/* Parses a singular statement. Note that a singular statement could also be
 * the only statement in a scope.
 * Lexer
 *   start: first token of statement
 *   end: first token of next statement
 */
stmt_p parse_stmt(Lexer& lexer, bool* is_return_stmt = nullptr);

/* This function handles three cases:
 *   variable initialization
 *   variable assignment
 *   function calls
 * Lexer:
 *   start: variable name
 *   end: first token of next statement
 */
stmt_p parse_var(Lexer& lexer);

/* Lexer:
 *   start: variable type
 *   end: semicolon
 * Examples:
 *   my_var int;
 *   my_var int = [expression];
 *   my_var int[2] = [array];
 */
VariableInit parse_var_init(Lexer& lexer, std::string const& var_name);

ArrayInitialization parse_array_init(Lexer& lexer, std::string const& var_name);

/* It is the callers responsibility to check if lexer.curr() is their expected
 * token after this function is called.
 * Lexer:
 *   start: assignment operator
 *   end: first token of next statement
 * Examples:
 *   my_var = [expression];
 *   for (;; my_var += 1) {}
 */
VariableAssign parse_var_assign(Lexer& lexer, std::string const& var_name);

/* Lexer:
 *   start: variable name
 *   end: first token of next statement
 */
ArrayMethod parse_array_method(Lexer& lexer, std::string const& var_name);

/* It is the callers responsibility to check if lexer.curr() is their expected
 * token after this function is called.
 * Lexer:
 *   start: open bracket
 *   end: closing bracket
 * Examples:
 *   print(" ");
 *	 my_var = func(2) * 3;
 */
expr::FunctionCall parse_func_call(Lexer& lexer, std::string const& func_name);

/* Parses the entire conditional chain (every 'else if' and 'else' that follows)
 * Lexer:
 *   start: if token
 *   end: first token of next statement
 */
Conditional parse_if(Lexer& lexer);

/* Lexer:
 *   start: while token
 *   end: first token of next statement
 */
While parse_while(Lexer& lexer);

/* Lexer:
 *   start: while token
 *   end: first token of next statement
 */
For parse_for(Lexer& lexer);

/* Lexer:
 *   start: def token
 *   end: first token of next statement
 */
Function parse_func(Lexer& lexer);

/* Lexer:
 *   start: return token
 *   end: first token of next statement
 */
Return parse_return(Lexer& lexer);

/* Useful for function parameter types and return values as those can include
 * subscripts
 * Lexer:
 *   start: name of parameter
 *   end: first token after type
 * @returns tuple of parameter type and dimension
 * @returns empty tuple if lexing error
 */
std::tuple<std::string, int> parse_type(Lexer& lexer);