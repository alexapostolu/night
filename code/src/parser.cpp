#include "parser.hpp"
#include "lexer.hpp"
#include "parser_scope.hpp"
#include "bytecode.hpp"
#include "interpreter_scope.hpp"
#include "ast/ast.hpp"
#include "ast/expression.hpp"
#include "value.hpp"
#include "error.hpp"

#include <iostream>
#include <algorithm>
#include <variant>
#include <string>
#include <assert.h>
#include <unordered_map>

std::vector<std::shared_ptr<AST>> parse_stmts(Lexer& lexer, ParserScope& upper_scope, bool* curly_enclosed)
{
	ParserScope scope{ upper_scope.vars };

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

		while (lexer.eat().type != TokenType::CLOSE_BRACKET)
		{
			if (lexer.curr().type == TokenType::END_OF_FILE)
				throw NIGHT_CREATE_FATAL("missing closing curly bracket");

			if (!conditional &&
				(lexer.curr().type == TokenType::ELIF || lexer.curr().type == TokenType::ELSE))
				throw NIGHT_CREATE_FATAL("expected 'elif' and 'else' statement to come after 'if' or 'elif' statement");

			conditional = lexer.curr().type == TokenType::IF ||
						  lexer.curr().type == TokenType::ELIF;

			block.push_back(parse_stmt(lexer, scope));
		}

		return block;
	}
	case TokenType::END_OF_FILE:
		return {};
	default:
		return { parse_stmt(lexer, scope) };
	}
}

std::shared_ptr<AST> parse_stmt(Lexer& lexer, ParserScope& scope)
{
	assert(lexer.curr().type != TokenType::CLOSE_CURLY);
	assert(lexer.curr().type != TokenType::END_OF_FILE);

	switch (lexer.curr().type)
	{
	case TokenType::VARIABLE: return parse_var(lexer, scope);
	case TokenType::IF:
	case TokenType::ELIF:
	case TokenType::ELSE:	  return std::make_shared<Conditional>(parse_if(lexer, scope));
	case TokenType::FOR:	  return std::make_shared<For>(parse_for(lexer, scope));
	case TokenType::WHILE:	  return std::make_shared<While>(parse_while(lexer, scope));
	case TokenType::DEF:	  return std::make_shared<Function>(parse_func(lexer, scope));
	case TokenType::RETURN:	  return std::make_shared<Return>(parse_return(lexer, scope));

	default: throw NIGHT_CREATE_FATAL("unknown syntax '" + lexer.curr().str + "'");
	}
}

std::shared_ptr<AST> parse_var(Lexer& lexer, ParserScope& scope)
{
	assert(lexer.curr().type == TokenType::VARIABLE);

	std::string var_name = lexer.curr().str;
	
	lexer.eat();

	if (lexer.curr().is_type())
	{
		return std::make_shared<VariableInit>(parse_var_init(lexer, scope, var_name));
	}
	else if (lexer.curr().type == TokenType::ASSIGN)
	{
		auto ast = parse_var_assign(lexer, scope, var_name);
		if (lexer.curr().type != TokenType::SEMICOLON)
			throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected semicolon after variable assignment");

		return std::make_shared<VariableAssign>(ast);
	}
	else if (lexer.curr().type == TokenType::OPEN_BRACKET)
	{
		return std::make_shared<FunctionCall>(parse_func_call(lexer, scope, var_name));
	}
	else
	{
		throw NIGHT_CREATE_FATAL("expected variable type or assignment after variable name '" + var_name + "'");
	}
}

