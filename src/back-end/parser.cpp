#include "../../include/back-end/parser.hpp"
#include "../../include/back-end/utils.hpp"
#include "../../include/back-end/token.hpp"
#include "../../include/back-end/stmt.hpp"
#include "../../include/error.hpp"

#include <functional>
#include <memory>
#include <algorithm>
#include <iterator>
#include <string>
#include <vector>
#include <unordered_set>

Parser::Parser(Lexer& lexer)
	: lexer(lexer)
{
	check_funcs = {
		make_check_function("print", { all_types }),
		make_check_function("input", {}, { Type::STR }),

		make_check_function("range",
			{ { Type::INT }, { Type::INT } },
			{ Type::ARR }),
			
		make_check_function("int",
			{ { Type::INT, Type::FLOAT, Type::STR } },
			{ Type::INT }),
		make_check_function("float",
			{ { Type::INT, Type::FLOAT, Type::STR } },
			{ Type::FLOAT }),
		make_check_function("str",
			{ { Type::BOOL, Type::INT, Type::FLOAT, Type::STR } },
			{ Type::STR }),

		make_check_function("system",
			{ { Type::STR } })
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

	Token token = lexer.eat(true);
	switch (token.type)
	{
	case TokenType::SET: {
		// validating statement

		if (lexer.eat(false).type != TokenType::VAR) {
			throw NIGHT_COMPILE_ERROR(
				"expected variable name after 'set' keyword",
				night::format_init, night::learn_variables);
		}

		std::string const var_name = lexer.get_curr(false).data;

		if (lexer.eat(false).type != TokenType::ASSIGN) {
			throw NIGHT_COMPILE_ERROR(
				"expected assignment operator after variable name",
				night::format_init, night::learn_variables);
		}

		if (scope.get_var(var_name) != nullptr) {
			throw NIGHT_COMPILE_ERROR(
				"variable '" + var_name + "' has already been defined",
				"variables can only be defined once, regardless of their scope",
				night::learn_variables);
		}
		if (check_funcs.contains(var_name)) {
			throw NIGHT_COMPILE_ERROR(
				"variable '" + var_name + "' has the same name as a function",
				"variable and function names must be unique",
				night::learn_variables);
		}
		if (check_classes.contains(var_name)) {
			throw NIGHT_COMPILE_ERROR(
				"variable '" + var_name + "' has the same name as a class",
				"variable and class names must be unique",
				night::learn_variables);
		}

		// parsing statement

		if (lexer.eat(false).type == TokenType::EOL) {
			throw NIGHT_COMPILE_ERROR(
				"expected expression after assignment operator",
				night::format_init, night::learn_variables);
		}

		auto [expr, types] = parse_expression(scope);

		if (lexer.get_curr(false).type != TokenType::EOL) {
			throw NIGHT_COMPILE_ERROR(
				"unexpected token '" + token.data + "' after expression",
				night::format_init, night::learn_variables);
		}

		scope.vars[var_name] = CheckVariable{ types };

		return Stmt{
			lexer.get_loc(), StmtType::INIT,
			StmtInit{ var_name, expr }
		};
	}
	case TokenType::VAR: {
		std::string const var_name = token.data;

		token = lexer.eat(false);
		if (token.type == TokenType::OPEN_BRACKET)
		{
			auto const check_func = check_funcs.find(var_name);
			if (check_func == check_funcs.end()) {
				throw NIGHT_COMPILE_ERROR(
					"function '" + var_name + "' is undefined",
					"functions must be defined before they are used",
					night::learn_functions);
			}

			// parsing arguments

			auto const [arg_exprs, arg_types] =
				parse_arguments(scope, check_func->first);

			// type checking arguments

			if (arg_types.size() != check_func->second.param_types.size()) {
				throw NIGHT_COMPILE_ERROR(
					"function call '" + check_func->first + "' has '" + std::to_string(arg_types.size()) + "' arguments",
					"function must be called with '" + std::to_string(check_func->second.param_types.size()) + "' arguments",
					night::learn_functions);
			}

			check_call_types(check_func->second.param_types,
				arg_types, check_func->first);

			return Stmt{
				lexer.get_loc(), StmtType::CALL,
				StmtCall{ check_func->first, arg_exprs }
			};
		}
		else
		{
			std::shared_ptr<ExprNode> var_expr(nullptr);
			bool contains_method = false;

			auto* const check_var = scope.get_var(var_name);
			if (check_var == nullptr) {
				throw NIGHT_COMPILE_ERROR(
					"variable '" + var_name + "' is undefined",
					"variables must be defined before they are used",
					night::learn_variables);
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
							night::format_method, night::learn_classes);
					}

					std::string const method_name = lexer.get_curr(false).data;

					if (lexer.eat(false).type != TokenType::OPEN_BRACKET) {
						throw NIGHT_COMPILE_ERROR(
							"expected open bracket after method '" + method_name + "'",
							night::format_method, night::learn_classes);
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
						auto const method = std::ranges::find_if(
							check_class.second.methods,
							[&](auto const& method) {
								return method.first == method_name &&
									method.second.param_types.size() == arg_exprs.size();
							}
						);

						if (method == check_class.second.methods.end())
							continue;

						try {
							check_call_types(method->second.param_types,
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
							"no matching function call '" + method_name + "' in class method",
							"methods have to be defined before they are used",
							night::learn_functions);
					}

					// adding expression node
					// since all the operators here are dot or subscript, we
					// can just keep building the AST up

					ValueCall const val_call{ method_name, arg_exprs };

					std::shared_ptr<ExprNode> const call_node =
						std::make_shared<ExprNode>(lexer.get_loc(), ExprNode::CALL, val_call);

					BinaryOPNode const op_expr{
						BinaryOPNode::DOT, ".", var_expr, call_node };

					std::shared_ptr<ExprNode> const op_node =
						std::make_shared<ExprNode>(lexer.get_loc(), ExprNode::BINARY_OP, op_expr);

					var_expr = op_node;
				}
				else if (token.type == TokenType::OPEN_SQUARE)
				{
					auto [expr, types] = parse_expression(scope, { Type::INT });

					if (lexer.get_curr(true).type != TokenType::CLOSE_SQUARE) {
						throw NIGHT_COMPILE_ERROR(
							"expected closing square bracket in subscript operator",
							night::format_subscript, night::learn_arrays);
					}
					if (!types.contains(Type::INT)) {
						throw NIGHT_COMPILE_ERROR(
							"subscript index must contain type 'int'",
							"index currently contains " + types_as_str(types),
							night::learn_arrays);
					}

					UnaryOPNode const sub_op{
						UnaryOPNode::SUBSCRIPT, "[]", var_expr };

					std::shared_ptr<ExprNode> const sub_expr =
						std::make_shared<ExprNode>(lexer.get_loc(), ExprNode::UNARY_OP, sub_op);

					var_expr = sub_expr;
				}
				else
				{
					throw NIGHT_COMPILE_ERROR(
						"unexpected token '" + token.data + "' in variable statement",
						"",
						night::learn_variables);
				}

				token = lexer.eat(false);
			}

			if (token.type == TokenType::ASSIGN)
			{
				if (contains_method) {
					throw NIGHT_COMPILE_ERROR(
						"method calls do not return references",
						"assigning values to return values is not allowed",
						night::learn_classes);
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

				return Stmt{
					lexer.get_loc(), StmtType::ASSIGN,
					StmtAssign{ assign_type, check_var->first, var_expr }
				};
			}
			else if (token.type == TokenType::EOL)
			{
				if (var_expr != nullptr && var_expr->type == ExprNode::UNARY_OP) {
					throw NIGHT_COMPILE_ERROR(
						"values are not valid statements",
						"",
						night::learn_variables);
				}

				return Stmt{
					lexer.get_loc(), StmtType::METHOD,
					StmtMethod{ var_expr }
				};
			}
			else
			{
				throw NIGHT_COMPILE_ERROR(
					"unexpected token '" + token.data + "' in variable statement",
					"",
					night::learn_variables);
			}
		}
	}
	case TokenType::IF: {
		auto const condition_expr = parse_condition(
			scope, "if", "if statement", night::learn_conditionals);

		// parsing body

		Scope if_scope{ scope };
		
		std::vector<Stmt> const body = parse_body(
			if_scope, "if conditional",
			night::format_if, night::learn_conditionals);

		std::vector<Conditional> conditionals{
			Conditional{ condition_expr, body } };

		while (true)
		{
			token = lexer.eat(false);

			Scope condition_scope{ scope };
			std::shared_ptr<ExprNode> condition_expr(nullptr);

			if (token.type == TokenType::IF)
			{
				condition_expr = parse_condition(
					scope, "else if", "else if statement", night::learn_conditionals);
			}
			else if (token.type != TokenType::OPEN_CURLY)
			{
				throw NIGHT_COMPILE_ERROR(
					"unexpected token '" + token.data + "' after else keyword",
					"else keyword must be followed by an if keyword or an opening curly bracket",
					night::learn_conditionals);
			}

			conditionals.push_back(Conditional{
				condition_expr,
				parse_body(condition_scope, "conditional",
					night::format_elif, night::learn_conditionals)
			});

			if (condition_expr == nullptr)
				break;
		}

		return Stmt{
			lexer.get_loc(), StmtType::IF,
			StmtIf{ conditionals }
		};
	}
	case TokenType::ELIF: {
		throw NIGHT_COMPILE_ERROR(
			"elif statement does not precede an if or elif statement",
			"elif statements must come after an if or an elif statement",
			night::learn_conditionals);
	}
	case TokenType::ELSE: {
		throw NIGHT_COMPILE_ERROR(
			"else statement does not precede an if or elif statement",
			"else statements must come after an if or an elif statement",
			night::learn_conditionals);
	}
	case TokenType::FN: {
		// validating statement

		if (lexer.eat(false).type != TokenType::VAR) {
			throw NIGHT_COMPILE_ERROR(
				"expected function name after 'def' keyword",
				night::format_fn, night::learn_functions);
		}

		std::string const func_name = lexer.get_curr(false).data;

		if (lexer.eat(false).type != TokenType::OPEN_BRACKET) {
			throw NIGHT_COMPILE_ERROR(
				"expected opening bracket after function name",
				night::format_fn, night::learn_functions);
		}

		if (scope.vars.contains(func_name)) {
			throw NIGHT_COMPILE_ERROR(
				"function '" + func_name + "' can not have the same name as a variable",
				"function and variable names must be unique",
				night::learn_functions);
		}
		if (check_funcs.contains(func_name)) {
			throw NIGHT_COMPILE_ERROR(
				"function '" + func_name + "' has already been defined",
				"functions can only be defined once",
				night::learn_functions);
		}
		if (check_classes.contains(func_name)) {
			throw NIGHT_COMPILE_ERROR(
				"function '" + func_name + "' can not have the same name as a class",
				"function and class names must be unique",
				night::learn_functions);
		}

		if (scope.upper != nullptr) {
			throw NIGHT_COMPILE_ERROR(
				"function '" + func_name + "' can not be defined inside of a local scope",
				"functions must be defined in the global scope",
				night::learn_functions);
		}

		// parsing parameters
		// also turns parameters into variables for definition checking the function body

		Scope func_scope{ scope };

		std::vector<std::string> param_names;

		while (true)
		{
			if (lexer.eat(false).type != TokenType::VAR) {
				throw NIGHT_COMPILE_ERROR(
					"expected variable names as function parameters",
					night::format_fn, night::learn_functions);
			}

			std::string const param_name = lexer.get_curr(false).data;

			if (Parser::check_funcs.contains(param_name)) {
				throw NIGHT_COMPILE_ERROR(
					"function parameter can not have the same name as a function",
					"function parameter names must be unique",
					night::learn_functions);
			}
			if (Parser::check_classes.contains(param_name)) {
				throw NIGHT_COMPILE_ERROR(
					"function parameter can not have the same name as a class",
					"function parameter names must be unique",
					night::learn_functions);
			}

			param_names.push_back(param_name);
			func_scope.vars[param_name] = CheckVariable{};

			Token const token = lexer.eat(false);

			if (token.type == TokenType::CLOSE_BRACKET)
				break;

			if (token.type != TokenType::COMMA) {
				throw NIGHT_COMPILE_ERROR(
					"expected command or closing bracket after parameter '" + param_name + "'",
					night::format_fn, night::learn_functions);
			}
		}

		if (lexer.eat(true).type != TokenType::OPEN_CURLY) {
			throw NIGHT_COMPILE_ERROR(
				"expected opening curly bracket after closing bracket",
				night::format_fn, night::learn_functions);
		}

		// define function before parsing body to ensure function is defined for recursion
		// and return types

		Parser::check_funcs[func_name] = CheckFunction();

		auto check_func = Parser::check_funcs.find(func_name);
		check_func->second.param_types.resize(param_names.size());

		// parsing body

		in_func = check_func;
		
		std::vector<Stmt> fn_stmts = parse_body(func_scope, "function definition",
			night::format_fn, night::learn_functions);

		in_func = Parser::check_funcs.end();

		if (lexer.eat(false).type != TokenType::EOL) {
			throw NIGHT_COMPILE_ERROR(
				"unexpected token '" + token.data + "' after closing curly bracket",
				night::format_fn, night::learn_functions);
		}

		// set check function to be void or not, and assign check function
		// parameters' types

		check_func->second.is_void = check_func->second.rtn_types.empty();

		for (std::size_t a = 0; a < check_func->second.param_types.size(); ++a)
			check_func->second.param_types[a] = func_scope.vars[param_names[a]].types;

		return Stmt{
			lexer.get_loc(), StmtType::FN,
			StmtFn{ func_name, param_names, fn_stmts }
		};
	}
	case TokenType::RETURN: {
		if (in_func == Parser::check_funcs.end()) {
			throw NIGHT_COMPILE_ERROR(
				"return statement is outside of a function",
				"return statements must be inside of a function",
				night::learn_functions);
		}

		auto const [expr, types] = parse_expression(scope);
		in_func->second.rtn_types.insert(types.begin(), types.end());

		return Stmt{
			lexer.get_loc(), StmtType::RETURN,
			StmtReturn{ expr }
		};
	}
	case TokenType::WHILE: {
		auto condition_expr = parse_condition(
			scope, "while", "while loop", night::learn_loops);

		if (lexer.peek(true).type == TokenType::EOL) {
			throw NIGHT_COMPILE_ERROR(
				"expected statement(s) after closing bracket",
				night::format_while, night::learn_loops);
		}

		ParserScope while_scope{ &scope };
		std::vector<Stmt> const stmts = parse_body(while_scope, "while loop",
			night::format_while, night::learn_loops);

		return Stmt{
			lexer.get_loc(), StmtType::WHILE,
			StmtWhile{ condition_expr, stmts }
		};
	}
	case TokenType::FOR: {
		// validating statement

		if (lexer.eat(false).type != TokenType::OPEN_BRACKET) {
			throw NIGHT_COMPILE_ERROR(
				"expected opening bracket after 'for' keyword",
				night::format_for, night::learn_loops);
		}
		if (lexer.eat(false).type != TokenType::VAR) {
			throw NIGHT_COMPILE_ERROR(
				"expected iterator after opening bracket",
				night::format_for, night::learn_loops);
		}

		std::string const it_name = lexer.get_curr(false).data;

		if (lexer.eat(false).type != TokenType::COLON) {
			throw NIGHT_COMPILE_ERROR(
				"expected colon after iterator",
				night::format_for, night::learn_loops);
		}
		if (lexer.eat(false).type == TokenType::EOL) {
			throw NIGHT_COMPILE_ERROR(
				"expected range after colon",
				night::format_for, night::learn_loops);
		}

		// parsing iterator and range

		auto [range_expr, range_types] = parse_expression(
			scope, { Type::STR, Type{ Type::ARR, Parser::all_types } });

		if (!range_types.contains(Type::STR) && !range_types.contains(Type::ARR)) {
			throw NIGHT_COMPILE_ERROR(
				"range currently contains " + types_as_str(range_types),
				"for loop range must contain type 'str' or 'arr'",
				night::learn_loops);
		}

		// parsing statement

		ParserScope for_scope{ &scope };
		for_scope.vars[it_name] = CheckVariable(range_types);

		if (lexer.peek(true).type == TokenType::EOL) {
			throw NIGHT_COMPILE_ERROR(
				"expected statement(s) after closing bracket",
				night::format_for, night::learn_loops);
		}

		return Stmt{
			lexer.get_loc(), StmtType::FOR,
			StmtFor{ it_name, range_expr, parse_body(for_scope, "for loop",
				night::format_for, night::learn_loops) }
		};
	}
	default: {
		throw NIGHT_COMPILE_ERROR(
			"unknown syntax",
			"no clue what you did here sorry :/",
			night::learn_learn);
	}
	}
}

