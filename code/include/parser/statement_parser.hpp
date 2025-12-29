#pragma once

#include "lexer/lexer.hpp"
#include "ast/statement.hpp"
#include "ast/expression.hpp"

#include <vector>
#include <string>
#include <tuple>

/*
 * Used to reserve space in vector<expr_p>'s used to store dimensions or
 * subscripts.
 * 
 * The average array dimension is 3.
 */
constexpr short AVG_ARRAY_DIMENSION = 3;

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
std::vector<stmt_p> parse_stmts(Lexer& lexer, bool requires_curly, bool* contains_return = nullptr);

/* Parses a singular statement. Note that a singular statement could also be
 * the only statement in a scope.
 * Lexer
 *   start: first token of statement
 *   end: first token of next statement
 */
stmt_p parse_stmt(Lexer& lexer, bool* contains_return = nullptr);

/* This function handles three cases:
 *   variable initialization
 *   variable assignment
 *   function calls
 * Lexer:
 *   start: variable name
 *   end: first token of next statement
 */
stmt_p parse_var(Lexer& lexer);

/*
 * Lexer starts at variable type and ends at semicolon.
 * 
 * Examples,
 *   my_var int;
 *   my_var int = <expr>;
 */
VariableInit parse_variable_initialization(
	Lexer& lexer,
	Token const& name
);

/*
 * Lexer starts at variable type and ends at semicolon.
 *
 * Examples,
 *   my_var int[<expr>];
 *   my_var int[<expr>] = [<expr>];
 */
ArrayInitialization parse_array_initialization(
	Lexer& lexer,
	Token const& name
);

/*
 * It is the callers responsibility to check if lexer.curr() is their expected
 * token after this function is called.
 * 
 * Lexer starts at assignment operator and ends at first token of next
 * statement.
 * 
 * Examples,
 *   my_var = <expr>;
 *   for (;; my_var += <expr>) {}
 */
VariableAssign parse_variable_assignment(
	Lexer& lexer,
	Token const& name
);

/*
 * It is the callers responsibility to check if lexer.curr() is their expected
 * token after this function is called.
 * 
 * Lexer starts at open bracket and ends at closing bracket.
 * 
 * Examples,
 *   my_func();
 *	 my_var = my_func();
 */
expr::FunctionCall parse_func_call(
	Lexer& lexer,
	Token const& name
);

/* Parses the entire conditional chain (every 'else if' and 'else' that follows)
 * Lexer:
 *   start: if token
 *   end: first token of next statement
 */
Conditional parse_if(Lexer& lexer, bool* contains_return = nullptr);

/* Lexer:
 *   start: while token
 *   end: first token of next statement
 */
While parse_while(Lexer& lexer, bool* contains_return = nullptr);

/* Lexer:
 *   start: while token
 *   end: first token of next statement
 */
For parse_for(Lexer& lexer, bool* contains_return = nullptr);

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