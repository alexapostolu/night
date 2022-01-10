#include "back-end/parser.hpp"
#include "back-end/token.hpp"
#include "back-end/stmt.hpp"
#include "back-end/check.hpp"
#include "error.hpp"
#include "util.hpp"

#include <functional>
#include <memory>
#include <algorithm>
#include <iterator>
#include <string>
#include <vector>
#include <unordered_set>
#include <iostream>

Parser::Parser(Lexer& lexer)
	: lexer(lexer)
{
	check_funcs = {
		make_check_function("print", { all_types }),
		make_check_function("input", {}, { Type::STR }),
			
		make_check_function("int",
			{ { Type::INT, Type::FLOAT, Type::STR } },
			{ Type::INT }),
		make_check_function("float",
			{ { Type::INT, Type::FLOAT, Type::STR } },
			{ Type::FLOAT }),
		make_check_function("str",
			{ { Type::BOOL, Type::INT, Type::FLOAT, Type::STR } },
			{ Type::STR }),
		//make_check_function("type",
			//{ all_types },
			//{ Type::TYPE }
		//),

		make_check_function("system",
			{ { Type::STR } }, { Type::INT })
	};

	check_classes = {
		make_check_class("array",
			CheckVariableContainer(),
			CheckFunctionContainer{
				make_check_function("len", {}, { Type::INT }),

				make_check_function("push", { all_types }),
				make_check_function("push", { { Type::INT } }, all_types),

				make_check_function("pop"),
				make_check_function("pop", { { Type::INT } })
			}
		),
		make_check_class("string",
			CheckVariableContainer(),
			CheckFunctionContainer{
				make_check_function("len", {}, { Type::INT })
			}
		)
	};
}

Stmt Parser::parse_statement(ParserScope& scope)
{
	static auto in_func = Parser::check_funcs.end();

	switch (lexer.get_curr().type)
	{
	case TokenType::LET:
		return parse_stmt_let(scope);
	case TokenType::VAR:
		return parse_stmt_var(scope);

	case TokenType::IF:
		return parse_stmt_if(scope);

	case TokenType::WHILE:
		return parse_stmt_while(scope);
	case TokenType::FOR:
		return parse_stmt_for(scope);

	case TokenType::FN:
		return parse_stmt_fn(scope, in_func);
	case TokenType::RETURN:
		return parse_stmt_rtn(scope, in_func);

	case TokenType::ELIF:
	case TokenType::ELSE:
		throw NIGHT_COMPILE_ERROR(
			lexer.get_curr().data + " statement does not precede an if or elif statement",
			lexer.get_curr().data + " statements must come after an if or an elif statement");

	default:
		throw NIGHT_COMPILE_ERROR(
			"unknown syntax",
			"no clue what you did here sorry :/");
	}
}

Stmt Parser::parse_stmt_let(ParserScope& scope)
{
	// validating statement

	if (lexer.eat(false).type != TokenType::VAR) {
		throw NIGHT_COMPILE_ERROR(
			"expected variable name after `let` keyword",
			night::format_init);
	}

	auto const var_name = lexer.get_curr().data;

	if (lexer.eat(false).type != TokenType::ASSIGN) {
		throw NIGHT_COMPILE_ERROR(
			"expected assignment after variable name",
			night::format_init);
	}

	if (scope.get_var(var_name) != nullptr) {
		throw NIGHT_COMPILE_ERROR(
			"variable '" + var_name + "' has already been defined",
			"variables can only be defined once, regardless of their scope");
	}
	if (check_funcs.contains(var_name)) {
		throw NIGHT_COMPILE_ERROR(
			"variable '" + var_name + "' has the same name as a function",
			"variable and function names must be unique");
	}
	if (check_classes.contains(var_name)) {
		throw NIGHT_COMPILE_ERROR(
			"variable '" + var_name + "' has the same name as a class",
			"variable and class names must be unique");
	}

	// parsing statement

	lexer.eat(false); // move lexer to start of expression
	auto [expr, types] = parse_expression(scope);

	if (expr == nullptr) {
		throw NIGHT_COMPILE_ERROR(
			"expected expression after assignment operator",
			night::format_init);
	}
	if (lexer.get_curr().type != TokenType::EOL) {
		throw NIGHT_COMPILE_ERROR(
			"unexpected token '" + lexer.get_curr().data + "' after expression",
			night::format_init);
	}

	scope.vars[var_name] = CheckVariable{ types };

	return Stmt{
		lexer.get_loc(), StmtType::INIT,
		StmtInit{ var_name, expr }
	};
}

