#include "parser/statement_parser.hpp"

#include "lexer.hpp"
#include "ast/statement.hpp"
#include "ast/expression.hpp"
#include "parser/expression_parser.hpp"
#include "value_type.hpp"
#include "error.hpp"
#include "debug.hpp"

#include <vector>
#include <string>
#include <assert.h>

std::vector<stmt_p> parse_file(std::string const& main_file)
{
	Lexer lexer(main_file);
	std::vector<stmt_p> stmts;

	while (lexer.curr().type != TokenType::END_OF_FILE)
	{
		auto stmt = parse_stmts(lexer, false);
		stmts.insert(std::end(stmts), std::begin(stmt), std::end(stmt));
	}

	return stmts;
}

std::vector<stmt_p> parse_stmts(Lexer& lexer, bool requires_curly)
{
	// two cases:
	//   { stmt1; stmt2; ... }
	//   stmt1;
	switch (lexer.curr().type)
	{
	case TokenType::OPEN_CURLY:
	{
		std::vector<stmt_p> stmts;

		lexer.eat();

		while (lexer.curr().type != TokenType::CLOSE_CURLY)
		{
			stmts.push_back(parse_stmt(lexer));

			if (lexer.curr().type == TokenType::END_OF_FILE)
				throw NIGHT_CREATE_FATAL("missing closing curly bracket");
		}

		lexer.eat();

		return stmts;
	}
	case TokenType::END_OF_FILE:
		return {};
	default:
		if (requires_curly)
			throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected opening curly bracket");

		return { parse_stmt(lexer) };
	}
}

stmt_p parse_stmt(Lexer& lexer)
{
	switch (lexer.curr().type)
	{
	case TokenType::VARIABLE: return parse_var(lexer);
	case TokenType::IF:		  return std::make_shared<Conditional>(parse_if(lexer));
	case TokenType::FOR:	  return std::make_shared<For>(parse_for(lexer));
	case TokenType::WHILE:	  return std::make_shared<While>(parse_while(lexer));
	case TokenType::DEF:	  return std::make_shared<Function>(parse_func(lexer));
	case TokenType::RETURN:	  return std::make_shared<Return>(parse_return(lexer));

	case TokenType::ELIF: throw NIGHT_CREATE_FATAL("elif statement must come before an if or elif statement");
	case TokenType::ELSE: throw NIGHT_CREATE_FATAL("else statement must come before an if or elif statement");
	default: throw NIGHT_CREATE_FATAL("unknown syntax '" + lexer.curr().str + "'");
	}
}

stmt_p parse_var(Lexer& lexer)
{
	assert(lexer.curr().type == TokenType::VARIABLE);

	std::string var_name = lexer.curr().str;

	switch (lexer.peek().type)
	{
	case TokenType::TYPE: {
		lexer.eat();

		auto const& ast = std::make_shared<VariableInit>(parse_var_init(lexer, var_name));

		lexer.eat();
		return ast;
	}
	case TokenType::ASSIGN: 
	case TokenType::ASSIGN_OPERATOR: {
		lexer.eat();

		auto const& ast = std::make_shared<VariableAssign>(parse_var_assign(lexer, var_name));
		lexer.curr_check(TokenType::SEMICOLON);

		lexer.eat();
		return ast;
	}
	case TokenType::OPEN_SQUARE: {
		auto const& ast = std::make_shared<ArrayMethod>(parse_array_method(lexer, var_name));
		return ast;
	}
	case TokenType::OPEN_BRACKET: {
		lexer.eat();

		auto const& ast = std::make_shared<expr::FunctionCall>(parse_func_call(lexer, var_name));
		lexer.expect(TokenType::SEMICOLON);

		lexer.eat();
		return ast;
	}
	default:
		throw night::error::get().create_fatal_error("found '" + lexer.peek().str + "', expected type, assign, open square, or open bracket", lexer.loc);
	}
}

VariableInit parse_var_init(Lexer& lexer, std::string const& var_name)
{
	assert(lexer.curr().type == TokenType::TYPE);

	// Determine the subscript expressions if any (for arrays), and construct
	// ValueType for the variable.

	auto type_str = lexer.curr().str;

	std::vector<expr::expr_p> arr_sizes;
	while (lexer.eat().type == TokenType::OPEN_SQUARE)
		arr_sizes.push_back(parse_expr(lexer, false, TokenType::CLOSE_SQUARE));

	// Determine the expression the variable is being assigned to, if any.

	expr::expr_p expr(nullptr);
	
	if (lexer.curr().type == TokenType::ASSIGN)
		expr = parse_expr(lexer, true);

	lexer.curr_check(TokenType::SEMICOLON);

	return VariableInit(lexer.loc, var_name, type_str, arr_sizes, expr);
}

VariableAssign parse_var_assign(Lexer& lexer, std::string const& var_name)
{
	assert(lexer.curr().type == TokenType::ASSIGN ||
		   lexer.curr().type == TokenType::ASSIGN_OPERATOR);

	auto assign_operator = lexer.curr().str;
	auto expression		 = parse_expr(lexer, true);

	return VariableAssign(lexer.loc, var_name, assign_operator, expression);
}