std::vector<Stmt> Parser::parse_body(
	ParserScope& scope,
	std::string const& stmt_name,
	std::string const& stmt_format,
	std::string const& stmt_learn)
{
	if (lexer.peek(true).type != TokenType::OPEN_CURLY)
		return { parse_statement(scope) };

	lexer.eat(true);

	std::vector<Stmt> stmts;
	while (lexer.peek(true).type != TokenType::CLOSE_CURLY &&
		   lexer.peek(true).type != TokenType::_EOF)
		stmts.push_back(parse_statement(scope));

	if (lexer.eat(true).type == TokenType::_EOF) {
		throw NIGHT_COMPILE_ERROR(
			"expected closing bracket at the end of " + stmt_name + " body",
			stmt_format, stmt_learn);
	}

	return stmts;
}

std::shared_ptr<ExprNode> Parser::parse_condition(
	ParserScope& scope,
	std::string const& stmt_name,
	std::string const& stmt_format,
	std::string const& stmt_learn)
{
	// validating statement

	std::string const loop_name = lexer.get_curr(false).data;

	if (lexer.eat(false).type != TokenType::OPEN_BRACKET) {
		throw NIGHT_COMPILE_ERROR(
			"expected opening bracket after '" + loop_name + "' keyword",
			stmt_format, stmt_learn);
	}
	if (lexer.eat(false).type == TokenType::EOL) {
		throw NIGHT_COMPILE_ERROR(
			"expected an expression after opening bracket",
			stmt_format, stmt_learn);
	}

	// parsing condition

	auto [condition_expr, condition_types] =
		parse_expression(scope, { Type::BOOL });

	if (lexer.get_curr(false).type != TokenType::CLOSE_BRACKET) {
		throw NIGHT_COMPILE_ERROR(
			"expected closing bracket after " + stmt_name + " condition",
			stmt_format, stmt_learn);
	}
	if (!condition_types.contains(Type::BOOL)) {
		throw NIGHT_COMPILE_ERROR(
			stmt_name + " condition currently contains " + types_as_str(condition_types),
			"condition must contain type 'bool'",
			stmt_learn);
	}

	return condition_expr;
}