Stmt Parser::parse_stmt_var(ParserScope& scope)
{
	auto const var_name = lexer.get_curr().data;

	Token token = lexer.eat(false);
	if (token.type == TokenType::OPEN_BRACKET)
	{
		auto const check_func = check_funcs.find(var_name);
		if (check_func == check_funcs.end()) {
			throw NIGHT_COMPILE_ERROR(
				"function `" + var_name + "` is undefined",
				"functions must be defined before they are used");
		}

		auto const [arg_exprs, arg_types] =
			parse_arguments(scope, check_func->first);

		// type checking arguments

		if (arg_types.size() != check_func->second.param_types.size()) {
			throw NIGHT_COMPILE_ERROR(
				"function call `" + check_func->first + "` has " + std::to_string(arg_types.size()) + " argument(s)",
				"function must be called with " + std::to_string(check_func->second.param_types.size()) + " argument(s)");
		}

		check_call_types(check_func->second.param_types,
			arg_types, check_func->first);

		auto const loc = lexer.get_loc();

		// move lexer to start of next statement
		lexer.eat(true);

		return Stmt{
			loc, StmtType::CALL,
			StmtCall{ check_func->first, arg_exprs }
		};
	}
	else
	{
		ValueVar const val_var{ var_name };
		auto var_expr = std::make_shared<ExprNode>(lexer.get_loc(), ExprNode::VARIABLE, val_var);
		std::vector<std::shared_ptr<ExprNode> > subscript_chain;
		bool contains_method = false;

		auto* const check_var = scope.get_var(var_name);
		if (check_var == nullptr) {
			throw NIGHT_COMPILE_ERROR(
				"variable '" + var_name + "' is undefined",
				"variables must be defined before they are used");
		}

		while (token.type != TokenType::ASSIGN && token.type != TokenType::EOL)
		{
			if (token.type == TokenType::BINARY_OP && token.data == ".")
			{
				contains_method = true;

				// validating statement

				if (lexer.eat(false).type != TokenType::VAR) {
					throw NIGHT_COMPILE_ERROR(
						"expected variable name after dot operator",
						night::format_method);
				}

				auto const method_name = lexer.get_curr().data;

				if (lexer.eat(false).type != TokenType::OPEN_BRACKET) {
					throw NIGHT_COMPILE_ERROR(
						"expected open bracket after method `" + method_name + "`",
						night::format_method);
				}

				// parsing arguments

				auto const [arg_exprs, arg_types] =
					parse_arguments(scope, method_name);

				// definition checking method call based on name and argument types

				// [ refactor ]
				// find check functions based on previous expression class
				// have a 'previous_class_types' variable

				bool ok = false;
				for (auto& check_class : Parser::check_classes)
				{
					auto method = std::ranges::find_if(
						check_class.second.methods,
						[&](auto const& method) {
							return method.first == method_name &&
								method.second.param_types.size() == arg_exprs.size();
						}
					);

					if (method == check_class.second.methods.end())
						continue;

					try {
						check_call_types(
							method->second.param_types,
							arg_types, method->first);
					}
					catch (night::error const&) {
						continue;
					}

					ok = true;
					break;
				}

				if (!ok) {
					throw NIGHT_COMPILE_ERROR(
						"no matching function call `" + method_name + "` in class method",
						"methods have to be defined before they are used");
				}

				// adding expression node
				// since all the operators here are dot or subscript, we
				// can just keep building the AST up

				ValueCall const val_call{ method_name, arg_exprs };

				auto const call_node =
					std::make_shared<ExprNode>(lexer.get_loc(), ExprNode::CALL, val_call);

				BinaryOPNode const op_expr{ lexer.get_loc(),
					BinaryOPNode::DOT, ".", var_expr, call_node };

				auto const op_node =
					std::make_shared<ExprNode>(lexer.get_loc(), ExprNode::BINARY_OP, op_expr);

				if (var_expr->type == ExprNode::VARIABLE && method_name == "push")
				{
					TypeContainer& arr = scope.vars[std::get<ValueVar>(var_expr->data).name].types;
					auto const& arr_t = std::ranges::find(arr, Type::ARR);
					if (arr_t != arr.end())
						arr_t->elem_types.insert(arr_t->elem_types.end(), arg_types.back().begin(), arg_types.back().end());
				}

				var_expr = op_node;
			}
			else if (token.type == TokenType::OPEN_SQUARE)
			{
				lexer.eat(false);
				auto [expr, types] = parse_expression(scope, { Type::INT });

				if (lexer.get_curr().type != TokenType::CLOSE_SQUARE) {
					throw NIGHT_COMPILE_ERROR(
						"expected closing square bracket in subscript operator",
						night::format_subscript);
				}
				if (!night::contains(types, Type::INT)) {
					throw NIGHT_COMPILE_ERROR(
						"subscript index must contain type 'int'",
						"index currently contains " + types_as_str(types));
				}

				subscript_chain.push_back(expr);

				UnaryOPNode const sub_op{
					UnaryOPNode::SUBSCRIPT, "[]", var_expr };

				std::shared_ptr<ExprNode> const sub_expr =
					std::make_shared<ExprNode>(lexer.get_loc(), ExprNode::UNARY_OP, sub_op);

				var_expr = sub_expr;
			}
			else
			{
				throw NIGHT_COMPILE_ERROR(
					"unexpected token '" + token.data + "' in variable statement", "");
			}

			token = lexer.eat(false);
		}

		if (token.type == TokenType::ASSIGN)
		{
			if (contains_method) {
				throw NIGHT_COMPILE_ERROR(
					"method calls do not return references",
					"assigning values to return values is not allowed");
			}

			StmtAssign::T assign_type;

			if (token.data == "=")
				assign_type = StmtAssign::ASSIGN;
			else if (token.data == "+=")
				assign_type = StmtAssign::PLUS;
			else if (token.data == "-=")
				assign_type = StmtAssign::MINUS;
			else if (token.data == "*=")
				assign_type = StmtAssign::TIMES;
			else if (token.data == "/=")
				assign_type = StmtAssign::DIVIDE;
			else
				assign_type = StmtAssign::MOD;

			lexer.eat(false);
			auto [expr, type] = parse_expression(scope);

			lexer.eat(true);

			return Stmt{
				lexer.get_loc(), StmtType::ASSIGN,
				StmtAssign{ assign_type, check_var->first, subscript_chain, expr }
			};
		}
		else if (token.type == TokenType::EOL)
		{
			if (var_expr != nullptr && var_expr->type == ExprNode::UNARY_OP) {
				throw NIGHT_COMPILE_ERROR("values are not valid statements", "");
			}

			return Stmt{
				lexer.get_loc(), StmtType::METHOD,
				StmtMethod{ var_expr }
			};
		}
		else
		{
			throw NIGHT_COMPILE_ERROR(
				"unexpected token '" + token.data + "' in variable statement", "");
		}
	}
}

