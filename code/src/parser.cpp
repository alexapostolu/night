#include "parser.hpp"
#include "lexer.hpp"
#include "parser_scope.hpp"
#include "bytecode.hpp"
#include "interpreter_scope.hpp"
#include "ast/ast.hpp"
#include "ast/expression.hpp"
#include "value_type.hpp"
#include "utils.hpp"
#include "error.hpp"
#include "debug.hpp"

#include <unordered_map>
#include <variant>
#include <string>
#include <assert.h>

AST_Block parse_file(std::string const& main_file)
{
	Lexer lexer(main_file);
	AST_Block block;
	
	while (lexer.curr().type != TokenType::END_OF_FILE)
	{
		auto stmt = parse_stmts(lexer);
		block.insert(std::end(block), std::begin(stmt), std::end(stmt));
	}

	return block;
}

std::vector<std::shared_ptr<AST>> parse_stmts(Lexer& lexer, bool* curly_enclosed)
{
	bool conditional = false;

	// two cases:
	//   { stmt1; stmt2; ... }
	//   stmt1;
	switch (lexer.curr().type)
	{
	case TokenType::OPEN_CURLY:
	{
		if (curly_enclosed)
			*curly_enclosed = true;

		std::vector<std::shared_ptr<AST>> block;

		lexer.eat();

		while (lexer.curr().type != TokenType::CLOSE_CURLY)
		{
			block.push_back(parse_stmt(lexer));

			if (lexer.curr().type == TokenType::END_OF_FILE)
				throw NIGHT_CREATE_FATAL("missing closing curly bracket");
		}

		lexer.eat();

		return block;
	}
	case TokenType::END_OF_FILE:
		return {};
	default:
		return { parse_stmt(lexer) };
	}
}

std::shared_ptr<AST> parse_stmt(Lexer& lexer)
{
	switch (lexer.curr().type)
	{
	case TokenType::VARIABLE: return parse_var(lexer);
	case TokenType::IF:		  return std::make_shared<Conditional>(parse_if(lexer));
	case TokenType::ELIF:	  throw NIGHT_CREATE_FATAL("elif statement must come before an if or elif statement");
	case TokenType::ELSE:	  throw NIGHT_CREATE_FATAL("else statement must come before an if or elif statement");
	case TokenType::FOR:	  return std::make_shared<For>(parse_for(lexer));
	case TokenType::WHILE:	  return std::make_shared<While>(parse_while(lexer));
	case TokenType::DEF:	  return std::make_shared<Function>(parse_func(lexer));
	case TokenType::RETURN:	  return std::make_shared<Return>(parse_return(lexer));

	default: throw NIGHT_CREATE_FATAL("unknown syntax '" + lexer.curr().str + "'");
	}
}

std::shared_ptr<AST> parse_var(Lexer& lexer)
{
	std::string var_name = lexer.curr().str;

	if (lexer.peek().is_type())
	{
		lexer.eat();

		auto const& ast = std::make_shared<VariableInit>(parse_var_init(lexer, var_name));

		lexer.eat();
		return ast;
	}
	else if (lexer.peek().type == TokenType::ASSIGN)
	{
		lexer.eat();

		auto const& ast = std::make_shared<VariableAssign>(parse_var_assign(lexer, var_name));
		lexer.curr_check(TokenType::SEMICOLON);

		lexer.eat();
		return ast;
	}
	else if (lexer.peek().type == TokenType::OPEN_SQUARE)
	{
		auto const& ast = std::make_shared<ArrayMethod>(parse_array_method(lexer, var_name));
		return ast;
	}
	else if (lexer.peek().type == TokenType::OPEN_BRACKET)
	{
		lexer.eat();
		auto const& ast = std::make_shared<expr::FunctionCall>(parse_func_call(lexer, var_name));
		lexer.expect(TokenType::SEMICOLON);
		lexer.eat();
		return ast;
	}
}

VariableInit parse_var_init(Lexer& lexer, std::string const& var_name)
{
	assert(lexer.curr().is_type());
	ValueType var_type(token_var_type_to_val_type(lexer.curr().str));

	bool is_arr = false;
	std::vector<std::optional<expr::expr_p>> arr_sizes;
	while (lexer.eat().type == TokenType::OPEN_SQUARE)
	{
		arr_sizes.push_back(parse_expr(lexer, true, false));
		lexer.curr_check(TokenType::CLOSE_SQUARE);
		
		is_arr = true;
	}

	var_type.dim = arr_sizes.size();

	// default value
	expr::expr_p expr;
	if (is_arr)
		expr = std::make_shared<expr::Array>(lexer.loc, std::vector<expr::expr_p>());
	else
		expr = std::make_shared<expr::Value>(lexer.loc, var_type.type, "0");

	if (lexer.curr().type == TokenType::ASSIGN && lexer.curr().str == "=")
	{
		expr = parse_expr(lexer, true, true);
		lexer.curr_check(TokenType::SEMICOLON);
	}
	else if (lexer.curr().type == TokenType::ASSIGN && lexer.curr().str != "=")
	{
		throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected assignment '='")
	}
	else if (lexer.curr().type != TokenType::SEMICOLON)
	{
		throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "' expected semicolon or assignment after variable type");
	}

	return VariableInit(lexer.loc, var_name, var_type, arr_sizes, expr);
}

