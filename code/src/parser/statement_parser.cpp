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

std::vector<stmt_p> parse_stmts(Lexer& lexer, bool requires_curly, bool* contains_return)
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

		bool body_contains_return = false;

		while (lexer.curr().type != TokenType::CLOSE_CURLY)
		{
			stmts.push_back(parse_stmt(lexer, &body_contains_return));

			if (lexer.curr().type == TokenType::END_OF_FILE)
				throw night::error::get().create_fatal_error("missing closing curly bracket", lexer.loc);
		}

		if (body_contains_return)
			*contains_return = true;

		lexer.eat();

		return stmts;
	}
	case TokenType::END_OF_FILE:
		return {};
	default:
		if (requires_curly)
			throw night::error::get().create_fatal_error("found '" + lexer.curr().str + "', expected opening curly bracket", lexer.loc);

		return { parse_stmt(lexer, contains_return) };
	}
}

stmt_p parse_stmt(Lexer& lexer, bool* contains_return)
{
	switch (lexer.curr().type)
	{
	case TokenType::VARIABLE: return parse_var(lexer);
	case TokenType::IF:		  return std::make_shared<Conditional>(parse_if(lexer, contains_return));
	case TokenType::WHILE:	  return std::make_shared<While>(parse_while(lexer, contains_return));
	case TokenType::FOR:	  return std::make_shared<For>(parse_for(lexer, contains_return));
	case TokenType::DEF:	  return std::make_shared<Function>(parse_func(lexer));
	case TokenType::RETURN: {
		if (contains_return)
			*contains_return = true;

		return std::make_shared<Return>(parse_return(lexer));
	}

	case TokenType::ELIF: throw night::error::get().create_fatal_error("elif statement must come before an if or elif statement", lexer.loc);
	case TokenType::ELSE: throw night::error::get().create_fatal_error("else statement must come before an if or elif statement", lexer.loc);
	default: throw night::error::get().create_fatal_error("unknown syntax '" + lexer.curr().str + "'", lexer.loc);
	}
}

stmt_p parse_var(Lexer& lexer)
{
	assert(lexer.curr().type == TokenType::VARIABLE);

	Token name = lexer.curr();

	switch (lexer.peek().type)
	{
	case TokenType::TYPE: {
		lexer.eat();

		if (lexer.peek().type == TokenType::OPEN_SQUARE)
		{
			auto const& ast = std::make_shared<ArrayInitialization>(parse_array_initialization(lexer, name));

			lexer.eat();
			return ast;
		}
		else
		{
			auto const& ast = std::make_shared<VariableInit>(parse_variable_initialization(lexer, name));

			lexer.eat();
			return ast;
		}
	}
	case TokenType::ASSIGN: 
	case TokenType::ASSIGN_OPERATOR: {
		lexer.eat();

		auto const& ast = std::make_shared<VariableAssign>(parse_variable_assignment(lexer, name));
		lexer.curr_is(TokenType::SEMICOLON);

		lexer.eat();
		return ast;
	}
	case TokenType::OPEN_SQUARE: {
		return std::make_shared<ArrayMethod>(parse_array_method(lexer, name));
	}
	case TokenType::OPEN_BRACKET: {
		lexer.eat();

		auto const& ast = std::make_shared<expr::FunctionCall>(parse_func_call(lexer, name));
		lexer.expect(TokenType::SEMICOLON);

		lexer.eat();
		return ast;
	}
	default: {
		throw night::error::get().create_fatal_error("found '" + lexer.peek().str + "', expected type, assign, open square, or open bracket", lexer.loc);
	}
	}
}

VariableInit parse_variable_initialization(Lexer& lexer, Token const& name)
{
	assert(lexer.curr().type == TokenType::TYPE);

	std::string type = lexer.curr().str;
	expr::expr_p expr;

	if (lexer.eat().type == TokenType::ASSIGN)
		expr = parse_expr(lexer, true);

	lexer.curr_is(TokenType::SEMICOLON);

	return VariableInit(name.str, name.loc, type, expr);
}