Stmt Parser::parse_stmt_if(ParserScope& scope)
{
	// parsing if statement

	auto const if_expr = parse_condition(scope, "if statement");

	Scope if_scope{ scope };
	auto const body = parse_body(if_scope, "if conditional", night::format_if);

	// parsing next conditional statements

	std::vector<Conditional> conditionals{ Conditional{ if_expr, body } };

	while (true)
	{
		Scope condition_scope{ scope };
		std::shared_ptr<ExprNode> condition_expr(nullptr);

		if (lexer.get_curr().type == TokenType::ELIF)
			condition_expr = parse_condition(scope, "else if statement");
		else if (lexer.get_curr().type == TokenType::ELSE)
			lexer.eat(true);
		else
			break;

		conditionals.push_back(Conditional{
			condition_expr,
			parse_body(condition_scope, "conditional", night::format_elif)
		});

		if (condition_expr == nullptr)
			break;
	}

	return Stmt{
		lexer.get_loc(), StmtType::IF,
		StmtIf{ conditionals }
	};
}

Stmt Parser::parse_stmt_while(ParserScope& scope)
{
	auto const condition_expr = parse_condition(scope, "while loop");

	ParserScope while_scope{ &scope };
	auto const body = parse_body(while_scope, "while loop", night::format_while);

	return Stmt{
		lexer.get_loc(), StmtType::WHILE,
		StmtWhile{ condition_expr, body }
	};
}

Stmt Parser::parse_stmt_for(ParserScope& scope)
{
	// validating statement

	if (lexer.eat(false).type != TokenType::OPEN_BRACKET) {
		throw NIGHT_COMPILE_ERROR(
			"expected opening bracket after 'for' keyword",
			night::format_for);
	}
	if (lexer.eat(false).type != TokenType::VAR) {
		throw NIGHT_COMPILE_ERROR(
			"expected iterator after opening bracket",
			night::format_for);
	}

	auto const it_name = lexer.get_curr().data;

	if (lexer.eat(false).type != TokenType::COLON) {
		throw NIGHT_COMPILE_ERROR(
			"expected colon after iterator",
			night::format_for);
	}
	if (lexer.eat(false).type == TokenType::EOL) {
		throw NIGHT_COMPILE_ERROR(
			"expected range after colon",
			night::format_for);
	}

	// parsing iterator and range

	auto [range_expr, range_types] = parse_expression(
		scope, { Type::STR, Type{ Type::ARR, Parser::all_types } });

	if (lexer.get_curr().type != TokenType::CLOSE_BRACKET) {
		throw NIGHT_COMPILE_ERROR(
			"expected closing bracket at the end of expression",
			night::format_for);
	}

	if (!night::contains(range_types, Type::STR, Type::ARR)) {
		throw NIGHT_COMPILE_ERROR(
			"range currently contains " + types_as_str(range_types),
			"for loop range must contain type 'str' or 'arr'");
	}

	// parsing statement

	if (lexer.eat(true).type == TokenType::EOL) {
		throw NIGHT_COMPILE_ERROR(
			"expected statement(s) after closing bracket",
			night::format_for);
	}

	ParserScope for_scope{ &scope };

	for_scope.vars[it_name] = CheckVariable();
	if (night::contains(range_types, Type::STR))
		for_scope.vars[it_name].types.push_back(Type::STR);
	if (auto it = std::ranges::find(range_types, Type::ARR); it != range_types.end())
		for_scope.vars[it_name].types.insert(for_scope.vars[it_name].types.end(), it->elem_types.begin(), it->elem_types.end());

	return Stmt{
		lexer.get_loc(), StmtType::FOR,
		StmtFor{ it_name, range_expr,
				 parse_body(for_scope, "for loop", night::format_for) }
	};
}

Stmt Parser::parse_stmt_fn(ParserScope& scope, CheckFunctionContainer::iterator &in_func)
{
	// validating statement

	if (lexer.eat(false).type != TokenType::VAR) {
		throw NIGHT_COMPILE_ERROR(
			"expected function name after 'def' keyword",
			night::format_fn);
	}

	auto const func_name = lexer.get_curr().data;

	if (lexer.eat(false).type != TokenType::OPEN_BRACKET) {
		throw NIGHT_COMPILE_ERROR(
			"expected opening bracket after function name",
			night::format_fn);
	}

	if (scope.vars.contains(func_name)) {
		throw NIGHT_COMPILE_ERROR(
			"function '" + func_name + "' can not have the same name as a variable",
			"function and variable names must be unique");
	}
	if (check_funcs.contains(func_name)) {
		throw NIGHT_COMPILE_ERROR(
			"function '" + func_name + "' has already been defined",
			"functions can only be defined once");
	}
	if (check_classes.contains(func_name)) {
		throw NIGHT_COMPILE_ERROR(
			"function '" + func_name + "' can not have the same name as a class",
			"function and class names must be unique");
	}

	if (scope.upper != nullptr) {
		throw NIGHT_COMPILE_ERROR(
			"function '" + func_name + "' can not be defined inside of a local scope",
			"functions must be defined in the global scope");
	}

	// parsing parameters
	// also turns parameters into variables for definition checking the function body

	Scope func_scope{ scope };

	std::vector<std::string> param_names;

	bool once = true;
	while (true)
	{
		Token token = lexer.eat(false);
		if (once && token.type == TokenType::CLOSE_BRACKET)
			break;

		if (token.type != TokenType::VAR) {
			throw NIGHT_COMPILE_ERROR(
				"expected variable names as function parameters",
				night::format_fn);
		}

		std::string const param_name = lexer.get_curr().data;

		if (Parser::check_funcs.contains(param_name)) {
			throw NIGHT_COMPILE_ERROR(
				"function parameter can not have the same name as a function",
				"function parameter names must be unique");
		}
		if (Parser::check_classes.contains(param_name)) {
			throw NIGHT_COMPILE_ERROR(
				"function parameter can not have the same name as a class",
				"function parameter names must be unique");
		}

		param_names.push_back(param_name);
		func_scope.vars[param_name] = CheckVariable{};

		token = lexer.eat(false);

		if (token.type == TokenType::CLOSE_BRACKET)
			break;

		if (token.type != TokenType::COMMA) {
			throw NIGHT_COMPILE_ERROR(
				"expected command or closing bracket after parameter '" + param_name + "'",
				night::format_fn);
		}
	}

	// define function before parsing body to ensure function is defined for recursion
	// and return types

	Parser::check_funcs[func_name] = CheckFunction();

	in_func = Parser::check_funcs.find(func_name);

	auto& check_func = in_func->second;
	check_func.param_types.resize(param_names.size());

	// parsing body

	// move lexer to first token of body
	lexer.eat(true);

	auto fn_stmts = parse_body(func_scope, "function definition", night::format_fn);

	in_func = Parser::check_funcs.end();

	// set check function to be void or not, and assign check function
	// parameters' types

	check_func.is_void = check_func.rtn_types.empty();

	for (std::size_t a = 0; a < check_func.param_types.size(); ++a)
	{
		auto& assign_types = func_scope.vars.at(param_names.at(a)).types;

		check_func.param_types[a] = assign_types.empty()
			? all_types
			: assign_types;
	}

	return Stmt{
		lexer.get_loc(), StmtType::FN,
		StmtFn{ func_name, param_names, fn_stmts }
	};
}