VariableAssign parse_var_assign(Lexer& lexer, std::string const& var_name)
{
	assert(lexer.curr().type == TokenType::ASSIGN);

	auto assign_op = lexer.curr().str;

	lexer.eat();
	auto expr = parse_expr(lexer, false, true);

	return VariableAssign(lexer.loc, var_name, assign_op, expr);
}

ArrayMethod parse_array_method(Lexer& lexer, std::string const& var_name)
{
	assert(lexer.curr().type == TokenType::VARIABLE);

	std::vector<expr::expr_p> subscripts;

	while (lexer.eat().type == TokenType::OPEN_SQUARE)
	{
		lexer.eat();
		subscripts.push_back(parse_expr(lexer, false, true));

		lexer.curr_check(TokenType::CLOSE_SQUARE);
	}


	if (lexer.curr().type == TokenType::SEMICOLON)
	{
		lexer.eat();
		return ArrayMethod(lexer.loc, var_name, subscripts, nullptr);
	}

	lexer.curr_check(TokenType::ASSIGN);

	lexer.eat();
	auto assign_expr = parse_expr(lexer, false, true);
	lexer.curr_check(TokenType::SEMICOLON);

	lexer.eat();
	return ArrayMethod(lexer.loc, var_name, subscripts, assign_expr);
}