ArrayInitialization parse_array_initialization(Lexer& lexer, Token const& name)
{
	assert(lexer.curr().type == TokenType::TYPE);

	std::string type = lexer.curr().str;

	lexer.eat();

	std::vector<expr::expr_p> array_sizes;
	array_sizes.reserve(AVG_ARRAY_DIMENSION);

	while (lexer.curr().type == TokenType::OPEN_SQUARE)
	{
		expr::expr_p array_size = parse_expr(lexer, false, TokenType::CLOSE_SQUARE);
		array_sizes.push_back(array_size);

		lexer.eat();
	}

	expr::expr_p expr;

	if (lexer.curr().type == TokenType::ASSIGN)
		expr = parse_expr(lexer, true);

	lexer.curr_is(TokenType::SEMICOLON);

	return ArrayInitialization(name.str, name.loc, type, array_sizes, expr);
}

VariableAssign parse_variable_assignment(Lexer& lexer, Token const& name)
{
	assert(lexer.curr().type == TokenType::ASSIGN ||
		   lexer.curr().type == TokenType::ASSIGN_OPERATOR);

	std::string	 assignment_operator = lexer.curr().str;
	expr::expr_p expression			 = parse_expr(lexer, true);

	return VariableAssign(lexer.loc, name.str, assignment_operator, expression, name.loc);
}

ArrayMethod parse_array_method(Lexer& lexer, Token const& name)
{
	assert(lexer.curr().type == TokenType::VARIABLE);

	// Parse subscripts.

	std::vector<expr::expr_p> subscripts;
	subscripts.reserve(AVG_ARRAY_DIMENSION);

	while (lexer.eat().type == TokenType::OPEN_SQUARE)
	{
		expr::expr_p subscript = parse_expr(lexer, true, TokenType::CLOSE_SQUARE);
		subscripts.emplace_back(subscript);
	}

	// Parse operator and expression.

	if (lexer.curr().type != TokenType::ASSIGN && lexer.curr().type != TokenType::ASSIGN_OPERATOR)
		throw night::error::get().create_fatal_error("Found '" + lexer.curr().str + "', expected assignment operator", lexer.loc);

	auto assign_operator = lexer.curr().str;
	auto assign_expr = parse_expr(lexer, true, TokenType::SEMICOLON);

	lexer.eat();
	return ArrayMethod(lexer.loc, name.str, assign_operator, subscripts, assign_expr);
}

expr::FunctionCall parse_func_call(Lexer& lexer, Token const& name)
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

	return expr::FunctionCall(name, arg_exprs);
}

Conditional parse_if(Lexer& lexer, bool* contains_return)
{
	assert(lexer.curr().type == TokenType::IF);

	conditional_container conditionals;

	int number_of_returns = 0;
	bool contains_else = false;

	do {
		// Default to else statement.
		expr::expr_p cond_expr(nullptr);

		// Parse condition.
		if (lexer.curr().type != TokenType::ELSE)
		{
			lexer.expect(TokenType::OPEN_BRACKET);

			cond_expr = parse_expr(lexer, true, TokenType::CLOSE_BRACKET);
		}
		else
		{
			contains_else = true;
		}

		// Parse body.
		lexer.eat();
		bool body_contains_return = false;
		conditionals.push_back({ cond_expr, parse_stmts(lexer, false, &body_contains_return) });

		if (body_contains_return)
			number_of_returns++;

	} while (lexer.curr().type == TokenType::IF	  ||
			 lexer.curr().type == TokenType::ELIF ||
			 lexer.curr().type == TokenType::ELSE);

	// Every conditional must have a return statement.
	if (contains_return && number_of_returns == conditionals.size() && contains_else)
		*contains_return = true;

	return Conditional(lexer.loc, conditionals);
}

While parse_while(Lexer& lexer, bool* contains_return)
{
	assert(lexer.curr().type == TokenType::WHILE);

	// Parse condition.
	lexer.expect(TokenType::OPEN_BRACKET);

	auto cond_expr = parse_expr(lexer, true, TokenType::CLOSE_BRACKET);

	// Parse body.
	lexer.eat();
	return While(lexer.loc, cond_expr, parse_stmts(lexer, false, contains_return));
}