Stmt Parser::parse_stmt_rtn(ParserScope& scope, CheckFunctionContainer::iterator &in_func)
{
	if (in_func == Parser::check_funcs.end()) {
		throw NIGHT_COMPILE_ERROR(
			"return statement is outside of a function",
			"return statements must be inside of a function");
	}

	lexer.eat(false);

	auto const [expr, types] = parse_expression(scope);

	in_func->second.rtn_types.insert(
		in_func->second.rtn_types.end(),
		types.begin(), types.end());

	lexer.eat(true);

	return Stmt{
		lexer.get_loc(), StmtType::RETURN,
		StmtReturn{ expr }
	};
}

std::vector<Stmt> Parser::parse_body(
	ParserScope& scope, std::string const& stmt_name,
	std::string const& stmt_format)
{
	if (lexer.get_curr().type != TokenType::OPEN_CURLY)
		return { parse_statement(scope) };

	// move lexer to first token of body
	lexer.eat(true);

	std::vector<Stmt> stmts;
	while (true)
	{
		auto token = lexer.get_curr();
		if (token.type == TokenType::CLOSE_CURLY)
		{
			if (lexer.eat(false).type != TokenType::EOL) {
				throw NIGHT_COMPILE_ERROR(
					"unexpected token '" + lexer.get_curr().data + "' after closing curly bracket",
					stmt_format);
			}

			// move lexer to next statement
			lexer.eat(true);

			return stmts;
		}
		else if (token.type == TokenType::EOL)
		{
			// move lexer to next line
			lexer.eat(true);
		}
		else if (token.type == TokenType::_EOF)
		{
			throw NIGHT_COMPILE_ERROR(
				"expected closing bracket at the end of " + stmt_name + " body",
				stmt_format);
		}

		stmts.push_back(parse_statement(scope));
	}
}

std::shared_ptr<ExprNode> Parser::parse_condition(
	ParserScope& scope, std::string const& stmt_format)
{
	// validating statement

	auto const loop_name = lexer.get_curr().data;

	if (lexer.eat(false).type != TokenType::OPEN_BRACKET) {
		throw NIGHT_COMPILE_ERROR(
			"expected opening bracket after '" + loop_name + "' keyword",
			stmt_format);
	}
	if (lexer.eat(false).type == TokenType::EOL) {
		throw NIGHT_COMPILE_ERROR(
			"expected an expression after opening bracket",
			stmt_format);
	}

	// parsing condition

	auto [condition_expr, condition_types] =
		parse_expression(scope, { Type::BOOL });

	if (lexer.get_curr().type != TokenType::CLOSE_BRACKET) {
		throw NIGHT_COMPILE_ERROR(
			"expected closing bracket after " + loop_name + " condition",
			stmt_format);
	}
	if (!night::contains(condition_types, Type::BOOL)) {
		throw NIGHT_COMPILE_ERROR(
			loop_name + " condition currently contains " + types_as_str(condition_types),
			"condition must contain type 'bool'");
	}

	// move to start of body
	if (lexer.eat(true).type == TokenType::_EOF) {
		throw NIGHT_COMPILE_ERROR(
			"expected statement(s) after closing bracket",
			night::format_while);
	}

	return condition_expr;
}

std::pair<ExprContainer, std::vector<TypeContainer> >
Parser::parse_arguments(
	ParserScope& scope, std::string_view func_name)
{
	if (lexer.eat(false).type == TokenType::EOL) {
		throw NIGHT_COMPILE_ERROR(
			"for function call	" + std::string(func_name) + "`: expected expression after open bracket",
			night::format_call);
	}

	ExprContainer call_exprs;
	std::vector<TypeContainer> call_types;

	while (true)
	{
		auto const [expr, types] = parse_expression(scope);

		auto const token = lexer.get_curr();
		if (token.type == TokenType::EOL) {
			throw NIGHT_COMPILE_ERROR(
				"for function call `" + std::string(func_name) + "`, last argument: expected closing bracket after expression",
				night::format_call);
		}

		call_exprs.push_back(expr);
		call_types.push_back(types);

		if (token.type == TokenType::CLOSE_BRACKET)
		{
			if (expr == nullptr)
				return { {}, {} };
			else
				return { call_exprs, call_types };
		}

		if (token.type != TokenType::COMMA) {
			throw NIGHT_COMPILE_ERROR(
				"unexpected token '" + token.data + "' in function call '" + std::string(func_name) + "'",
				night::format_call);
		}

		lexer.eat(false);
	}
}

