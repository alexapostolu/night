#include "parser.hpp"
#include "lexer.hpp"
#include "parser_scope.hpp"
#include "bytecode.hpp"
#include "interpreter_scope.hpp"
#include "ast/ast.hpp"
#include "ast/expression.hpp"
#include "value_type.hpp"
#include "error.hpp"
#include "debug.hpp"

#include <algorithm>
#include <variant>
#include <string>
#include <assert.h>
#include <unordered_map>

std::vector<std::shared_ptr<AST>> parse_stmts(Lexer& lexer, ParserScope& scope, bool* curly_enclosed)
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
		throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected variable type or assignment after variable name '" + var_name + "'");
	}
}

VariableInit parse_var_init(Lexer& lexer, ParserScope& scope, std::string const& var_name)
{
	assert(lexer.curr().is_type());

	if (scope.vars.contains(var_name))
		throw NIGHT_CREATE_FATAL("variable '" + var_name + "' is already defined");

	auto var_type = token_var_type_to_val_type(lexer.curr().str);
	std::shared_ptr<expr::Expression> var_expr =
		std::make_shared<expr::Value>(lexer.loc, (value_t)ValueType::INT, "0");

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
			NIGHT_CREATE_MINOR("variable '" + var_name + "' of type '" + night::to_str(var_type) +
				"' can not be initialized with expression of type '" + night::to_str(*expr_type) + "'");
	}
	else if (lexer.curr().type == TokenType::ASSIGN && lexer.curr().str != "=")
	{
		throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected assignment '='")
	}
	else if (lexer.curr().type != TokenType::SEMICOLON)
	{
		throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "' expected semicolon or assignment after variable type");
	}

	auto [msg, id] = scope.create_variable(var_name, var_type);
	if (!msg.empty())
		NIGHT_CREATE_MINOR(msg);

	return VariableInit(lexer.loc, id, var_expr);
}

VariableAssign parse_var_assign(Lexer& lexer, ParserScope& scope, std::string const& var_name)
{
	assert(lexer.curr().type == TokenType::ASSIGN);

	if (!scope.vars.contains(var_name))
		throw NIGHT_CREATE_FATAL("variable '" + var_name + "' is undefined");
	
	std::string assign_op = lexer.curr().str;

	auto expr = parse_expr(lexer, scope,
		"found '" + lexer.curr().str + "', expected expression after assignment");

	if (lexer.curr().type != TokenType::SEMICOLON)
		throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected semicolon after expression");

	auto expr_type = expr->type_check(scope);

	if (expr_type.has_value() && assign_op != "=" && is_object_t(*expr_type))
		NIGHT_CREATE_MINOR("assignment '" + assign_op + "' can not be used on expression of type '" + night::to_str(*expr_type) + "'");

	if (expr_type.has_value() && compare_value_t(scope.vars.at(var_name).type, *expr_type))
		NIGHT_CREATE_MINOR("variable '" + var_name + "' of type '" + night::to_str(scope.vars.at(var_name).type) +
			"' can not be initialized with expression of type '" + night::to_str(*expr_type) + "'");

	return VariableAssign(lexer.loc, scope.vars[var_name].id, expr, assign_op);
}

FunctionCall parse_func_call(Lexer& lexer, ParserScope const& scope, std::string const& func_name)
{
	assert(lexer.curr().type == TokenType::OPEN_BRACKET);
	
	// parse argument expressions and types

	std::vector<std::shared_ptr<expr::Expression>> arg_exprs;
	std::vector<value_t> arg_types;

	while (lexer.curr().type != TokenType::CLOSE_BRACKET)
	{
		auto expr = parse_expr(lexer, scope,
			"found '" + lexer.curr().str + "' in function call '" + func_name +
			"', expected expression");

		arg_exprs.push_back(expr);
		arg_types.push_back(*expr->type_check(scope));

		if (lexer.curr().type == TokenType::CLOSE_BRACKET)
			break;

		if (lexer.curr().type != TokenType::COMMA)
			throw NIGHT_CREATE_FATAL(
				"found '" + lexer.curr().str + "' in function call '" + func_name + "', "
				"expected comma or closing bracket");

		lexer.eat();
	}

	// match the function call with its ParserScope function

	auto [func, range_end] = ParserScope::funcs.equal_range(func_name);

	if (func == range_end)
		throw NIGHT_CREATE_FATAL("function '" + func_name + "' is not defined");

	for (; func != range_end; ++func)
	{
		if (std::equal(std::begin(arg_types), std::end(arg_types),
					   std::begin(func->second.param_types), std::end(func->second.param_types)))
			break;
	}

	if (func == range_end)
		throw NIGHT_CREATE_FATAL(
			"arguments in function call '" + func_name +
			"' do not match with the parameters in its function definition");

	return FunctionCall(lexer.loc, func_name, func->second.id, arg_exprs);
}