VariableInit parse_var_init(Lexer& lexer, ParserScope& scope, std::string const& var_name)
{
	assert(lexer.curr().is_type());

	if (scope.vars.contains(var_name))
		throw NIGHT_CREATE_FATAL("variable '" + var_name + "' is already defined");

	auto var_type = token_var_type_to_val_type(lexer.curr().str);
	std::shared_ptr<expr::Expression> var_expr =
		std::make_shared<expr::Value>(lexer.loc, val::Value{ (val::value_t)val::ValueType::U_INT, 0 });

	lexer.eat();

	if (lexer.curr().type == TokenType::ASSIGN && lexer.curr().str == "=")
	{
		// case:
		//   var int = [expression];

		var_expr = parse_expr(lexer, scope,
			"found '" + lexer.curr().str + "', expected expression after assignment");

		if (lexer.curr().type != TokenType::SEMICOLON)
			throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected semicolon after variable initialization");

		auto expr_type = var_expr->type_check(scope);

		if (expr_type.has_value() && !compare_value_t(var_type, *expr_type))
			NIGHT_CREATE_MINOR("variable '" + var_name + "' of type '" + val_type_to_str(var_type) +
				"' can not be initialized with expression of type '" + val_type_to_str(*expr_type) + "'");
	}
	else if (lexer.curr().type == TokenType::ASSIGN && lexer.curr().str != "=")
	{
		throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected assignment '='")
	}
	else if (lexer.curr().type != TokenType::SEMICOLON)
	{
		throw NIGHT_CREATE_FATAL("expected semicolon or assignment after variable type");
	}

	auto msg = scope.create_variable(var_name, var_type);
	if (!msg.empty())
		NIGHT_CREATE_MINOR(msg);

	return VariableInit(lexer.loc, scope.vars[var_name].id, var_expr);
}

VariableAssign parse_var_assign(Lexer& lexer, ParserScope& scope, std::string const& var_name)
{
	assert(lexer.curr().type == TokenType::ASSIGN);

	if (!scope.vars.contains(var_name))
		NIGHT_CREATE_MINOR("variable '" + var_name + "' is undefined");
	
	std::string assign_op = lexer.curr().str;

	auto expr = parse_expr(lexer, scope,
		"found '" + lexer.curr().str + "', expected expression after assignment");

	if (lexer.curr().type != TokenType::SEMICOLON)
		throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected semicolon after expression");

	auto expr_type = expr->type_check(scope);

	if (expr_type.has_value() && assign_op != "=" && is_object_t(*expr_type))
		NIGHT_CREATE_MINOR("assignment '" + assign_op + "' can not be used on expression of type '" + val_type_to_str(*expr_type) + "'");

	if (expr_type.has_value() && compare_value_t(scope.vars.at(var_name).type, *expr_type))
		NIGHT_CREATE_MINOR("variable '" + var_name + "' of type '" + val_type_to_str(scope.vars.at(var_name).type) +
			"' can not be initialized with expression of type '" + val_type_to_str(*expr_type) + "'");

	return VariableAssign(lexer.loc, scope.vars[var_name].id, expr, assign_op);
}

FunctionCall parse_func_call(Lexer& lexer, ParserScope& scope, std::string const& func_name)
{
	assert(lexer.curr().type == TokenType::OPEN_BRACKET);
	
	std::vector<std::shared_ptr<expr::Expression>> param_expr;

	while (lexer.curr().type != TokenType::CLOSE_BRACKET)
	{
		auto expr = parse_expr(lexer, scope,
			"expected expression in function call");
	}

	if (param_expr.size() != ParserScope::funcs[func_name].param_names.size())
		throw NIGHT_CREATE_FATAL("function call has " + std::to_string(param_expr.size()) + " parameters, expected " +
			std::to_string(ParserScope::funcs[func_name].param_names.size()) + " parameters");

	for (std::size_t i = 0; i < param_expr.size(); ++i)
	{
		auto type = param_expr[i]->type_check(scope);
		if (type.has_value() && compare_value_t(*type, ParserScope::funcs[func_name].param_types[i]))
			NIGHT_CREATE_MINOR("found type '" + val_type_to_str(*type) + "', expected type '" + val_type_to_str(ParserScope::funcs[func_name].param_types[i]) + "'");
	}

	std::vector<bytecode_t> param_ids;
	for (auto const& param_name : ParserScope::funcs[func_name].param_names)
		param_ids.push_back(scope.vars[param_name].id);

	return FunctionCall(lexer.loc, func_name, param_expr, param_ids);
}