std::tuple<std::shared_ptr<ExprNode>, TypeContainer>
Parser::parse_expression(
	ParserScope& scope,
	TypeContainer const& required_types)
{
	// there can only be one range per expression
	int range_count = 0;

	static std::unordered_map<std::string, UnaryOPNode::T> const unary_op_convers{
		{ "-", UnaryOPNode::NEGATIVE }, { "!", UnaryOPNode::NOT } };

	static std::unordered_map<std::string, BinaryOPNode::T> const binary_op_convers{
		{ "+", BinaryOPNode::PLUS },		{ "-", BinaryOPNode::MINUS },
		{ "*", BinaryOPNode::TIMES },		{ "/", BinaryOPNode::DIVIDE },
		{ "%", BinaryOPNode::MOD },
		{ ">", BinaryOPNode::GREATER },     { "<", BinaryOPNode::SMALLER },
		{ ">=", BinaryOPNode::GREATER_EQ }, { "<=", BinaryOPNode::SMALLER_EQ },
		{ "==", BinaryOPNode::EQUAL },		{ "!=", BinaryOPNode::NOT_EQUAL },
		{ "||", BinaryOPNode::OR },			{ "&&", BinaryOPNode::AND },
		{ ".", BinaryOPNode::DOT },			{ "..", BinaryOPNode::RANGE }
	};

	std::shared_ptr<ExprNode> root(nullptr), protect(nullptr);

	while (true)
	{
		ExprNode* prev = nullptr;
		std::shared_ptr<ExprNode> curr(root);

		Token token = lexer.get_curr();
		if (token.is_value())
		{
			// traveling tree

			while (curr != nullptr)
			{
				if (curr->is_value()) {
					throw NIGHT_COMPILE_ERROR(
						"expected operator between values",
						"");
				}

				prev = curr.get();
				curr = curr->travel_ast();
			}

			// constructing node

			if (prev == nullptr)
			{
				root = std::make_shared<ExprNode>();
				curr = root;
			}
			else
			{
				assert(prev->travel_ast() == nullptr);

				prev->travel_ast() = std::make_shared<ExprNode>();
				curr = prev->travel_ast();
			}

			curr->loc = lexer.get_loc();

			switch (token.type)
			{
			case TokenType::BOOL_L: {
				curr->type = ExprNode::LITERAL;
				curr->data = ValueLiteral{ ValueLiteral::BOOL, token.data == "true" };

				break;
			}
			case TokenType::INT_L: {
				curr->type = ExprNode::LITERAL;
				curr->data = ValueLiteral{ ValueLiteral::INT, std::stoi(token.data) };

				break;
			}
			case TokenType::FLOAT_L: {
				curr->type = ExprNode::LITERAL;
				curr->data = ValueLiteral{ ValueLiteral::FLOAT, std::stof(token.data) };

				break;
			}
			case TokenType::STR_L: {
				curr->type = ExprNode::LITERAL;
				curr->data = ValueLiteral{ ValueLiteral::STR, token.data };

				break;
			}
			case TokenType::VAR: {
				auto const var_name = lexer.get_curr().data;

				token = lexer.eat(false);

				if (token.type == TokenType::OPEN_BRACKET)
				{
					auto const [arg_exprs, arg_types] =
						parse_arguments(scope, var_name);

					curr->type = ExprNode::CALL;
					curr->data = ValueCall(var_name, arg_exprs);
				}
				else if (token.type == TokenType::OPEN_SQUARE)
				{	
					curr->type = ExprNode::VARIABLE;
					curr->data = ValueVar{ var_name };

					// constructing node

					if (lexer.eat(false).type == TokenType::EOL) {
						throw NIGHT_COMPILE_ERROR(
							"expected expression after open bracket",
							night::format_subscript);
					}

					auto const [expr, types] = parse_expression(scope);

					if (lexer.get_curr().type != TokenType::CLOSE_SQUARE) {
						throw NIGHT_COMPILE_ERROR(
							"expected closing square bracket at the end of subscript operator",
							night::format_subscript);
					}

					auto const sub_op = std::make_shared<ExprNode>(
						lexer.get_loc(),
						ExprNode::UNARY_OP,
						UnaryOPNode{ UnaryOPNode::SUBSCRIPT, "[]", curr, expr }
					);

					if (prev == nullptr)
						root = sub_op;
					else
						prev->travel_ast() = sub_op;
				}
				else
				{
					curr->type = ExprNode::VARIABLE;
					curr->data = ValueVar{ var_name };
					goto END;
				}

				break;
			}
			case TokenType::OPEN_SQUARE: {
				ExprContainer arr_exprs;

				bool once = true;
				while (true)
				{
					lexer.eat(false);

					if (once && lexer.get_curr().type == TokenType::CLOSE_SQUARE)
						break;
					once = false;

					auto const expr = std::get<0>(parse_expression(scope));
					if (expr == nullptr) {
						throw NIGHT_COMPILE_ERROR(
							"expected element in array", night::format_array);
					}

					arr_exprs.push_back(expr);

					token = lexer.get_curr();

					if (token.type == TokenType::CLOSE_SQUARE)
						break;

					if (token.type == TokenType::EOL) {
						throw NIGHT_COMPILE_ERROR(
							"expected closing square bracket in array",
							night::format_array);
					}
					if (token.type != TokenType::COMMA) {
						throw NIGHT_COMPILE_ERROR(
							"unexpected token '" + token.data + "' in array",
							night::format_array);
					}
				}

				curr->type = ExprNode::ARRAY;
				curr->data = ValueArray{ arr_exprs };

				break;
			}
			default:
				throw std::runtime_error("Parser::parse_expression(), missing token value");
			}
		}
		else if (token.type == TokenType::OPEN_BRACKET)
		{
			while (curr != nullptr)
			{
				if (curr->is_value()) {
					throw NIGHT_COMPILE_ERROR(
						"expected operator between value and opening bracket", "");
				}

				prev = curr.get();
				curr = curr->travel_ast();
			}

			if (root == nullptr)
			{
				lexer.eat(false);
				root = std::get<0>(parse_expression(scope, required_types));
				protect = root;
			}
			else
			{
				lexer.eat(false);
				prev->travel_ast() = std::get<0>(parse_expression(scope, required_types));
				protect = prev->travel_ast();
			}

			if (lexer.get_curr().type != TokenType::CLOSE_BRACKET) {
				throw NIGHT_COMPILE_ERROR(
					"expected closing bracket in expression", "");
			}
		}
		else if (token.is_operator())
		{
			if (curr == nullptr && token.type == TokenType::BINARY_OP) {
				throw NIGHT_COMPILE_ERROR(
					"expected value before operator `" + token.data + "`",
					"");
			}

			//traveling tree

			while (curr != nullptr && !curr->is_value() && curr != protect)
			{
				auto const op_data = curr->type == ExprNode::UNARY_OP
					? std::get<UnaryOPNode>(curr->data).data
					: std::get<BinaryOPNode>(curr->data).data;

				if (!higher_precedence(token.data, op_data))
					break;

				prev = curr.get();
				curr = curr->travel_ast();
			}

			// constructing node

			auto op_node = std::make_shared<ExprNode>();
			op_node->loc = lexer.get_loc();

			if (token.type == TokenType::UNARY_OP)
			{
				auto const pos = unary_op_convers.find(token.data);
				assert(pos != unary_op_convers.end());

				op_node->type = ExprNode::UNARY_OP;
				op_node->data = UnaryOPNode{ pos->second, token.data, curr };
			}
			else
			{
				auto const op_type = binary_op_convers.find(token.data);
				assert(op_type != binary_op_convers.end());

				if (op_type->second == BinaryOPNode::RANGE)
				{
					range_count++;
					if (range_count > 1) {
						throw NIGHT_COMPILE_ERROR("there can only be one range per expression",
							"");
					}
				}

				op_node->type = ExprNode::BINARY_OP;
				op_node->data = BinaryOPNode{ lexer.get_loc(), op_type->second, token.data, curr, nullptr };
			}

			if (prev == nullptr)
				root = op_node;
			else
				prev->travel_ast() = op_node;
		}
		else
		{
			return root == nullptr
				? std::make_tuple(nullptr, TypeContainer{})
				: std::make_tuple(root, type_check_expr(scope, root, required_types));
		}

		lexer.eat(false);

	END:;
	}
}