Conditional parse_if(Lexer& lexer, ParserScope& scope)
{
	assert(lexer.curr().type == TokenType::IF);

	std::vector<
		std::pair<std::shared_ptr<expr::Expression>, AST_Block>
	> conditionals;

	do {
		std::shared_ptr<expr::Expression> cond_expr =
			std::make_shared<expr::Value>(lexer.loc, (value_t)ValueType::BOOL, "1");

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
				NIGHT_CREATE_MINOR("condition is type '" + night::to_str(*cond_type) + "', expected type 'bool', 'char', or 'int'");
		}

		ParserScope stmt_scope{ scope.vars };
		conditionals.push_back({ cond_expr, parse_stmts(lexer, stmt_scope) });

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
		NIGHT_CREATE_MINOR("condition of type '" + night::to_str(*cond_type) + "', expected type 'bool', 'char', or 'int'");

	ParserScope stmt_scope{ scope.vars };
	return While(lexer.loc, cond_expr, parse_stmts(lexer, stmt_scope));
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
		NIGHT_CREATE_MINOR("condition of type '" + night::to_str(*cond_type) + "', expected type 'bool', 'char', or 'int'");

	// assignment

	auto var_assign_name = lexer.expect(TokenType::VARIABLE).str;
	lexer.expect(TokenType::ASSIGN);

	auto var_assign = parse_var_assign(lexer, scope, var_assign_name);

	ParserScope stmt_scope{ scope.vars };
	auto stmts = parse_stmts(lexer, stmt_scope);
	stmts.push_back(std::make_shared<VariableAssign>(var_assign));

	return For(lexer.loc, var_init, cond_expr, stmts);
}

Function parse_func(Lexer& lexer, ParserScope& scope)
{
	std::string func_name = lexer.expect(TokenType::VARIABLE).str;
	lexer.expect(TokenType::OPEN_BRACKET);

	// parse names and types of parameters

	std::vector<std::string> param_names;
	std::vector<value_t> param_types;

	while (lexer.curr().type != TokenType::CLOSE_BRACKET)
	{
		param_names.push_back(lexer.expect(TokenType::VARIABLE).str);

		if (!lexer.eat().is_type())
			throw NIGHT_CREATE_FATAL("expected type after variable name in function parameter list, found '" + lexer.curr().str + "'");

		param_types.push_back(token_var_type_to_val_type(lexer.curr().str));

		lexer.eat();

		if (lexer.curr().type == TokenType::CLOSE_BRACKET)
			break;
		if (lexer.curr().type != TokenType::COMMA)
			throw NIGHT_CREATE_FATAL("expected comma after variable type, found '" + lexer.curr().str + "'");

		lexer.eat();
	}

	// parse return type

	auto tok_rtn = lexer.eat();

	ParserScope::curr_rtn_type = std::nullopt;
	if (tok_rtn.is_type())
		ParserScope::curr_rtn_type = token_var_type_to_val_type(tok_rtn.str);
	else if (tok_rtn.type != TokenType::VOID)
		throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected return type");

	// create the function

	auto [err_msg, func_wrapper] = ParserScope::create_function(func_name, param_names, param_types, ParserScope::curr_rtn_type);
	if (!err_msg.empty())
		throw NIGHT_CREATE_FATAL(err_msg);

	auto& func_it = func_wrapper->second;

	// parse the body

	// create parameter ids and add them as variable in body scope

	std::vector<bytecode_t> param_ids;
	ParserScope body_scope{ scope.vars };

	for (std::size_t i = 0; i < param_names.size(); ++i)
	{
		auto [msg, id] = body_scope.create_variable(param_names[i], param_types[i]);
		if (msg.empty())
			throw NIGHT_CREATE_FATAL(msg);

		param_ids.push_back(id);
	}

	Function func(lexer.loc, func_it.id, param_ids, parse_stmts(lexer, body_scope));

	return func;
}

Return parse_return(Lexer& lexer, ParserScope& scope)
{
	assert(lexer.curr().type == TokenType::RETURN);

auto expr = parse_expr(lexer, scope);

if (lexer.curr().type != TokenType::SEMICOLON)
throw NIGHT_CREATE_FATAL("expected semicolon after return");

if (!expr)
{
	if (ParserScope::curr_rtn_type.has_value())
		NIGHT_CREATE_MINOR("return statement does nto return a value, yet function expects one of type '" +
			night::to_str(*ParserScope::curr_rtn_type) + "'");
}
else
{
	if (!ParserScope::curr_rtn_type.has_value())
		NIGHT_CREATE_MINOR("return statement found, yet function does not return a value");

	auto expr_type = expr->type_check(scope);

	if (expr_type.has_value() && !compare_value_t(*ParserScope::curr_rtn_type, *expr_type))
		NIGHT_CREATE_MINOR("return is type '" + night::to_str(*expr_type) + "', expected type '" +
			night::to_str(*ParserScope::curr_rtn_type) + "'");
}

return Return(lexer.loc, expr);
}

std::shared_ptr<expr::Expression> parse_expr(Lexer& lexer, ParserScope const& scope, std::string const& err_msg)
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
				node = std::make_shared<FunctionCall>(parse_func_call(lexer, scope, var_name));
				next.str = "";
			}
			else
			{
				node = std::make_shared<expr::Variable>(lexer.loc, var_name, scope.vars.at(var_name).id);
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
			node = parse_expr(lexer, scope, err_msg);

			if (lexer.curr().type != TokenType::CLOSE_BRACKET)
				throw NIGHT_CREATE_FATAL("missing closing bracket in expression");

			node->guard = true;
			allow_unary_next = false;
			break;
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

value_t token_var_type_to_val_type(std::string const& type)
{
	if (type == "bool") return (value_t)ValueType::BOOL;
	else if (type == "char") return (value_t)ValueType::CHAR;
	else if (type == "int") return (value_t)ValueType::INT;
	else if (type == "float") return (value_t)ValueType::FLOAT;
	else if (type == "str") return (value_t)ValueType::STRING;
	else throw debug::unhandled_case(type);
}