#include "parser/statement_parser.hpp"

#include "lexer.hpp"
#include "ast/statement.hpp"
#include "ast/expression.hpp"
#include "parser/expression_parser.hpp"
#include "error.hpp"

#include <vector>
#include <tuple>
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
				throw night::create_fatal_error("missing closing curly bracket", lexer.loc);
		}

		lexer.eat();

		return stmts;
	}
	case TokenType::END_OF_FILE:
		return {};
	default:
		if (requires_curly)
			throw night::create_fatal_error("found '" + lexer.curr().str + "', expected opening curly bracket", lexer.loc);

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

	case TokenType::ELIF: throw night::create_fatal_error("elif statement must come before an if or elif statement", lexer.loc);
	case TokenType::ELSE: throw night::create_fatal_error("else statement must come before an if or elif statement", lexer.loc);
	default: throw night::create_fatal_error("unknown syntax '" + lexer.curr().str + "'", lexer.loc);
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

		if (lexer.peek().type == TokenType::OPEN_SQUARE)
		{
			auto const& ast = std::make_shared<ArrayInitialization>(parse_array_init(lexer, var_name));

			lexer.eat();
			return ast;
		}
		else
		{
			auto const& ast = std::make_shared<VariableInit>(parse_var_init(lexer, var_name));

			lexer.eat();
			return ast;
		}
	}
	case TokenType::ASSIGN: 
	case TokenType::ASSIGN_OPERATOR: {
		lexer.eat();

		auto const& ast = std::make_shared<VariableAssign>(parse_var_assign(lexer, var_name));
		lexer.curr_is(TokenType::SEMICOLON);

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
	default: {
		throw night::create_fatal_error("found '" + lexer.peek().str + "', expected type, assign, open square, or open bracket", lexer.loc);
	}
	}
}

VariableInit parse_var_init(Lexer& lexer, std::string const& var_name)
{
	assert(lexer.curr().type == TokenType::TYPE);

	auto type = lexer.curr().str;

	// Determine the expression the variable is being assigned to, if any.

	expr::expr_p expr(nullptr);
	
	if (lexer.eat().type == TokenType::ASSIGN)
		expr = parse_expr(lexer, true);

	lexer.curr_is(TokenType::SEMICOLON);

	return VariableInit(lexer.loc, var_name, type, expr);
}

ArrayInitialization parse_array_init(Lexer& lexer, std::string const& var_name)
{
	assert(lexer.curr().type == TokenType::TYPE);

	auto type = lexer.curr().str;

	lexer.eat();

	std::vector<expr::expr_p> arr_sizes;
	while (lexer.curr().type == TokenType::OPEN_SQUARE)
	{
		arr_sizes.push_back(parse_expr(lexer, false, TokenType::CLOSE_SQUARE));
		lexer.eat();
	}

	// Determine the expression the variable is being assigned to, if any.

	expr::expr_p expr(nullptr);

	if (lexer.curr().type == TokenType::ASSIGN)
		expr = parse_expr(lexer, true);

	lexer.curr_is(TokenType::SEMICOLON);

	return ArrayInitialization(lexer.loc, var_name, type, arr_sizes, expr);
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

	if (lexer.curr().type != TokenType::ASSIGN && lexer.curr().type != TokenType::ASSIGN_OPERATOR)
		throw night::create_fatal_error("found '" + lexer.curr().str + "', expected assignment operator", lexer.loc);

	auto assign_operator = lexer.curr().str;
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
			lexer.curr_is(TokenType::CLOSE_BRACKET);
			break;
		}

		arg_exprs.push_back(expr);

		if (lexer.curr().type == TokenType::CLOSE_BRACKET)
			break;

		lexer.curr_is(TokenType::COMMA);
	}

	return expr::FunctionCall(lexer.loc, func_name, arg_exprs);
}

Conditional parse_if(Lexer& lexer)
{
	assert(lexer.curr().type == TokenType::IF);

	conditional_container conditionals;

	do {
		// Default to else statement.
		expr::expr_p cond_expr(nullptr);

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
	lexer.curr_is(TokenType::CLOSE_BRACKET);

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
	std::vector<std::tuple<std::string, std::string, bool>> parameters;

	while (true)
	{
		if (lexer.eat().type == TokenType::CLOSE_BRACKET)
			break;

		auto name = lexer.curr_is(TokenType::VARIABLE).str;
		auto type = lexer.expect(TokenType::TYPE).str;
		auto is_arr = false;

		if (lexer.eat().type == TokenType::CLOSE_BRACKET)
		{
			parameters.push_back(std::make_tuple(name, type, is_arr));
			break;
		}

		if (lexer.curr().type == TokenType::OPEN_SQUARE)
		{
			lexer.expect(TokenType::CLOSE_SQUARE);
			is_arr = true;
		}
		
		lexer.curr_is(TokenType::COMMA);

		parameters.push_back(std::make_tuple(name, type, is_arr));
	}

	// Parse return type.

	auto rtn_type = lexer.eat();

	if (rtn_type.type != TokenType::TYPE && rtn_type.type != TokenType::VOID)
		throw night::create_fatal_error("found '" + lexer.curr().str + "', expected return type", lexer.loc);

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