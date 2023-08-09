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

#include <algorithm>
#include <variant>
#include <string>
#include <assert.h>
#include <unordered_map>

std::vector<std::shared_ptr<AST>> parse_stmts(Lexer& lexer, bool* curly_enclosed)
{
	bool conditional = false;

	// two cases:
	//   { stmt1; stmt2; ... }
	//   stmt1;
	switch (lexer.eat().type)
	{
	case TokenType::OPEN_CURLY:
	{
		*curly_enclosed = true;

		std::vector<std::shared_ptr<AST>> block;

		while (lexer.eat().type != TokenType::CLOSE_CURLY)
		{
			if (lexer.curr().type == TokenType::END_OF_FILE)
				throw NIGHT_CREATE_FATAL("missing closing curly bracket");

			block.push_back(parse_stmt(lexer));
		}

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
	assert(lexer.curr().type != TokenType::CLOSE_CURLY);
	assert(lexer.curr().type != TokenType::END_OF_FILE);

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
	
	lexer.eat();

	std::shared_ptr<AST> ast;

	if (lexer.curr().is_type())
	{
		return std::make_shared<VariableInit>(parse_var_init(lexer, var_name));
	}
	else if (lexer.curr().type == TokenType::ASSIGN)
	{
		return std::make_shared<VariableAssign>(parse_var_assign(lexer, var_name));
	}
	else if (lexer.curr().type == TokenType::OPEN_BRACKET)
	{
		ast = std::make_shared<FunctionCall>(parse_func_call(lexer, var_name));
	}
	else
	{
		throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected variable type or assignment after variable name '" + var_name + "'");
	}

	lexer.expect(TokenType::SEMICOLON);
	return ast;
}

VariableInit parse_var_init(Lexer& lexer, std::string const& var_name)
{
	std::string var_type = lexer.curr().str;

	// default value
	std::shared_ptr<expr::Expression> var_expr =
		std::make_shared<expr::Value>(lexer.loc, token_var_type_to_val_type(var_type), "0");

	lexer.eat();

	if (lexer.curr().type == TokenType::ASSIGN && lexer.curr().str == "=")
	{
		var_expr = parse_expr(lexer, true);
		lexer.curr_expect(TokenType::SEMICOLON);
	}
	else if (lexer.curr().type == TokenType::ASSIGN && lexer.curr().str != "=")
	{
		throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected assignment '='")
	}
	else if (lexer.curr().type != TokenType::SEMICOLON)
	{
		throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "' expected semicolon or assignment after variable type");
	}

	return VariableInit(lexer.loc, var_name, var_type, var_expr);
}

VariableAssign parse_var_assign(Lexer& lexer, std::string const& var_name)
{
	auto assign_op = lexer.curr().str;
	auto expr = parse_expr(lexer, true);

	lexer.curr_expect(TokenType::SEMICOLON);

	return VariableAssign(lexer.loc, var_name, assign_op, expr);
}

FunctionCall parse_func_call(Lexer& lexer, std::string const& func_name)
{
	// parse argument expressions and types

	std::vector<expr::expr_p> arg_exprs;

	while (lexer.curr().type != TokenType::CLOSE_BRACKET)
	{
		auto expr = parse_expr(lexer, false);

		if (!expr)
		{
			lexer.curr_expect(TokenType::CLOSE_BRACKET);
			break;
		}

		arg_exprs.push_back(expr);

		if (lexer.curr().type == TokenType::CLOSE_BRACKET)
			break;

		lexer.curr_expect(TokenType::COMMA);
		lexer.eat();
	}

	// match the function call with its ParserScope function

	return FunctionCall(lexer.loc, func_name, arg_exprs);
}

Conditional parse_if(Lexer& lexer)
{
	std::vector<std::pair<expr::expr_p, AST_Block>> conditionals;

	do {
		// default for else statements
		std::shared_ptr<expr::Expression> cond_expr =
			std::make_shared<expr::Value>(lexer.loc, (value_t)ValueType::BOOL, "1");

		// parse condition
		if (lexer.curr().type != TokenType::ELSE)
		{
			lexer.expect(TokenType::OPEN_BRACKET);

			cond_expr = parse_expr(lexer, true);
			lexer.curr_expect(TokenType::CLOSE_BRACKET);
		}

		conditionals.push_back({ cond_expr, parse_stmts(lexer) });

		lexer.eat();
	} while (lexer.curr().type == TokenType::IF	  ||
			 lexer.curr().type == TokenType::ELIF ||
			 lexer.curr().type == TokenType::ELSE);

	return Conditional(lexer.loc, conditionals);
}

While parse_while(Lexer& lexer)
{
	lexer.expect(TokenType::OPEN_BRACKET);

	auto cond_expr = parse_expr(lexer, true);

	lexer.curr_expect(TokenType::CLOSE_BRACKET);

	return While(lexer.loc, cond_expr, parse_stmts(lexer));
}

For parse_for(Lexer& lexer)
{
	// initialization

	lexer.expect(TokenType::OPEN_BRACKET);
	auto var_init_name = lexer.expect(TokenType::VARIABLE).str;

	auto var_init = parse_var_init(lexer, var_init_name);
	
	// condition

	auto cond_expr = parse_expr(lexer, true);

	lexer.curr_expect(TokenType::SEMICOLON);

	// assignment

	auto var_assign_name = lexer.expect(TokenType::VARIABLE).str;
	lexer.expect(TokenType::ASSIGN);

	auto var_assign = parse_var_assign(lexer, var_assign_name);

	// body

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

	while (lexer.curr().type != TokenType::CLOSE_BRACKET)
	{
		param_names.push_back(lexer.expect(TokenType::VARIABLE).str);

		if (!lexer.eat().is_type())
			throw NIGHT_CREATE_FATAL("expected type after variable name in function parameter list, found '" + lexer.curr().str + "'");

		param_types.push_back(lexer.curr().str);

		lexer.eat();

		if (lexer.curr().type == TokenType::CLOSE_BRACKET)
			break;

		lexer.curr_expect(TokenType::COMMA);
		lexer.eat();
	}

	auto rtn_type = lexer.eat();

	if (!rtn_type.is_type() && rtn_type.type != TokenType::VOID)
		throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected return type");

	return Function(lexer.loc, func_name, param_names, param_types, rtn_type.str, parse_stmts(lexer));;
}

Return parse_return(Lexer& lexer)
{
	auto expr = parse_expr(lexer, false);
	lexer.curr_expect(TokenType::SEMICOLON);

	return Return(lexer.loc, expr);
}

std::shared_ptr<expr::Expression> parse_expr(Lexer& lexer, bool err_on_empty)
{
	std::shared_ptr<expr::Expression> head(nullptr);
	Token next{ .str = "" };
	bool allow_unary_next = true;

	while (true)
	{
		std::shared_ptr<expr::Expression> node(nullptr);

		Token comp;
		if (next.str.empty())
		{
			comp = lexer.eat();
		}
		else
		{
			comp = next;
			next.str = "";
		}

		switch (comp.type)
		{
		case TokenType::CHAR_LIT:
		{
			node = std::make_shared<expr::Value>(lexer.loc, (value_t)ValueType::CHAR, lexer.curr().str);
			allow_unary_next = false;
			break;
		}
		case TokenType::INT_LIT:
		{
			node = std::make_shared<expr::Value>(lexer.loc, (value_t)ValueType::INT, lexer.curr().str);
			allow_unary_next = false;
			break;
		}
		case TokenType::FLOAT_LIT:
		{
			node = std::make_shared<expr::Value>(lexer.loc, (value_t)ValueType::FLOAT, lexer.curr().str);
			allow_unary_next = false;
			break;
		}
		case TokenType::STRING_LIT:
		{
			node = std::make_shared<expr::Value>(lexer.loc, (value_t)ValueType::STRING, lexer.curr().str);
			allow_unary_next = false;
			break;
		}
		case TokenType::VARIABLE:
		{
			auto var_name = lexer.curr().str;
			next = lexer.eat();
			if (next.type == TokenType::OPEN_BRACKET)
			{
				node = std::make_shared<FunctionCall>(parse_func_call(lexer, var_name));
				next.str = "";
			}
			else
			{
				node = std::make_shared<expr::Variable>(lexer.loc, var_name);
			}

			allow_unary_next = false;
			break;
		}
		case TokenType::UNARY_OP:
		{
			node = std::make_shared<expr::UnaryOp>(lexer.loc, lexer.curr().str);
			allow_unary_next = true;
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

			break;
		}
		case TokenType::OPEN_BRACKET:
		{
			node = parse_expr(lexer, err_on_empty);

			if (lexer.curr().type != TokenType::CLOSE_BRACKET)
				throw NIGHT_CREATE_FATAL("missing closing bracket in expression");

			node->guard = true;
			allow_unary_next = false;
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
	}
}