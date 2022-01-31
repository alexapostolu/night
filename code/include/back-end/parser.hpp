#pragma once

#include "lexer.hpp"
#include "token.hpp"
#include "stmt.hpp"
#include "check.hpp"

#include <memory>
#include <tuple>
#include <vector>
#include <string>
#include <unordered_set>
#include <functional>

#define NIGHT_COMPILE_ERROR(msg, fix) \
	night::error(__FILE__, __LINE__, night::error_compile, lexer.get_loc(), msg, fix)

// one parser for each file
class Parser
{
public:
	using ParserScope = Scope<CheckVariable>;

public:
	Parser(
		Lexer& lexer
	);

public:
	/*
	 * parses statement
	 *   ex. while loop, function definition
	 *
	 * lexer should be at first token of statement; ends at first token of next
	 * statement
	 */
	Stmt parse_statement(
		ParserScope& scope
	);

	Stmt parse_stmt_let(ParserScope& scope);
	Stmt parse_stmt_var(ParserScope& scope);
	Stmt parse_stmt_if(ParserScope& scope);
	Stmt parse_stmt_fn(ParserScope& scope, CheckFunctionContainer::iterator &in_func);
	Stmt parse_stmt_rtn(ParserScope& scope, CheckFunctionContainer::iterator &in_func);
	Stmt parse_stmt_loop(ParserScope& scope);

private:
	/*
	 * parses body that precedes a statement
	 *   ex. body of conditionals, loops, or functions
	 *
	 * lexer should be at first token of body (including curly brackets)
	 * ends at first token of next statement
	 */
	std::vector<Stmt> parse_body(
		ParserScope& scope,
		std::string const& stmt_name,
		std::string const& stmt_format
	);

	/*
	 * parses condition
	 *   ex. conditionals, loops
	 *
	 * lexer should be at token before opening bracket; ends at first token of body
	 */
	std::shared_ptr<ExprNode> parse_condition(
		ParserScope& scope,
		std::string const& stmt_format
	);

	/*
	 * parses arguments
	 *   ex. function call, methods
	 *
	 * lexer should be at opening bracket token; ends at closing bracket
	 */
	std::pair<ExprContainer, std::vector<TypeContainer> > parse_arguments(
		ParserScope& scope,
		std::string_view func_name
	);

	/*
	 * lexer starts at first token of expression
	 * ends at the first non-expression token of same line
	 */
	std::tuple<std::shared_ptr<ExprNode>, TypeContainer> parse_expression(
		ParserScope& scope,
		TypeContainer const& required_types = {}
	);

	bool higher_precedence(
		std::string const& op1,
		std::string const& op2
	) const;

	TypeContainer type_check_expr(
		ParserScope& scope,
		std::shared_ptr<ExprNode> const& expr,
		TypeContainer const& required_types = {},
		TypeContainer* const_types = nullptr, // used by if statements for type(var) == int
		CheckVariable* const_var = nullptr
	) const;

	void check_call_types(
		std::vector<TypeContainer> const& param_types,
		std::vector<TypeContainer> const& arg_types,
		std::string const& func_name
	);

	std::string types_as_str(
		TypeContainer const& var_types_set
	) const;

	inline void throw_binary_type_err(
		BinaryOPNode const& op,
		TypeContainer const& types,
		std::string const& side,
		std::string const& used_types) const;

private:
	Lexer& lexer;

private:
	static TypeContainer const all_types;

	CheckFunctionContainer check_funcs;
	CheckClassContainer check_classes;
};