Conditional parse_if(Lexer& lexer, ParserScope& scope)
{
	assert(lexer.curr().type == TokenType::IF);

	std::vector<
		std::pair<std::shared_ptr<expr::Expression>, AST_Block>
	> conditionals;

	do {
		std::shared_ptr<expr::Expression> cond_expr =
			std::make_shared<expr::Value>(lexer.loc, val::Value{ (val::value_t)val::ValueType::BOOL, 1 });

		// parse condition
		if (lexer.curr().type != TokenType::ELSE)
		{
			lexer.expect(TokenType::OPEN_BRACKET);

			cond_expr = parse_expr(lexer, scope,
				"found '" + lexer.curr().str + "', expected expression after opening bracket");

			if (lexer.curr().type != TokenType::CLOSE_BRACKET)
				throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected closing bracket after condition");

			auto cond_type = cond_expr->type_check(scope);

			if (cond_type.has_value() && is_object_t(*cond_type))
				NIGHT_CREATE_MINOR("condition is type '" + val_type_to_str(*cond_type) + "', expected type 'bool', 'char', or 'int'");
		}

		conditionals.push_back({ cond_expr, parse_stmts(lexer, scope) });

		lexer.eat();
	} while (lexer.curr().type == TokenType::IF	  ||
			 lexer.curr().type == TokenType::ELIF ||
			 lexer.curr().type == TokenType::ELSE);

	return Conditional(lexer.loc, conditionals);
}

While parse_while(Lexer& lexer, ParserScope& scope)
{
	assert(lexer.curr().type == TokenType::WHILE);

	lexer.expect(TokenType::OPEN_BRACKET);

	auto cond_expr = parse_expr(lexer, scope,
		"found '" + lexer.curr().str + "', expected expression after opening bracket");

	if (lexer.curr().type != TokenType::CLOSE_BRACKET)
		throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected closing bracket after condition");

	auto cond_type = cond_expr->type_check(scope);

	if (cond_type.has_value() && is_object_t(*cond_type))
		NIGHT_CREATE_MINOR("condition of type '" + val_type_to_str(*cond_type) + "', expected type 'bool', 'char', or 'int'");

	return While(lexer.loc, cond_expr, parse_stmts(lexer, scope));
}

For parse_for(Lexer& lexer, ParserScope& scope)
{
	assert(lexer.curr().type == TokenType::FOR);

	// initialization

	lexer.expect(TokenType::OPEN_BRACKET);
	auto var_init_name = lexer.expect(TokenType::VARIABLE).str;

	auto var_init = parse_var_init(lexer, scope, var_init_name);
	
	// condition

	auto cond_expr = parse_expr(lexer, scope,
		"expected expression after condition in for loop");

	if (lexer.curr().type != TokenType::SEMICOLON)
		throw NIGHT_CREATE_FATAL("expected semicolon after condition");

	auto cond_type = cond_expr->type_check(scope);

	if (cond_type.has_value() && is_object_t(*cond_type))
		NIGHT_CREATE_MINOR("condition of type '" + val_type_to_str(*cond_type) + "', expected type 'bool', 'char', or 'int'");

	// assignment

	auto var_assign_name = lexer.expect(TokenType::VARIABLE).str;
	lexer.expect(TokenType::ASSIGN);

	auto var_assign = parse_var_assign(lexer, scope, var_assign_name);

	return For(lexer.loc, var_init, cond_expr, var_assign, parse_stmts(lexer, scope));
}

Function parse_func(Lexer& lexer, ParserScope& scope)
{
	std::string func_name = lexer.expect(TokenType::VARIABLE).str;

	lexer.expect(TokenType::OPEN_BRACKET);
	
	ParserScope::funcs[func_name] = {};

	std::vector<std::string> param_names;

	while (lexer.curr().type != TokenType::CLOSE_BRACKET)
	{
		auto var_name = lexer.expect(TokenType::VARIABLE).str;
		param_names.push_back(var_name);

		if (!lexer.eat().is_type())
			throw NIGHT_CREATE_FATAL("expected type after variable name in function parameter list");

		auto var_type = token_var_type_to_val_type(lexer.curr().str);

		ParserScope::funcs[func_name].param_types.push_back(var_type);

		lexer.eat();

		if (lexer.curr().type == TokenType::CLOSE_BRACKET)
			break;
		if (lexer.curr().type != TokenType::COMMA)
			throw NIGHT_CREATE_FATAL("expected comma after variable type");

		lexer.eat();
	}

	ParserScope::curr_func = ParserScope::funcs.find(func_name);

	std::vector<bytecode_t> param_ids(param_names.size());
	std::transform(std::begin(param_names), std::end(param_names), std::begin(param_ids),
		[&scope](auto const& name) { return scope.vars[name].id; });

	Function func(lexer.loc, func_name, param_ids, parse_stmts(lexer, scope));

	ParserScope::curr_func = std::end(ParserScope::funcs);

	return func;
}

