#include "parser.hpp"
#include "lexer.hpp"
#include "parser_scope.hpp"
#include "bytecode.hpp"
#include "expression.hpp"
#include "interpreter_scope.hpp"
#include "value.hpp"
#include "error.hpp"
#include "ast.hpp"

#include <iostream>
#include <variant>
#include <string>
#include <assert.h>
#include <unordered_map>
#include <limits>

std::vector<std::shared_ptr<AST>> parse_stmts(Lexer& lexer, ParserScope& upper_scope, func_container* funcs, bool* curly_enclosed)
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

			block.push_back(parse_stmt(lexer, scope, *funcs));
		}

		return block;
	}
	case TokenType::END_OF_FILE:
		return {};
	default:
		return { parse_stmt(lexer, scope, *funcs) };
	}
}

std::shared_ptr<AST> parse_stmt(Lexer& lexer, ParserScope& scope, func_container& funcs)
{
	assert(lexer.curr().type != TokenType::CLOSE_CURLY && "should be handled by caller");
	assert(lexer.curr().type != TokenType::END_OF_FILE && "should be handled by caller");

	switch (lexer.curr().type)
	{
	case TokenType::VARIABLE: return parse_var(lexer, scope);
	case TokenType::IF:
	case TokenType::ELIF:	  return std::make_shared<If>(parse_if(lexer, scope, false));
	case TokenType::ELSE:	  return std::make_shared<If>(parse_if(lexer, scope, true));
	case TokenType::FOR:	  return std::make_shared<For>(parse_for(lexer, scope));
	case TokenType::WHILE:	  return std::make_shared<While>(parse_while(lexer, scope));
	case TokenType::DEF:	  parse_func(lexer, scope); return nullptr;
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

	return VariableInit(lexer.loc, var_name, var_expr);
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

	return VariableAssign(lexer.loc, var_name, expr, assign_op);
}

If parse_if(Lexer& lexer, ParserScope& scope, bool is_else)
{
	assert(lexer.curr().type == TokenType::IF);

	std::shared_ptr<expr::Expression> cond_expr =
		std::make_shared<expr::Value>(lexer.loc, val::Value{ (val::value_t)val::ValueType::BOOL, 1 });

	// parse condition
	if (!is_else)
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

	return If(lexer.loc, cond_expr, parse_stmts(lexer, scope));
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

	Function func(lexer.loc, func_name, param_names, parse_stmts(lexer, scope));

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







BytecodeType token_var_type_to_bytecode(std::string const& type)
{
	if (type == "int8")
		return BytecodeType::S_INT1;
	else if (type == "int16")
		return BytecodeType::S_INT2;
	else if (type == "int32")
		return BytecodeType::S_INT4;
	else if (type == "int64")
		return BytecodeType::S_INT8;
	else if (type == "uint8")
		return BytecodeType::U_INT1;
	else if (type == "uint16")
		return BytecodeType::U_INT2;
	else if (type == "uint32")
		return BytecodeType::U_INT4;
	else if (type == "uint64")
		return  BytecodeType::U_INT8;
	else
		night::unhandled_case(type);
}

val::value_t token_var_type_to_val_type(std::string const& type)
{
	if (type == "int8")
		return (value_t)ValueType::S_INT;
	else if (type == "int16")
		return (value_t)ValueType::S_INT;
	else if (type == "int32")
		return (value_t)ValueType::S_INT;
	else if (type == "int64")
		return (value_t)ValueType::S_INT;
	else if (type == "uint8")
		return (value_t)ValueType::U_INT;
	else if (type == "uint16")
		return (value_t)ValueType::U_INT;
	else if (type == "uint32")
		return (value_t)ValueType::U_INT;
	else if (type == "uint64")
		return (value_t)ValueType::U_INT;
	else
		night::unhandled_case(type);
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
			node = std::make_shared<expr::Expression>(val::ValueType::INT, std::stoi(lexer.curr().str));
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