bool
Parser::higher_precedence(
	std::string const& op1,
	std::string const& op2) const
{
	std::vector<std::unordered_set<std::string> > const operators{
		{ ".." },
		{ "||", "&&" },
		{ "==", "!=" },
		{ ">", "<", ">=", "<=" },
		{ "+", "-" },
		{ "*", "/", "%" },
		{ "!" },
		{ "[]", "." },
	};

	std::size_t pre1 = operators.size(), pre2;
	for (std::size_t a = 0; a < operators.size(); ++a)
	{
		if (operators[a].contains(op1))
			pre1 = a;
		if (operators[a].contains(op2))
			pre2 = a;
	}

	assert(pre1 != operators.size());
	return pre1 > pre2;
}

TypeContainer
Parser::type_check_expr(
	ParserScope&					 scope,
	std::shared_ptr<ExprNode> const& expr,
	TypeContainer const&			 required_types,
	TypeContainer* const_types,
	CheckVariable* const_var) const
{
	assert(expr != nullptr);
	switch (expr->type)
	{
	case ExprNode::LITERAL: {
		auto& val = std::get<ValueLiteral>(expr->data);

		switch (val.type)
		{
		case ValueLiteral::BOOL:
			return { Type::BOOL };
		case ValueLiteral::INT:
			return { Type::INT };
		case ValueLiteral::FLOAT:
			return { Type::FLOAT };
		case ValueLiteral::STR:
			return { Type::STR };
		}
	}
	case ExprNode::ARRAY: {
		auto& val = std::get<ValueArray>(expr->data);

		TypeContainer elems_types;
		for (auto const& elem_expr : val.elem_exprs)
		{
			auto elem_types = type_check_expr(scope, elem_expr);
			if (night::contains(elem_types, Type::RNG))
			{
				assert(elem_types.size() == 1);
				elems_types.insert(elems_types.end(), { Type::INT, Type::FLOAT });
			}
			else
			{
				elems_types.insert(elems_types.end(),
					elem_types.begin(), elem_types.end());
			}
		}

		return { Type(Type::ARR, elems_types) };
	}
	case ExprNode::VARIABLE: {
		auto& val = std::get<ValueVar>(expr->data);

		auto* const check_var = scope.get_var(val.name);
		if (check_var == nullptr) {
			throw NIGHT_COMPILE_ERROR(
				"variable '" + val.name + "' is undefined",
				"variables must be defined before they are used");
		}

		// if the variable is a function parameter
		if (check_var->second.types.empty())
			check_var->second.types = required_types;

		return check_var->second.types;
	}
	case ExprNode::CALL: {
		auto& val = std::get<ValueCall>(expr->data);

		auto const check_func_it = check_funcs.find(val.name);
		if (check_func_it == check_funcs.end()) {
			throw NIGHT_COMPILE_ERROR(
				"function '" + val.name + "' is not defined",
				"functions have to be defined before they are used");
		}

		auto& check_func = check_func_it->second;

		if (val.param_exprs.size() != check_func.param_types.size()) {
			throw NIGHT_COMPILE_ERROR(
				"function `" + val.name + "` is called with `" + std::to_string(val.param_exprs.size()) + "` argument(s)",
				"function can only be called with `" + std::to_string(check_func.param_types.size()) + "` argument(s)");
		}

		if (check_func.is_void) {
			throw NIGHT_COMPILE_ERROR(
				"function `" + val.name + "` does not have a return value",
				"functions must have a return value to be used in expression");
		}

		// type checking arguments
		for (std::size_t a = 0; a < check_func.param_types.size(); ++a)
		{
			auto const arg_types = type_check_expr(
				scope, val.param_exprs.at(a), check_func.param_types[a]);

			if (arg_types.empty() || check_func.param_types[a].empty())
				continue;

			bool const match = std::ranges::any_of(
				check_func.param_types[a],
				[&](auto const& param_type) {
					return night::contains(arg_types, param_type); }
			);

			if (!match) {
				throw NIGHT_COMPILE_ERROR(
					"function '" + val.name + "' argument number " + std::to_string(a + 1) + ", must contain " + types_as_str(check_func.param_types[a]),
					"argument currently contains " + types_as_str(arg_types));
			}

			// store the types of constant type checking
			if (const_types != nullptr && check_func_it->first == "type")
			{
				const_types->insert(const_types->end(),
					arg_types.begin(), arg_types.end());
			}
		}

		return check_func.rtn_types;
	}
	case ExprNode::UNARY_OP: {
		auto const throw_unary_op_err = [&](
			UnaryOPNode   const& unary_op,
			TypeContainer const& types,
			std::string   const& used_types)
		{
			throw NIGHT_COMPILE_ERROR(
				"operator '" + unary_op.data + "' is currently used on " + types_as_str(types),
				"operator '" + unary_op.data + "' can only be used on " + used_types);
		};

		auto& unary_op = std::get<UnaryOPNode>(expr->data);
		switch (unary_op.type)
		{
		case UnaryOPNode::NEGATIVE: {
			auto const types = type_check_expr(
				scope, unary_op.value, { Type::INT, Type::FLOAT });

			if (!night::contains(types, Type::INT, Type::FLOAT))
				throw_unary_op_err(unary_op, types, "types 'int' or 'float'");

			return { Type::INT, Type::FLOAT };
		}
		case UnaryOPNode::NOT: {
			auto const types = type_check_expr(
				scope, unary_op.value, { Type::BOOL });

			if (!night::contains(types, Type::BOOL))
				throw_unary_op_err(unary_op, types, "type 'bool'");

			return { Type::BOOL };
		}
		case UnaryOPNode::SUBSCRIPT: {
			auto index = type_check_expr(scope, unary_op.index, { Type::INT });
			if (!night::contains(index, Type::INT)) {
				throw NIGHT_COMPILE_ERROR(
					"subscript operator currently contains " + types_as_str(index),
					"subscript operator must contains type 'int'");
			}

			auto const types = type_check_expr(
				scope, unary_op.value, { Type::STR, Type::ARR});

			TypeContainer rtn_types;
			if (night::contains(types, Type::STR))
				rtn_types.insert(rtn_types.end(), Type::STR);
			if (auto const pos = std::ranges::find(types, Type::ARR); pos != types.end())
			{
				rtn_types.insert(rtn_types.end(), pos->elem_types.begin(), pos->elem_types.end());
				return rtn_types;
			}

			if (rtn_types.empty())
				throw_unary_op_err(unary_op, types, "types 'str' or 'arr'");

			return rtn_types;
		}
		}
	}
	case ExprNode::BINARY_OP: {
		auto const& binary_op = std::get<BinaryOPNode>(expr->data);

		switch (binary_op.type)
		{
		case BinaryOPNode::PLUS: {
			auto left_types = type_check_expr(
				scope, binary_op.left, { Type::INT, Type::FLOAT, Type::STR });

			if (!night::contains(left_types, Type::INT, Type::FLOAT, Type::STR))
				throw_binary_type_err(binary_op, left_types, "left", "`int`, `float`, or `str`");

			auto right_types = type_check_expr(
				scope, binary_op.right, { Type::INT, Type::FLOAT, Type::STR });
			
			if (!night::contains(right_types, Type::INT, Type::FLOAT, Type::STR))
				throw_binary_type_err(binary_op, right_types, "right", "`int`, `float`, or `str`");

			return { Type::INT, Type::FLOAT, Type::STR };
		}

		case BinaryOPNode::MINUS:
		case BinaryOPNode::TIMES:
		case BinaryOPNode::DIVIDE:
		case BinaryOPNode::MOD: {
			auto const left_types = type_check_expr(
				scope, binary_op.left, { Type::INT, Type::FLOAT });

			if (!night::contains(left_types, Type::INT, Type::FLOAT))
				throw_binary_type_err(binary_op, left_types, "left", "types 'int' or 'float'");

			auto const right_types = type_check_expr(
				scope, binary_op.right, { Type::INT, Type::FLOAT });

			if (!night::contains(right_types, Type::INT, Type::FLOAT))
				throw_binary_type_err(binary_op, right_types, "right", "types 'int' or 'float'");

			return TypeContainer{ Type::INT, Type::FLOAT };
		}
		
		case BinaryOPNode::GREATER:
		case BinaryOPNode::GREATER_EQ:
		case BinaryOPNode::SMALLER:
		case BinaryOPNode::SMALLER_EQ: {
			auto const left_types = type_check_expr(
				scope, binary_op.left, { Type::INT, Type::FLOAT });

			if (!night::contains(left_types, Type::INT, Type::FLOAT))
				throw_binary_type_err(binary_op, left_types, "left", "types 'int' or 'float'");

			auto const right_types = type_check_expr(
				scope, binary_op.right, { Type::INT, Type::FLOAT });

			if (!night::contains(right_types, Type::INT, Type::FLOAT))
				throw_binary_type_err(binary_op, right_types, "right", "types 'int' or 'float'");

			return TypeContainer{ Type::BOOL };
		}

		case BinaryOPNode::AND:
		case BinaryOPNode::OR: {
			auto const left_types = type_check_expr(
				scope, binary_op.left, { Type::BOOL });

			if (!night::contains(left_types, Type::BOOL))
				throw_binary_type_err(binary_op, left_types, "left", "type 'bool'");

			auto const right_types = type_check_expr(
				scope, binary_op.left, { Type::BOOL });

			if (!night::contains(right_types, Type::BOOL))
				throw_binary_type_err(binary_op, right_types, "left", "type 'bool'");

			return { Type::BOOL };
		}

		case BinaryOPNode::EQUAL:
		case BinaryOPNode::NOT_EQUAL: {
			TypeContainer get_const_types;
			CheckVariable get_const_var;

			auto const& left_types = type_check_expr(
				scope, binary_op.left, all_types, &get_const_types, &get_const_var);

			auto const& right_types = type_check_expr(
				scope, binary_op.right, all_types, &get_const_types, &get_const_var);

			bool const match = std::ranges::any_of(left_types,
				[&](Type const& type) { return night::contains(left_types, type); });

			if (!match) {
				throw NIGHT_COMPILE_ERROR(
					"left hand value of operator '" + binary_op.data + "' currently contains " + types_as_str(right_types) + ", but right hand value currently contains " + types_as_str(right_types),
					"operator '" + binary_op.data + "' can only be used to compare two equivalent non-class types");
			}

			//if (binary_op.type == BinaryOPNode::EQUAL)
				//const_type

			return { Type::BOOL };
		}

		case BinaryOPNode::DOT: {
			auto const& binary_op = std::get<BinaryOPNode>(expr->data);

			if (binary_op.right->type != ExprNode::CALL) {
				throw NIGHT_COMPILE_ERROR(
					"expected method call to the right of dot operator",
					night::format_method);
			}

			auto const& method = std::get<ValueCall>(binary_op.right->data);

			// find required types and return types based on method

			TypeContainer send_types, rtn_types;
			for (const auto& check_class : Parser::check_classes)
			{
				auto const it = check_class.second.methods.find(method.name);

				if (it == check_class.second.methods.end() ||
					it->second.is_void ||
					it->second.param_types.size() != method.param_exprs.size())
					continue;

				// type check method parameters to see if it matches

				bool valid = true;
				for (std::size_t a = 0; a < it->second.param_types.size() && valid; ++a)
				{
					TypeContainer arg_types = type_check_expr(
						scope, method.param_exprs[a], it->second.param_types[a]);

					if (arg_types.empty())
						continue;

					valid = std::ranges::any_of(
						it->second.param_types[a],
						[&](auto const& param_type)
							{ return night::contains(arg_types, param_type); }
					);
				}

				if (!valid)
					continue;

				// find required types

				if (check_class.first == "array")
					send_types.insert(send_types.end(), Type::ARR);
				else if (check_class.first == "string")
					send_types.insert(send_types.end(), Type::STR);

				rtn_types.insert(rtn_types.end(),
					it->second.rtn_types.begin(), it->second.rtn_types.end());
			}

			if (send_types.empty()) {
				throw NIGHT_COMPILE_ERROR(
					"method '" + method.name + "' does not exist within any class",
					"methods have to be defined before they are used");
			}

			auto const obj_types =
				type_check_expr(scope, binary_op.left, send_types);

			if (!night::contains(obj_types, Type::STR, Type::ARR)) {
				throw NIGHT_COMPILE_ERROR(
					"operator `" + binary_op.data + "` can only be used on types: `str`, `arr`, and `obj`",
					"left hand value of operator `" + binary_op.data + "` currently contains " + types_as_str(obj_types));
			}

			return rtn_types.empty()
				? required_types
				: rtn_types;
		}

		case BinaryOPNode::RANGE: {
			auto const left_types = type_check_expr(
				scope, binary_op.left, { Type::INT, Type::FLOAT });

			if (!night::contains(left_types, Type::INT, Type::FLOAT))
				throw_binary_type_err(binary_op, left_types, "left", "types `int` or `float`");

			auto const right_types = type_check_expr(
				scope, binary_op.right, { Type::INT, Type::FLOAT });

			if (!night::contains(right_types, Type::INT, Type::FLOAT))
				throw_binary_type_err(binary_op, right_types, "right", "types `int` or `float`");

			return TypeContainer{ Type::RNG };
		}
		}
	}
	default:
		throw std::runtime_error("Parser::type_check_expr(), missing node");
	}
}