Return parse_return(Lexer& lexer, ParserScope& scope)
{
	auto expr = parse_expr(lexer, scope,
		"expected expression after return");
	if (lexer.curr().type != TokenType::SEMICOLON)
		throw NIGHT_CREATE_FATAL("expected semicolon after expression");

	auto expr_type = expr->type_check(scope);
	
	assert(ParserScope::curr_func != std::end(ParserScope::funcs));

	if (!ParserScope::curr_func->second.rtn_type.has_value())
		ParserScope::curr_func->second.rtn_type = expr_type;
	else if (expr_type.has_value() && !compare_value_t(*ParserScope::curr_func->second.rtn_type, *expr_type))
		NIGHT_CREATE_MINOR("return is type '" + val_type_to_str(*expr_type) + "', expected type '" + val_type_to_str(*ParserScope::curr_func->second.rtn_type) + "'");

	return Return(lexer.loc, expr);
}

std::shared_ptr<expr::Expression> parse_expr(Lexer& lexer, ParserScope const& scope, std::string const& err_msg, bool bracket)
{
	std::shared_ptr<expr::Expression> head(nullptr);

	while (true)
	{
		std::shared_ptr<expr::Expression> node(nullptr);
		switch (lexer.eat().type)
		{
		case TokenType::CHAR_LIT:
		{
			node = std::make_shared<expr::Expression>(val::ValueType::CHAR, lexer.curr().str[0]);
			break;
		}
		case TokenType::INT_LIT:
		{
			if (std::stoi(lexer.curr().str) > 0)
				node = std::make_shared<expr::Expression>(val::ValueType::U_INT, std::stoi(lexer.curr().str));
			else
				node = std::make_shared<expr::Expression>(val::ValueType::S_INT, std::stoi(lexer.curr().str));
			break;
		}
		case TokenType::VARIABLE:
		{
			node = std::make_shared<expr::Variable>(lexer.curr().str);
			break;
		}
		case TokenType::UNARY_OP:
		{
			node = std::make_shared<expr::UnaryOp>(lexer.curr().str);
			break;
		}
		case TokenType::BINARY_OP:
		{
			node = std::make_shared<expr::BinaryOp>(lexer.curr().str);
			break;
		}
		case TokenType::OPEN_BRACKET:
		{
			node = parse_expr(lexer, scope, err_msg, true);

			if (lexer.curr().type != TokenType::CLOSE_BRACKET)
				throw NIGHT_CREATE_FATAL("missing closing bracket in expression");

			node->guard = true;
			break;
		}
		case TokenType::CLOSE_BRACKET:
		{
			if (!bracket)
				throw NIGHT_CREATE_FATAL("missing opening bracket");
		}
		default:
		{
			if (!err_msg.empty() && !head)
				throw NIGHT_CREATE_FATAL(err_msg);

			return head;
		}
		}

		if (!head)
			head = node;
		else
			head->insert_node(node, &head);
	}
}

val::value_t token_var_type_to_val_type(std::string const& type)
{
	if (type == "int8")
		return (val::value_t)val::ValueType::S_INT;
	else if (type == "int16")
		return (val::value_t)val::ValueType::S_INT;
	else if (type == "int32")
		return (val::value_t)val::ValueType::S_INT;
	else if (type == "int64")
		return (val::value_t)val::ValueType::S_INT;
	else if (type == "uint8")
		return (val::value_t)val::ValueType::U_INT;
	else if (type == "uint16")
		return (val::value_t)val::ValueType::U_INT;
	else if (type == "uint32")
		return (val::value_t)val::ValueType::U_INT;
	else if (type == "uint64")
		return (val::value_t)val::ValueType::U_INT;
	else
		night::throw_unhandled_case(type);
}