auto Parser::parse_arguments(
	ParserScope& scope,
	std::string const& func_name)
{
	ExprContainer call_exprs;
	std::vector<TypeContainer> call_types;

	while (true)
	{
		auto const [expr, types] = parse_expression(scope);
		if (expr == nullptr) {
			throw NIGHT_COMPILE_ERROR(
				"expected argument in function call '" + func_name + "'",
				night::format_call, night::learn_functions);
		}

		call_exprs.push_back(expr);
		call_types.push_back(types);

		Token const token = lexer.get_curr(false);

		if (token.type == TokenType::CLOSE_BRACKET)
			return std::make_tuple(call_exprs, call_types);

		if (token.type == TokenType::EOL) {
			throw NIGHT_COMPILE_ERROR(
				"expected closing bracket in function call '" + func_name + "'",
				night::format_call, night::learn_functions);
		}
		if (token.type != TokenType::COMMA) {
			throw NIGHT_COMPILE_ERROR(
				"unexpected token '" + token.data + "' in function call '" + func_name + "'",
				night::format_call, night::learn_functions);
		}
	}
}

std::tuple<std::shared_ptr<ExprNode>, Parser::TypeContainer>
Parser::parse_expression(
	ParserScope& scope,
	TypeContainer const& required_types)
{
	static std::unordered_map<std::string, UnaryOPNode::T> const unary_op_convers{
		{ "-", UnaryOPNode::NEGATIVE },
		{ "!", UnaryOPNode::NOT }
	};

	static std::unordered_map<std::string, BinaryOPNode::T> const binary_op_convers{
		{ "+", BinaryOPNode::PLUS },
		{ "-", BinaryOPNode::MINUS },
		{ "*", BinaryOPNode::TIMES },
		{ "/", BinaryOPNode::DIVIDE },
		{ "%", BinaryOPNode::MOD },
		{ ">", BinaryOPNode::GREATER },
		{ "<", BinaryOPNode::SMALLER },
		{ ">=", BinaryOPNode::GREATER_EQ },
		{ "<=", BinaryOPNode::SMALLER_EQ },
		{ "==", BinaryOPNode::EQUAL },
		{ "!=", BinaryOPNode::NOT_EQUAL },
		{ "||", BinaryOPNode::OR },
		{ "&&", BinaryOPNode::AND }
	};

	std::shared_ptr<ExprNode> root(nullptr), protect(nullptr);

	Token token = lexer.eat(false);
	while (true)
	{
		if (token.is_value())
		{
			// traveling tree

			std::shared_ptr<ExprNode> curr(root), prev(nullptr);
			while (curr != nullptr)
			{
				if (curr->is_value()) {
					throw NIGHT_COMPILE_ERROR(
						"expected operator between values",
						"",
						night::learn_learn);
				}

				prev = curr;
				curr = curr->travel_ast();
			}

			// constructing node

			if (prev == nullptr)
				prev = std::make_shared<ExprNode>();
			else
				prev->travel_ast() = std::make_shared<ExprNode>();

			std::shared_ptr<ExprNode> expr_node = prev->travel_ast();

			expr_node->loc = lexer.get_loc();

			switch (token.type)
			{
			case TokenType::BOOL_T: {
				expr_node->type = ExprNode::LITERAL;
				expr_node->data = ValueLiteral{ ValueLiteral::BOOL, token.data == "true" };

				break;
			}
			case TokenType::INT_T: {
				expr_node->type = ExprNode::LITERAL;
				expr_node->data = ValueLiteral{ ValueLiteral::INT, std::stoi(token.data) };

				break;
			}
			case TokenType::FLOAT_T: {
				expr_node->type = ExprNode::LITERAL;
				expr_node->data = ValueLiteral{ ValueLiteral::FLOAT, std::stof(token.data) };

				break;
			}
			case TokenType::STR_T: {
				expr_node->type = ExprNode::LITERAL;
				expr_node->data = ValueLiteral{ ValueLiteral::STR, token.data };

				break;
			}
			case TokenType::VAR: {
				if (lexer.peek(false).type == TokenType::OPEN_BRACKET)
				{
					std::string const func_name = lexer.get_curr(false).data;

					auto const [arg_exprs, arg_types] =
						parse_arguments(scope, func_name);

					expr_node->type = ExprNode::CALL;
					expr_node->data = ValueCall(
						func_name, arg_exprs);
				}
				else if (lexer.peek(false).type == TokenType::OPEN_SQUARE)
				{
					expr_node->type = ExprNode::VARIABLE;
					expr_node->data = ValueVar{ lexer.get_curr(false).data };

					lexer.eat(false);

					//traveling tree

					std::shared_ptr<ExprNode> curr(root), prev(nullptr);
					while (!token.is_value() && curr != protect)
					{
						prev = curr;

						std::string const op_data = curr->type == ExprNode::UNARY_OP
							? std::get<UnaryOPNode>(curr->data).data
							: std::get<BinaryOPNode>(curr->data).data;

						if (!higher_precedence(op_data, "[]"))
							break;

						curr = curr->travel_ast();
					}

					// constructing node

					auto const [expr, types] = parse_expression(scope);
					if (lexer.get_curr(false).type != TokenType::CLOSE_SQUARE) {
						throw NIGHT_COMPILE_ERROR(
							"expected closing square bracket at the end of subscript operator",
							"",
							night::learn_arrays);
					}
					if (!types.contains(Type::INT)) {
						throw NIGHT_COMPILE_ERROR(
							"subscript operator currently contains " + types_as_str(types),
							"subscript operator must contains type 'int'",
							night::learn_arrays);
					}

					auto const pos = unary_op_convers.find(token.data);
					assert(pos != unary_op_convers.end());

					std::shared_ptr<ExprNode> const expr_node = std::make_shared<ExprNode>(
						lexer.get_loc(),
						ExprNode::UNARY_OP,
						UnaryOPNode{ pos->second, "[]", curr, expr }
					);

					if (prev != nullptr)
						root = expr_node;
					else
						prev->travel_ast() = expr_node;
				}
				else
				{
					expr_node->type = ExprNode::VARIABLE;
					expr_node->data = ValueVar{ lexer.get_curr(false).data };
				}

				break;
			}
			case TokenType::OPEN_SQUARE: {
				ExprContainer arr_exprs;

				while (true)
				{
					auto const expr = std::get<0>(parse_expression(scope));
					if (expr == nullptr) {
						throw NIGHT_COMPILE_ERROR(
							"expected element in array",
							night::format_array, night::learn_arrays);
					}

					arr_exprs.push_back(expr);

					token = lexer.get_curr(false);

					if (token.type == TokenType::CLOSE_SQUARE)
						break;

					if (token.type == TokenType::EOL) {
						throw NIGHT_COMPILE_ERROR(
							"expected closing square bracket in array",
							night::format_array, night::learn_arrays);
					}
					if (token.type != TokenType::COMMA) {
						throw NIGHT_COMPILE_ERROR(
							"unexpected token '" + token.data + "' in array",
							night::format_array, night::learn_arrays);
					}
				}

				expr_node->type = ExprNode::ARRAY;
				expr_node->data = ValueArray{ arr_exprs };

				break;
			}
			default: {
				throw std::runtime_error(
					__FILE__ + std::string("\n") +
					std::to_string(__LINE__) + "missing value");
			}
			}
		}
		else if (token.type == TokenType::OPEN_BRACKET)
		{
			std::shared_ptr<ExprNode> curr(root), prev(nullptr);
			while (curr != nullptr)
			{
				if (curr->is_value()) {
					throw NIGHT_COMPILE_ERROR(
						"expected operator between value and opening bracket",
						"",
						night::learn_learn);
				}

				prev = curr;
				curr = curr->travel_ast();
			}

			if (curr == nullptr)
			{
				root = std::get<0>(parse_expression(scope, required_types));
				protect = root;
			}
			else
			{
				prev->travel_ast() = std::get<0>(parse_expression(scope, required_types));
				protect = prev->travel_ast();
			}

			if (lexer.get_curr(false).type != TokenType::CLOSE_BRACKET) {
				throw NIGHT_COMPILE_ERROR(
					"expected closing bracket in expression",
					"",
					night::learn_learn);
			}
		}
		else if (token.type == TokenType::UNARY_OP || token.type == TokenType::BINARY_OP)
		{
			//traveling tree

			std::shared_ptr<ExprNode> curr(root), prev(nullptr);
			while (!token.is_value() && curr != protect)
			{
				prev = curr;

				std::string const op_data = curr->type == ExprNode::UNARY_OP
					? std::get<UnaryOPNode>(curr->data).data
					: std::get<BinaryOPNode>(curr->data).data;

				if (!higher_precedence(op_data, token.data))
					break;

				curr = curr->travel_ast();
			}

			// constructing node

			std::shared_ptr<ExprNode> expr_node = std::make_shared<ExprNode>();
			expr_node->loc = lexer.get_loc();

			if (token.type == TokenType::UNARY_OP)
			{
				auto const pos = unary_op_convers.find(token.data);
				assert(pos != unary_op_convers.end());

				expr_node->type = ExprNode::UNARY_OP;
				expr_node->data = UnaryOPNode{ pos->second, token.data, curr };
			}
			else
			{
				auto const op_type = binary_op_convers.find(token.data);
				assert(op_type != binary_op_convers.end());

				expr_node->type = ExprNode::BINARY_OP;
				expr_node->data = BinaryOPNode{ op_type->second, token.data, curr, nullptr };
			}

			if (prev != nullptr)
				root = expr_node;
			else
				prev->travel_ast() = expr_node;
		}
		else
		{
			return std::make_tuple(root, type_check_expr(scope, root, required_types));
		}

		lexer.eat(false);
	}
}