ArrayMethod parse_array_method(Lexer& lexer, std::string const& var_name)
{
	assert(lexer.curr().type == TokenType::VARIABLE);

	// Parse subscripts.

	std::vector<expr::expr_p> subscripts;

	while (lexer.eat().type == TokenType::OPEN_SQUARE)
		subscripts.push_back(parse_expr(lexer, true, TokenType::CLOSE_SQUARE));

	// Parse operator and expression.

	auto assign_operator = lexer.curr_check(TokenType::ASSIGN).str;
	auto assign_expr	 = parse_expr(lexer, true, TokenType::SEMICOLON);

	lexer.eat();
	return ArrayMethod(lexer.loc, var_name, assign_operator, subscripts, assign_expr);
}

expr::FunctionCall parse_func_call(Lexer& lexer, std::string const& func_name)
{
	assert(lexer.curr().type == TokenType::OPEN_BRACKET);

	std::vector<expr::expr_p> arg_exprs;

	while (true)
	{
		auto expr = parse_expr(lexer, false);

		if (!expr)
		{
			lexer.curr_check(TokenType::CLOSE_BRACKET);
			break;
		}

		arg_exprs.push_back(expr);

		if (lexer.curr().type == TokenType::CLOSE_BRACKET)
			break;

		lexer.curr_check(TokenType::COMMA);
	}

	return expr::FunctionCall(lexer.loc, func_name, arg_exprs);
}

Conditional parse_if(Lexer& lexer)
{
	assert(lexer.curr().type == TokenType::IF);

	conditional_container conditionals;

	do {
		// Default for else statements.
		expr::expr_p cond_expr =
			std::make_shared<expr::Numeric>(lexer.loc, ValueType::BOOL, 1);

		// Parse condition.
		if (lexer.curr().type != TokenType::ELSE)
		{
			lexer.expect(TokenType::OPEN_BRACKET);

			cond_expr = parse_expr(lexer, true, TokenType::CLOSE_BRACKET);
		}

		// Parse body.
		lexer.eat();
		conditionals.push_back({ cond_expr, parse_stmts(lexer, false) });

	} while (lexer.curr().type == TokenType::IF	  ||
			 lexer.curr().type == TokenType::ELIF ||
			 lexer.curr().type == TokenType::ELSE);

	return Conditional(lexer.loc, conditionals);
}

While parse_while(Lexer& lexer)
{
	assert(lexer.curr().type == TokenType::WHILE);

	// Parse condition.
	lexer.expect(TokenType::OPEN_BRACKET);

	auto cond_expr = parse_expr(lexer, true, TokenType::CLOSE_BRACKET);

	// Parse body.
	lexer.eat();
	return While(lexer.loc, cond_expr, parse_stmts(lexer, false));
}

For parse_for(Lexer& lexer)
{
	assert(lexer.curr().type == TokenType::FOR);

	lexer.expect(TokenType::OPEN_BRACKET);

	// Parse variable initialization.

	auto var_init_name = lexer.expect(TokenType::VARIABLE).str;
	lexer.expect(TokenType::TYPE);

	auto var_init = parse_var_init(lexer, var_init_name);
	
	// Parse condition.

	auto condition = parse_expr(lexer, true, TokenType::SEMICOLON);

	// Parse assignment.

	std::string var_assign_name = lexer.expect(TokenType::VARIABLE).str;
	lexer.eat();

	auto var_assign = parse_var_assign(lexer, var_assign_name);
	lexer.curr_check(TokenType::CLOSE_BRACKET);

	// Parse body, and include the increment statement in the body.

	lexer.eat();

	auto body = parse_stmts(lexer, false);
	body.push_back(std::make_shared<VariableAssign>(var_assign));

	return For(lexer.loc, var_init, condition, body);
}

Function parse_func(Lexer& lexer) 
{
	assert(lexer.curr().type == TokenType::DEF);

	std::string func_name = lexer.expect(TokenType::VARIABLE).str;

	// Parser parameters.

	lexer.expect(TokenType::OPEN_BRACKET);

	// Name and types.
	std::vector<std::pair<std::string, std::string>> parameters;
	std::vector<std::string> param_types;

	while (true)
	{
		if (lexer.eat().type == TokenType::CLOSE_BRACKET)
			break;

		lexer.curr_check(TokenType::VARIABLE);

		parameters.push_back(std::make_pair(
			lexer.curr().str,
			lexer.expect(TokenType::TYPE).str));

		if (lexer.eat().type == TokenType::CLOSE_BRACKET)
			break;
		
		lexer.curr_check(TokenType::COMMA);
	}

	// Parse return type.

	auto rtn_type = lexer.eat();

	if (rtn_type.type != TokenType::TYPE && rtn_type.type != TokenType::VOID)
		throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected return type");

	// Parse body.

	lexer.eat();
	auto body = parse_stmts(lexer, true);

	return Function(lexer.loc, func_name, parameters, rtn_type.str, body);
}

Return parse_return(Lexer& lexer)
{
	assert(lexer.curr().type == TokenType::RETURN);

	auto const& expr = parse_expr(lexer, false, TokenType::SEMICOLON);

	lexer.eat();
	return Return(lexer.loc, expr);
}