expr::FunctionCall parse_func_call(Lexer& lexer, std::string const& func_name)
{
	assert(lexer.curr().type == TokenType::OPEN_BRACKET);

	// parse argument expressions and types

	std::vector<expr::expr_p> arg_exprs;

	while (true)
	{
		lexer.eat();
		auto expr = parse_expr(lexer, false, false);

		// case:
		//   func_call();
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
	std::vector<std::pair<expr::expr_p, AST_Block>> conditionals;

	do {
		// default for else statements
		expr::expr_p cond_expr =
			std::make_shared<expr::Value>(lexer.loc, ValueType::BOOL, "true");

		// parse condition
		if (lexer.curr().type != TokenType::ELSE)
		{
			lexer.expect(TokenType::OPEN_BRACKET);

			cond_expr = parse_expr(lexer, true, true);
			lexer.curr_check(TokenType::CLOSE_BRACKET);
		}

		lexer.eat();
		conditionals.push_back({ cond_expr, parse_stmts(lexer) });

	} while (lexer.curr().type == TokenType::IF	  ||
			 lexer.curr().type == TokenType::ELIF ||
			 lexer.curr().type == TokenType::ELSE);

	return Conditional(lexer.loc, conditionals);
}

While parse_while(Lexer& lexer)
{
	lexer.expect(TokenType::OPEN_BRACKET);

	auto cond_expr = parse_expr(lexer, true, true);

	lexer.curr_check(TokenType::CLOSE_BRACKET);
	lexer.eat();

	return While(lexer.loc, cond_expr, parse_stmts(lexer));
}

For parse_for(Lexer& lexer)
{
	// initialization

	lexer.expect(TokenType::OPEN_BRACKET);
	auto var_init_name = lexer.expect(TokenType::VARIABLE).str;

	if (!lexer.eat().is_type())
		throw NIGHT_CREATE_FATAL("expected variable type after '" + var_init_name + "'");

	auto var_init = parse_var_init(lexer, var_init_name);
	
	// condition

	lexer.eat();
	auto cond_expr = parse_expr(lexer, false, true);
	lexer.curr_check(TokenType::SEMICOLON);

	// assignment

	std::string var_assign_name = lexer.expect(TokenType::VARIABLE).str;

	lexer.eat();

	auto var_assign = parse_var_assign(lexer, var_assign_name);
	lexer.curr_check(TokenType::CLOSE_BRACKET);

	// body

	lexer.eat();

	auto stmts = parse_stmts(lexer);
	stmts.push_back(std::make_shared<VariableAssign>(var_assign));

	return For(lexer.loc, var_init, cond_expr, stmts);
}

Function parse_func(Lexer& lexer)
{
	std::string func_name = lexer.expect(TokenType::VARIABLE).str;
	lexer.expect(TokenType::OPEN_BRACKET);

	// parse function header

	std::vector<std::string> param_names;
	std::vector<std::string> param_types;

	lexer.eat();
	while (true)
	{
		if (lexer.curr().type == TokenType::CLOSE_BRACKET)
			break;

		lexer.curr_check(TokenType::VARIABLE);
		param_names.push_back(lexer.curr().str);

		if (!lexer.eat().is_type())
			throw NIGHT_CREATE_FATAL("expected type after variable name in function parameter list, found '" + lexer.curr().str + "'");

		param_types.push_back(lexer.curr().str);

		lexer.eat();

		if (lexer.curr().type == TokenType::CLOSE_BRACKET)
			break;

		lexer.curr_check(TokenType::COMMA);
		lexer.eat();
	}

	auto rtn_type = lexer.eat();

	if (!rtn_type.is_type() && rtn_type.type != TokenType::VOID)
		throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected return type");

	lexer.expect(TokenType::OPEN_CURLY);

	auto body = parse_stmts(lexer);

	return Function(lexer.loc, func_name, param_names, param_types, rtn_type.str, body);
}

Return parse_return(Lexer& lexer)
{
	auto const& expr = parse_expr(lexer, true, false);
	lexer.curr_check(TokenType::SEMICOLON);

	lexer.eat();

	return Return(lexer.loc, expr);
}

expr::expr_p parse_expr(Lexer& lexer, bool eat_tok, bool err_on_empty)
{
	if (eat_tok)
		lexer.eat();

	expr::expr_p head(nullptr);
	bool allow_unary_next = true;
	bool was_variable = false;
	bool was_sub = false;

	while (true)
	{
		expr::expr_p node(nullptr);

		auto curr = lexer.curr();

		if (curr.is_type())
			curr.type = TokenType::VARIABLE;

		switch (curr.type)
		{
		case TokenType::BOOL_LIT:
		{
			node = std::make_shared<expr::Value>(lexer.loc, ValueType::BOOL, lexer.curr().str);
			allow_unary_next = false;
			was_variable = false;
			was_sub = false;
			break;
		}
		case TokenType::CHAR_LIT:
		{
			node = std::make_shared<expr::Value>(lexer.loc, ValueType::CHAR, lexer.curr().str);
			allow_unary_next = false;
			was_variable = false;
			was_sub = false;
			break;
		}
		case TokenType::INT_LIT:
		{
			node = std::make_shared<expr::Value>(lexer.loc, ValueType::INT, lexer.curr().str);
			allow_unary_next = false;
			was_variable = false;
			was_sub = false;
			break;
		}
		case TokenType::FLOAT_LIT:
		{
			node = std::make_shared<expr::Value>(lexer.loc, ValueType::FLOAT, lexer.curr().str);
			allow_unary_next = false;
			was_variable = false;
			was_sub = false;
			break;
		}
		case TokenType::STRING_LIT:
		{
			node = std::make_shared<expr::Value>(lexer.loc, ValueType::STR, lexer.curr().str);
			allow_unary_next = false;
			was_variable = true;
			was_sub = false;
			break;
		}
		case TokenType::VARIABLE:
		{
			auto var_name = lexer.curr().str;

			if (lexer.peek().type == TokenType::OPEN_BRACKET)
			{
				lexer.eat();
				node = std::make_shared<expr::FunctionCall>(parse_func_call(lexer, var_name));
			}
			else
				node = std::make_shared<expr::Variable>(lexer.loc, var_name);

			allow_unary_next = false;
			was_variable = true;
			was_sub = false;
			break;
		}
		case TokenType::OPEN_SQUARE:
		{
			if (was_variable)
			{
				auto index_expr = parse_expr(lexer, true, true);
				lexer.curr_check(TokenType::CLOSE_SQUARE);

				node = std::make_shared<expr::BinaryOp>(lexer.loc, expr::BinaryOpType::SUBSCRIPT);
				node->insert_node(index_expr);

				allow_unary_next = false;
				was_sub = true;
				was_variable = true;
			}
			else
			{
				std::vector<expr::expr_p> arr;
				while (true)
				{
					auto elem = parse_expr(lexer, true, false);
					if (!elem)
					{
						lexer.curr_check(TokenType::CLOSE_SQUARE);
						break;
					}

					arr.push_back(elem);

					if (lexer.curr().type == TokenType::CLOSE_SQUARE)
						break;

					lexer.curr_check(TokenType::COMMA);
				}

				node = std::make_shared<expr::Array>(lexer.loc, arr);
				allow_unary_next = false;
				was_variable = false;
			}

			break;
		}
		case TokenType::UNARY_OP:
		{
			node = std::make_shared<expr::UnaryOp>(lexer.loc, lexer.curr().str);
			allow_unary_next = true;
			was_sub = false;
			break;
		}
		case TokenType::BINARY_OP:
		{
			if (allow_unary_next && lexer.curr().str == "-")
			{
				node = std::make_shared<expr::UnaryOp>(lexer.loc, lexer.curr().str);
			}
			else
			{
				node = std::make_shared<expr::BinaryOp>(lexer.loc, lexer.curr().str);
				allow_unary_next = true;
			}
			was_variable = false;
			was_sub = false;

			break;
		}
		case TokenType::OPEN_BRACKET:
		{
			node = parse_expr(lexer, true, err_on_empty);

			if (lexer.curr().type != TokenType::CLOSE_BRACKET)
				throw NIGHT_CREATE_FATAL("missing closing bracket in expression");

			node->guard = true;
			allow_unary_next = false;
			was_variable = false;
			was_sub = false;
			break;
		}
		default:
		{
			if (err_on_empty && !head)
				throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected expression");

			return head;
		}
		}

		if (!head)
			head = node;
		else
			head->insert_node(node, &head);

		lexer.eat();
	}
}