For parse_for(Lexer& lexer, bool* contains_return)
{
	assert(lexer.curr().type == TokenType::FOR);

	lexer.expect(TokenType::OPEN_BRACKET);

	// Parse variable initialization.

	Token variable_initialization = lexer.expect(TokenType::VARIABLE);
	lexer.expect(TokenType::TYPE);

	auto var_init = parse_variable_initialization(lexer, variable_initialization);
	
	// Parse condition.

	auto condition = parse_expr(lexer, true, TokenType::SEMICOLON);

	// Parse assignment.

	Token variable_assignment = lexer.expect(TokenType::VARIABLE);

	lexer.eat();

	auto var_assign = parse_variable_assignment(lexer, variable_assignment);
	lexer.curr_is(TokenType::CLOSE_BRACKET);

	// Parse body, and include the increment statement in the body.

	lexer.eat();

	auto body = parse_stmts(lexer, false, contains_return);
	body.push_back(std::make_shared<VariableAssign>(var_assign));

	return For(lexer.loc, var_init, condition, body);
}

Function parse_func(Lexer& lexer) 
{
	assert(lexer.curr().type == TokenType::DEF);

	std::string name = lexer.expect(TokenType::VARIABLE).str;
	Location name_location = lexer.loc;

	// Parser parameters.

	lexer.expect(TokenType::OPEN_BRACKET);

	std::vector<Parameter> parameters;

	while (true)
	{
		if (lexer.eat().type == TokenType::CLOSE_BRACKET)
			break;

		std::string name = lexer.curr_is(TokenType::VARIABLE).str;
		Location location = lexer.loc;
		std::string type = lexer.expect(TokenType::TYPE).str;
		bool is_arr = false;

		if (lexer.eat().type == TokenType::CLOSE_BRACKET)
		{
			parameters.push_back({ name, type, is_arr, location });
			break;
		}

		if (lexer.curr().type == TokenType::OPEN_SQUARE)
		{
			lexer.expect(TokenType::CLOSE_SQUARE);
			is_arr = true;

			if (lexer.eat().type == TokenType::CLOSE_BRACKET)
			{
				parameters.push_back({ name, type, is_arr, location });
				break;
			}
		}
		
		lexer.curr_is(TokenType::COMMA);

		parameters.push_back({ name, type, is_arr, location });
	}

	// Parse return type.

	lexer.eat();
	auto rtn_type = parse_type(lexer);

	// Parse body.

	bool contains_return = false;
	auto body = parse_stmts(lexer, true, &contains_return);

	if (std::get<0>(rtn_type) == "void" && contains_return) {
		throw night::error::get().create_fatal_error(
			"found return statement, expected no return statement in void function", lexer.loc);
	}
	if (std::get<0>(rtn_type) != "void" && !contains_return) {
		throw night::error::get().create_fatal_error(
			"found no return statement, expected return statement in function", lexer.loc);
	}

	return Function(name, name_location, parameters, std::get<0>(rtn_type), std::get<1>(rtn_type), body);
}

Return parse_return(Lexer& lexer)
{
	assert(lexer.curr().type == TokenType::RETURN);

	auto const& expr = parse_expr(lexer, false, TokenType::SEMICOLON);

	lexer.eat();
	return Return(lexer.loc, expr);
}

std::tuple<std::string, int> parse_type(Lexer& lexer)
{
	if (lexer.curr().type != TokenType::TYPE && lexer.curr().type != TokenType::VOID)
		throw night::error::get().create_fatal_error("found '" + lexer.curr().str + "', expected return type", lexer.loc);

	auto type = lexer.curr().str;
	int dim = 0;

	while (lexer.eat().type == TokenType::OPEN_SQUARE)
	{
		lexer.expect(TokenType::CLOSE_SQUARE);
		++dim;
	}

	return std::make_tuple(type, dim);
}