bool Parser::higher_precedence(
	std::string const& op1,
	std::string const& op2) const
{
	std::vector<std::unordered_set<std::string> > const operators{
		{ "||", "&&" },
		{ "==", "!=" },
		{ ">", "<", ">=", "<=" },
		{ "+", "-" },
		{ "*", "/", "%" },
		{ "!" },
		{ "[]", "." },
	};

	std::size_t pre1, pre2;
	for (std::size_t a = 0; a < operators.size(); ++a)
	{
		if (operators[a].contains(op1))
			pre1 = a;
		if (operators[a].contains(op2))
			pre2 = a;
	}

	return pre1 > pre2;
}

Parser::TypeContainer Parser::type_check_expr(
	ParserScope& scope,
	std::shared_ptr<ExprNode> const& expr,
	TypeContainer const& required_types) const
{
	switch (expr->type)
	{
	case ExprNode::LITERAL: {
		ValueLiteral const& val = std::get<ValueLiteral>(expr->data);

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
		ValueArray const& val = std::get<ValueArray>(expr->data);

		TypeContainer elem_types;
		for (auto const& elem_expr : val.elem_exprs)
		{
			TypeContainer const elem_type = type_check_expr(scope, elem_expr);
			elem_types.insert(elem_type.begin(), elem_type.end());
		}

		return { Type(Type::ARR, elem_types) };
	}
	case ExprNode::VARIABLE: {
		ValueVar const& val = std::get<ValueVar>(expr->data);

		auto* const check_var = scope.get_var(val.name);
		if (check_var == nullptr) {
			throw NIGHT_COMPILE_ERROR(
				"variable '" + val.name + "' is undefined",
				"variables must be defined before they are used",
				night::learn_variables);
		}

		// if the variable is a function parameter
		if (check_var->second.types.empty())
			check_var->second.types = required_types;

		return check_var->second.types;
	}
	case ExprNode::CALL: {
		ValueCall const& val = std::get<ValueCall>(expr->data);

		auto const check_func_it = check_funcs.find(val.func_name);
		if (check_func_it == check_funcs.end()) {
			throw NIGHT_COMPILE_ERROR(
				"function '" + val.func_name + "' is not defined",
				"functions have to be defined before they are used",
				night::learn_functions);
		}

		CheckFunction const& check_func = check_func_it->second;

		if (val.param_exprs.size() != check_func.param_types.size()) {
			throw NIGHT_COMPILE_ERROR(
				"function '" + val.func_name + "' is called with '" + std::to_string(val.param_exprs.size()) + "' argument(s)",
				"function '" + val.func_name + "' can only be called with '" + std::to_string(check_func.param_types.size()) + "' argument(s)",
				night::learn_functions);
		}

		// checking if function is void

		if (check_func.is_void) {
			throw NIGHT_COMPILE_ERROR(
				"function '" + val.func_name + "' does not have a return value",
				"functions must have a return value to be used in expression",
				night::learn_functions);
		}

		for (std::size_t a = 0; a < check_func.param_types.size(); ++a)
		{
			TypeContainer arg_types = type_check_expr(
				scope, val.param_exprs[a], check_func.param_types[a]);

			if (arg_types.empty())
				continue;

			bool const match = std::ranges::any_of(
				check_func.param_types[a],
				[&](auto const& param_type) {
					return arg_types.contains(param_type); }
			);

			if (!match) {
				throw NIGHT_COMPILE_ERROR(
					"argument number " + std::to_string(a + 1) + " for function " + val.func_name+ " must contain " + types_as_str(check_func.param_types[a]),
					"argument number " + std::to_string(a + 1) + " currently contains " + types_as_str(arg_types),
					night::learn_functions);
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
				"operator '" + unary_op.data + "' can only be used on " + used_types,
				night::learn_operators);
		};

		UnaryOPNode const& unary_op = std::get<UnaryOPNode>(expr->data);
		switch (unary_op.type)
		{
		case UnaryOPNode::NEGATIVE: {
			TypeContainer const types = type_check_expr(
				scope, unary_op.value, { Type::INT, Type::FLOAT });

			if (!types.contains(Type::INT) && !types.contains(Type::FLOAT))
				throw_unary_op_err(unary_op, types, "types 'int' or 'float'");

			return { Type::INT, Type::FLOAT };
		}
		case UnaryOPNode::NOT: {
			TypeContainer const types = type_check_expr(
				scope, unary_op.value, { Type::BOOL });

			if (!types.contains(Type::BOOL))
				throw_unary_op_err(unary_op, types, "type 'bool'");

			return { Type::BOOL };
		}
		case UnaryOPNode::SUBSCRIPT: {
			TypeContainer const types = type_check_expr(
				scope, unary_op.value, { Type::INT, Type::FLOAT });

			TypeContainer rtn_types;
			if (types.contains(Type::STR))
				rtn_types.insert(Type::STR);
			if (auto const pos = std::find(types.begin(), types.end(), Type::ARR); pos != types.end())
				rtn_types.insert(pos->elem_types.begin(), pos->elem_types.end());

			if (rtn_types.empty())
				throw_unary_op_err(unary_op, types, "types 'str' or 'arr'");

			return rtn_types;
		}
		}
	}
	case ExprNode::BINARY_OP: {
		BinaryOPNode const& binary_op = std::get<BinaryOPNode>(expr->data);

		auto const throw_binary_op_err = [&](
			TypeContainer const& types,
			std::string   const& side,
			std::string   const& used_types)
		{
			throw NIGHT_COMPILE_ERROR(
				side + " hand value of operator '" + binary_op.data + "' currently contains " + types_as_str(types),
				"operator '" + binary_op.data + "' can only be used on " + used_types,
				night::learn_operators);
		};

		TypeContainer rtn_num_types;

		switch (binary_op.type)
		{
		case BinaryOPNode::PLUS:
		case BinaryOPNode::MINUS:
		case BinaryOPNode::TIMES:
		case BinaryOPNode::DIVIDE:
		case BinaryOPNode::MOD:
			rtn_num_types = { Type::INT, Type::FLOAT };

		case BinaryOPNode::GREATER:
		case BinaryOPNode::GREATER_EQ:
		case BinaryOPNode::SMALLER:
		case BinaryOPNode::SMALLER_EQ: {
			TypeContainer const left_types = type_check_expr(
				scope, binary_op.left, { Type::INT, Type::FLOAT });

			if (!left_types.contains(Type::INT) && !left_types.contains(Type::FLOAT))
				throw_binary_op_err(left_types, "left", "types 'int' or 'float'");

			TypeContainer const right_types = type_check_expr(
				scope, binary_op.right, { Type::INT, Type::FLOAT });

			if (!right_types.contains(Type::INT) && !right_types.contains(Type::FLOAT))
				throw_binary_op_err(right_types, "right", "types 'int' or 'float'");

			return rtn_num_types.empty()
				? TypeContainer{ Type::BOOL }
				: rtn_num_types;
		}

		case BinaryOPNode::AND:
		case BinaryOPNode::OR: {
			TypeContainer const left_types = type_check_expr(
				scope, binary_op.left, { Type::BOOL });

			if (!left_types.contains(Type::BOOL))
				throw_binary_op_err(left_types, "left", "type 'bool'");

			TypeContainer const right_types = type_check_expr(
				scope, binary_op.left, { Type::BOOL });

			if (!right_types.contains(Type::BOOL))
				throw_binary_op_err(right_types, "left", "type 'bool'");

			return { Type::BOOL };
		}

		case BinaryOPNode::EQUAL:
		case BinaryOPNode::NOT_EQUAL: {
			TypeContainer const& left_types = type_check_expr(
				scope, binary_op.left, { Type::BOOL });

			TypeContainer const& right_types = type_check_expr(
				scope, binary_op.right, { Type::BOOL });

			bool const match = std::ranges::any_of(left_types,
				[&](Type const& type) { return left_types.contains(type); });

			if (!match) {
				throw NIGHT_COMPILE_ERROR(
					"left hand value of operator '" + binary_op.data + "' currently contains " + types_as_str(right_types) + ", but right hand value currently contains " + types_as_str(right_types),
					"operator '" + binary_op.data + "' can only be used to compare two equivalent non-class types",
					night::learn_operators);
			}

			return { Type::BOOL };
		}

		case BinaryOPNode::DOT: {
			BinaryOPNode const& binary_op = std::get<BinaryOPNode>(expr->data);

			if (binary_op.right->type != ExprNode::CALL) {
				throw NIGHT_COMPILE_ERROR(
					"expected method call to the right of dot operator",
					"method calls must be in this format: `object.method()`",
					night::learn_classes);
			}

			ValueCall const& method =
				std::get<ValueCall>(binary_op.right->data);

			// find required types and return types based on method

			TypeContainer send_types, rtn_types;
			for (const auto& check_class : Parser::check_classes)
			{
				auto const it = check_class.second.methods.find(method.func_name);

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
							{ return arg_types.contains(param_type); }
					);
				}

				if (!valid)
					continue;

				// find required types

				if (check_class.first == "array")
					send_types.insert(Type::ARR);
				else if (check_class.first == "string")
					send_types.insert(Type::STR);

				rtn_types.insert(
					it->second.rtn_types.begin(),
					it->second.rtn_types.end());
			}

			if (send_types.empty()) {
				throw NIGHT_COMPILE_ERROR(
					"method '" + method.func_name + "' does not exist within any class",
					"methods have to be defined before they are used",
					night::learn_classes);
			}

			TypeContainer const obj_types = type_check_expr(
				scope, binary_op.left, send_types);

			if (!obj_types.contains(Type::STR) && !obj_types.contains(Type::ARR)) {
				throw NIGHT_COMPILE_ERROR(
					"operator '" + binary_op.data + "' can only be used on types: 'str', 'arr', and 'obj'",
					"left hand value of operator '" + binary_op.data + "' currently contains " + types_as_str(obj_types),
					night::learn_type_checking);
			}

			return rtn_types.empty()
				? required_types
				: rtn_types;
		}
		}
	}
	}
}