void
Parser::check_call_types(
	std::vector<TypeContainer> const& param_types,
	std::vector<TypeContainer> const& arg_types,
	std::string const& func_name)
{
	assert(param_types.size() == arg_types.size());

	for (std::size_t a = 0; a < param_types.size(); ++a)
	{
		// parameter can be any type, so skip type checking
		if (param_types[a].empty() || arg_types[a].empty())
			continue;

		// checking to see if at least one argument type matches a parameter type

		bool const match = std::ranges::any_of(
			param_types[a],
			[&](auto const& param_type) {
				return night::contains(arg_types[a], param_type); }
		);

		if (!match) {
			throw NIGHT_COMPILE_ERROR(
				"for function call '" + func_name + ", argument number " + std::to_string(a + 1) +
					" must contain " + types_as_str(param_types[a]),
				"argument currently contains " + types_as_str(arg_types[a]));
		}
	}
}

std::string
Parser::types_as_str(TypeContainer const& var_types_set) const
{
	assert(!var_types_set.empty());

	std::vector<Type> var_types(var_types_set.begin(), var_types_set.end());

	std::string str_types = "";

	for (std::size_t a = 0; a < var_types.size() - 1; ++a)
		str_types += "`" + var_types[a].to_str() + "`, ";

	str_types = var_types.size() > 1
		? "types " + str_types + ", and "
		: "type " + str_types;

	str_types += "`" + var_types.back().to_str() + "`";

	return str_types;
}

inline void
Parser::throw_binary_type_err(
	BinaryOPNode  const& op,
	TypeContainer const& types,
	std::string   const& side,
	std::string   const& used_types) const
{
	throw NIGHT_COMPILE_ERROR(
		side + " hand value of operator '" + op.data + "' currently contains " + types_as_str(types),
		"operator '" + op.data + "' can only be used on " + used_types);
}

TypeContainer const Parser::all_types{
	Type::BOOL, Type::INT, Type::FLOAT, Type::STR, Type::ARR };