void Parser::check_call_types(
	std::vector<TypeContainer> const& param_types,
	std::vector<TypeContainer> const& arg_types,
	std::string const& func_name)
{
	assert(param_types.size() == arg_types.size());

	for (std::size_t a = 0; a < param_types.size(); ++a)
	{
		// parameter can be any type, so skip type checking
		if (param_types[a].empty())
			continue;

		// checking to see if at least one argument type matches a parameter type

		bool const match = std::ranges::any_of(
			param_types[a],
			[&](auto const& param_type) {
				return arg_types[a].contains(param_type); }
		);

		if (!match) {
			throw NIGHT_COMPILE_ERROR(
				"for function call '" + func_name + ", argument number " + std::to_string(a + 1) +
					" must contain " + types_as_str(param_types[a]),
				"argument currently contains " + types_as_str(arg_types[a]),
				night::learn_functions);
		}
	}
}

std::string Parser::types_as_str(TypeContainer const& var_types_set) const
{
	assert(!var_types_set.empty());

	std::vector<Type> var_types(var_types_set.begin(), var_types_set.end());

	std::string str_types = "";

	for (std::size_t a = 0; a < var_types.size() - 1; ++a)
		str_types += "'" + var_types[a].to_str() + "', ";

	str_types = var_types.size() > 1
		? "types " + str_types + ", and "
		: "type " + str_types;

	str_types += "'" + var_types.back().to_str() + "'";

	return str_types;
}


Parser::TypeContainer const Parser::all_types{
	Type::BOOL, Type::INT, Type::FLOAT, Type::STR, Type::ARR };

std::pair<std::string const, Parser::CheckFunction> Parser::make_check_function(
	std::string const& name,
	std::vector<TypeContainer> const& params,
	TypeContainer			   const& rtn_types)
{
	return { name, { params, rtn_types } };
}

std::pair<std::string const, Parser::CheckClass> Parser::make_check_class(
	std::string const& name,

	CheckVariableContainer const& vars,
	CheckFunctionContainer const& methods)
{
	return { name, { vars, methods } };
}



Parser::Type::Type(T _type)
	: type(_type) {}

Parser::Type::Type(T _type, TypeContainer const& _elem_types)
	: type(_type), elem_types(_elem_types) {}

std::string Parser::Type::to_str() const
{
	switch (type)
	{
	case T::BOOL:
		return "bool";
	case T::INT:
		return "int";
	case T::FLOAT:
		return "float";
	case T::STR:
		return "str";
	case T::ARR:
		return "arr";
	}
}