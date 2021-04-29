#include "../../include/back-end/parser.hpp"
#include "../../include/back-end/utils.hpp"
#include "../../include/back-end/token.hpp"
#include "../../include/error.hpp"

#include <fstream>
#include <memory>
#include <algorithm>
#include <iterator>
#include <string>
#include <vector>
#include <unordered_set>

bool parse_statement(
	Lexer& lexer,
	std::shared_ptr<Scope>& scope
) {
	Token token = lexer_eat(lexer, false);
	switch (token.type)
	{
	case TokenType::SET: {
		// validating statement

		if (token = lexer_eat(lexer, false); token.type != TokenType::VARIABLE) {
			throw NIGHT_COMPILE_ERROR(
				"expected variable name after 'set' keyword",
				"variable initializations must be in this format: `set variable = expression`",
				night::learn_variables, lexer.loc);
		}

		std::string var_name = token.data;

		if (token = lexer_eat(lexer, false); token.type != TokenType::ASSIGN) {
			throw NIGHT_COMPILE_ERROR(
				"expected assignment operator after variable name",
				"variable initializations must be in this format: `set variable = expression`",
				night::learn_variables, lexer.loc);
		}

		if (scope->vars.contains(var_name)) {
			throw NIGHT_COMPILE_ERROR(
				"variable '" + var_name + "' has already been defined",
				"variables can only be defined once, regardless of their scope",
				night::learn_variables, lexer.loc);
		}
		if (Parser::check_funcs.contains(var_name)) {
			throw NIGHT_COMPILE_ERROR(
				"variable '" + var_name + "' has the same name as a function",
				"variable and function names must be unique",
				night::learn_variables, lexer.loc);
		}
		if (Parser::check_classes.contains(var_name)) {
			throw NIGHT_COMPILE_ERROR(
				"variable '" + var_name + "' has the same name as a class",
				"variable and class names must be unique",
				night::learn_variables, lexer.loc);
		}

		// parsing statement

		auto [expr, types] = parse_expression(lexer);
		if (expr == nullptr) {
			throw NIGHT_COMPILE_ERROR(
				"expected expression after assignment operator",
				"variable initializations must be in this format: 'set variable = expression'",
				night::learn_variables, lexer.loc);
		}

		if (token = lexer_curr(lexer, false); token.type != TokenType::EOL) {
			throw NIGHT_COMPILE_ERROR(
				"unexpected token '" + token.data + "' after expression",
				"variable initializations must be in this format: `set variable = expression`",
				night::learn_variables, lexer.loc);
		}

		scope->vars[var_name] = CheckVariable{ types, expr };
		return true;
	}
	case TokenType::VARIABLE: {
		const std::string var_name = token.data;

		token = lexer_eat(lexer, false);
		if (token.type == TokenType::OPEN_BRACKET)
		{
			auto  check_func = Parser::check_funcs.find(var_name);
			if (check_func == Parser::check_funcs.end()) {
				throw NIGHT_COMPILE_ERROR(
					"function '" + var_name + "' is undefined",
					"functions must be defined before they are used",
					night::learn_functions, lexer.loc);
			}

			// parsing arguments

			ExprContainer arg_exprs;
			std::vector<TypeContainer> arg_types;

			try {
				auto args = parse_arguments(lexer, scope);

				arg_exprs = std::get<0>(args);
				arg_types = std::get<1>(args);
			}
			catch (const night::error_wrapper& e) {
				if (e.type == "arg") {
					throw NIGHT_COMPILE_ERROR(
						"expected argument in function call '" + var_name + "'",
						"function calls must be in this format: `function(arg1, arg2)`",
						night::learn_functions, lexer.loc);
				}
				if (e.type == "eol") {
					throw NIGHT_COMPILE_ERROR(
						"expected closing bracket in function call '" + var_name + "'",
						"function calls must be in this format: `function(arg1, arg2)`",
						night::learn_functions, lexer.loc);
				}
				else {
					throw NIGHT_COMPILE_ERROR(
						"unexpected token '" + e.type + "' in function call '" + var_name + "'",
						"function calls must be in this format: `function(arg1, arg2)`",
						night::learn_functions, lexer.loc);
				}
			}

			// type checking arguments

			if (check_func->second.params.size() != arg_types.size()) {
				throw NIGHT_COMPILE_ERROR(
					"function call '" + check_func->first + "' has '" + std::to_string(arg_types.size()) + "' arguments",
					"function '" + check_func->first + "' is defined with '" + std::to_string(check_func->second.params.size()) + "' parameters",
					night::learn_functions, lexer.loc);
			}

			for (int a = 0; a < check_func->second.params.size(); ++a)
			{
				// parameter can be any type, so skip type checking
				if (check_func->second.params[a].empty())
					continue;

				for (auto const& check_param : check_func->second.params[a])
				{
					if (std::find(arg_types[a].begin(), arg_types[a].end(),
						check_param) == arg_types[a].end()) {
						throw NIGHT_COMPILE_ERROR(
							"argument number " + std::to_string(a + 1) + " must contain " + get_types_as_str(check_func->second.params[a]),
							"argument number " + std::to_string(a + 1) + " currently contains " + get_types_as_str(arg_types[a]),
							night::learn_functions, lexer.loc);
					}
				}
			}

			scope->stmts.push_back(Stmt{
				lexer.loc, StmtType::FUNC_CALL,
				StmtFuncCall{ check_func, arg_exprs }
			});
		}
		else
		{
			std::shared_ptr<ExprNode> method_expr(nullptr);

			auto* const check_variable = get_variable(scope, var_name);
			if (check_variable == nullptr) {
				throw NIGHT_COMPILE_ERROR(
					"variable '" + var_name + "' is undefined",
					"variables must be defined before they are used",
					night::learn_variables, lexer.loc);
			}

			while (true)
			{
				if (token.type == TokenType::DOT)
				{
					if (token = lexer_eat(lexer, false); token.type != TokenType::VARIABLE) {
						throw NIGHT_COMPILE_ERROR(
							"expected variable name after dot operator",
							"method statements must be in this format: `object.method()`",
							night::learn_classes, lexer.loc);
					}

					const std::string& method_name = token.data;

					if (token = lexer_eat(lexer, false); token.type != TokenType::OPEN_BRACKET) {
						throw NIGHT_COMPILE_ERROR(
							"expected open bracket after method '" + method_name + "'",
							"method statements must be in this format: `object.method()`",
							night::learn_classes, lexer.loc);
					}

					// parse arguments

					ExprContainer arg_exprs;
					std::vector<TypeContainer> arg_types;

					try {
						auto args = parse_arguments(lexer, scope);

						arg_exprs = std::get<0>(args);
						arg_types = std::get<1>(args);
					}
					catch (const night::error_wrapper& e) {
						if (e.type == "arg") {
							throw NIGHT_COMPILE_ERROR(
								"expected argument in method call '" + var_name + "'",
								"method calls must be in this format: `object.method(arg1, arg2)`",
								night::learn_classes, lexer.loc);
						}
						if (e.type == "eol") {
							throw NIGHT_COMPILE_ERROR(
								"expected closing bracket in method call '" + var_name + "'",
								"method calls must be in this format: `object.method(arg1, arg2)`",
								night::learn_classes, lexer.loc);
						}
						else {
							throw NIGHT_COMPILE_ERROR(
								"unexpected token '" + e.type + "' in method call '" + var_name + "'",
								"method calls must be in this format: `object.method(arg1, arg2)`",
								night::learn_classes, lexer.loc);
						}
					}

					// type check arguments

					std::vector<CheckFunction> check_funcs;
					for (auto& check_class : Parser::check_classes)
					{
						auto method = std::ranges::find_if(
							check_class.second.methods,
							[&](auto& method) {
								return method.first == method_name &&
									method.second.params.size() == arg_exprs.size();
							}
						);

						if (method == check_class.second.methods.end())
							continue;

						for (int a = 0; a < method->second.params.size(); ++a)
						{
							// parameter can be any type, so skip type checking
							if (method->second.params[a].empty())
								continue;

							for (auto const& check_param : method->second.params[a])
							{
								if (std::find(arg_types[a].begin(), arg_types[a].end(),
									check_param) == arg_types[a].end()) {
									throw NIGHT_COMPILE_ERROR(
										"argument number " + std::to_string(a + 1) + " must contain " + get_types_as_str(method->second.params[a]),
										"argument number " + std::to_string(a + 1) + " currently contains " + get_types_as_str(arg_types[a]),
										night::learn_functions, lexer.loc);
								}
							}
						}

						check_funcs.push_back(method);
					}

					if (check_funcs.empty()) {
						throw NIGHT_COMPILE_ERROR("");
					}

					// adding expression node

					ExprCall const call{ method_name, arg_exprs };

					std::shared_ptr<ExprNode> const call_expr =
						std::make_shared<ExprNode>(lexer.loc, ExprNode::CALL, call);

					ExprBinaryOP const dot_op{
						BinaryOPType::DOT, ".", method_expr, call_expr };

					std::shared_ptr<ExprNode> const op_expr =
						std::make_shared<ExprNode>(lexer.loc, ExprNode::BINARY_OP, dot_op);

					method_expr = op_expr;
				}
				else if (token.type == TokenType::OPEN_SQUARE)
				{
					auto [expr, types] =
						parse_expression(lexer, { ValueType::INT });

					if (lexer_curr(lexer, true).type != TokenType::CLOSE_SQUARE) {
						throw NIGHT_COMPILE_ERROR(
							"expected closing square bracket in subscript operator",
							"subscript operators must be in this format: `array[index]`",
							night::learn_arrays, lexer.loc);
					}
					if (!night::contains(types, ValueType::INT)) {
						throw NIGHT_COMPILE_ERROR(
							"subscript index must contain type 'int'",
							"index currently contains " + get_types_as_str(types),
							night::learn_arrays, lexer.loc);
					}

					ExprUnaryOP const sub_op{
						UnaryOPType::SUBSCRIPT, "[]", method_expr };

					std::shared_ptr<ExprNode> const sub_expr =
						std::make_shared<ExprNode>(lexer.loc, ExprNode::UNARY_OP, sub_op);

					method_expr = sub_expr;
				}
				else
				{

				}
			}

			if (token.type == TokenType::ASSIGN)
			{
				if (method_expr != nullptr) {
					throw NIGHT_COMPILE_ERROR(
						"method calls do not return references",
						"assigning values to value is not allowed",
						night::learn_classes);
				}

				auto [expr, types] = parse_expression(lexer);

				StmtAssign stmt_assign;
				stmt_assign.check_var = check_variable->second;
				stmt_assign.assign_expr = expr;

				scope->stmts.push_back(Stmt{
					lexer.loc, StmtType::ASSIGN,
					StmtAssign{ 0, check_variable->second }
				});
			}
			else if (token.type == TokenType::EOL)
			{

			}
			else
			{

			}
		}
	}
	
	case TokenType::IF: {
		// validating statement

		if (token = lexer_eat(lexer, false); token.type != TokenType::OPEN_BRACKET) {
			throw NIGHT_COMPILE_ERROR(
				"expected opening bracket after 'if' keyword",
				"if statements must be in this format: 'if (condition) {}'",
				night::learn_conditionals, lexer.loc);
		}
		if (token = lexer_eat(lexer, true); token.type == TokenType::EOL) {
			throw NIGHT_COMPILE_ERROR(
				"expected an expression after opening bracket",
				"if statements must be in this format: 'if (condition) {}'",
				night::learn_conditionals, lexer.loc);
		}

		// parsing condition

		auto [condition_expr, condition_types] =
			parse_expression(lexer, { ValueType::BOOL });

		if (!night::contains(condition_types, ValueType::BOOL)) {
			throw NIGHT_COMPILE_ERROR(
				"if statement condition must contain type: 'bool'",
				"condition currently contains " + get_types_as_str(condition_types),
				night::learn_conditionals, lexer.loc);
		}

		// parsing body

		std::shared_ptr<Scope> if_scope = std::make_shared<Scope>(scope);
		parse_body(lexer, if_scope);

		scope->stmts.push_back(Stmt{
			lexer.loc, StmtType::IF,
			StmtIf{ { Conditional{ condition_expr, if_scope } } }
		});
	}
	case TokenType::ELSE: {
		token = lexer_eat(lexer, false);
		switch (token.type)
		{
		case TokenType::IF: {
			// validating statement

			token = lexer_eat(lexer, false);
			if (token.type != TokenType::OPEN_BRACKET) {
				throw NIGHT_COMPILE_ERROR(
					"expected open bracket after if keyword",
					"else if statements must be in this format: `else if (condition) {}`",
					night::learn_conditionals, lexer.loc);
			}

			token = lexer_eat(lexer, true);
			if (token.type == TokenType::EOL) {
				throw NIGHT_COMPILE_ERROR(
					"expected open bracket after if keyword",
					"else if statements must be in this format: `else if (condition) {}`",
					night::learn_conditionals, lexer.loc);
			}

			if (scope->stmts.empty() || scope->stmts.back().type != StmtType::IF) {
				throw NIGHT_COMPILE_ERROR(
					"else if statement does not precede an if or else if statement",
					"else if statements must come after an if or an else if statement",
					night::learn_conditionals, lexer.loc);
			}

			// parsing condition

			auto [condition_expr, condition_types] =
				parse_expression(lexer, { ValueType::BOOL });

			if (!night::contains(condition_types, ValueType::BOOL)) {
				throw NIGHT_COMPILE_ERROR(
					"else if statement condition must contain type: 'bool'",
					"condition currently contains " + get_types_as_str(condition_types),
					night::learn_conditionals, lexer.loc);
			}

			// parsing body
			
			StmtIf* const if_stmt = &std::get<StmtIf>(scope->stmts.back().data);

			token = lexer_eat(lexer, true);
			if (token.type != TokenType::OPEN_CURLY) {
				throw NIGHT_COMPILE_ERROR(
					"expected open curly bracket after if keyword",
					"else if statements must be in this format: 'else if (condition) {}'",
					night::learn_conditionals, lexer.loc);
			}

			std::shared_ptr<Scope> if_else_scope = std::make_shared<Scope>(scope);

			parse_body(lexer, if_else_scope);

			// pushing statement

			if_stmt->chains.push_back(Conditional{ condition_expr, if_else_scope });
			return false;
		}
		case TokenType::OPEN_CURLY: {
			if (scope->stmts.empty() || scope->stmts.back().type != StmtType::IF) {
				throw NIGHT_COMPILE_ERROR(
					"else if statement does not precede an if or else if statement",
					"else if statements must come after an if or an else if statement",
					night::learn_conditionals, lexer.loc);
			}

			StmtIf* const if_stmt = &std::get<StmtIf>(scope->stmts.back().data);

			// parsing body

			if (token = lexer_eat(lexer, true); token.type != TokenType::OPEN_CURLY) {
				throw NIGHT_COMPILE_ERROR(
					"expected open curly bracket after 'else' keyword",
					"else statements must be in this format: `else {}`",
					night::learn_conditionals, lexer.loc);
			}

			std::shared_ptr<Scope> else_scope = std::make_shared<Scope>(scope);

			parse_body(lexer, else_scope);

			// pushing statement

			if_stmt->chains.push_back(Conditional{ nullptr, else_scope });
			return false;
		}
		default: {
			throw NIGHT_COMPILE_ERROR(
				"unexpected token '" + token.data + "' after else keyword",
				"else keyword must be followed by an if keyword or an opening curly bracket",
				night::learn_conditionals, lexer.loc);
		}
		}
	}
	
	case TokenType::DEF: {
		// validating statement

		if (token = lexer_eat(lexer, false); token.type != TokenType::VARIABLE) {
			throw NIGHT_COMPILE_ERROR(
				"expected function name after 'def' keyword",
				"function definitions must be in this format: `def function (parameters) {}`",
				night::learn_functions, lexer.loc);
		}
		
		std::string const func_name = token.data;
		
		if (token = lexer_eat(lexer, false); token.type != TokenType::OPEN_BRACKET) {
			throw NIGHT_COMPILE_ERROR(
				"expected opening bracket after function name",
				"function definitions must be in this format: `def function(parameters) {}`",
				night::learn_functions, lexer.loc);
		}
		
		if (scope->vars.contains(func_name)) {
			throw NIGHT_COMPILE_ERROR(
				"function '" + func_name + "' can not have the same name as a variable",
				"function and variable names must be unique",
				night::learn_functions, lexer.loc);
		}
		if (Parser::check_funcs.contains(func_name)) {
			throw NIGHT_COMPILE_ERROR(
				"function '" + func_name + "' has already been defined",
				"functions can only be defined once",
				night::learn_functions, lexer.loc);
		}
		if (Parser::check_classes.contains(func_name)) {
			throw NIGHT_COMPILE_ERROR(
				"function '" + func_name + "' can not have the same name as a class",
				"function and class names must be unique",
				night::learn_functions, lexer.loc);
		}
		
		if (scope->upper_scope != nullptr) {
			throw NIGHT_COMPILE_ERROR(
				"function '" + func_name + "' can not be defined inside of a local scope",
				"functions must be defined in the global scope",
				night::learn_functions, lexer.loc);
		}

		// parsing parameters
		// also turns parameters into variables for definition checking the function body

		std::shared_ptr<Scope> func_scope = std::make_shared<Scope>(scope);

		std::vector<std::string> param_names;

		while (true)
		{
			if (token = lexer_eat(lexer, false); token.type != TokenType::VARIABLE) {
				throw NIGHT_COMPILE_ERROR(
					"expected variable names as function parameters",
					"function definitions must be in this format: `def function(param1, param2) {}`",
					night::learn_functions, lexer.loc);
			}
			
			const std::string param_name = token.data;

			if (Parser::check_funcs.contains(param_name)) {
				throw NIGHT_COMPILE_ERROR(
					"function parameter can not have the same name as a function",
					"function parameter names must be unique",
					night::learn_functions, lexer.loc);
			}
			if (Parser::check_classes.contains(param_name)) {
				throw NIGHT_COMPILE_ERROR(
					"function parameter can not have the same name as a class",
					"function parameter names must be unique",
					night::learn_functions, lexer.loc);
			}

			param_names.push_back(param_name);
			func_scope->vars[param_name] = CheckVariable{};

			token = lexer_eat(lexer, false);

			if (token.type == TokenType::CLOSE_BRACKET)
				break;

			if (token.type != TokenType::COMMA) {
				throw NIGHT_COMPILE_ERROR(
					"expected command or closing bracket after parameter '" + param_name + "'",
					"function definitions must be in this format: `def function(parameters) {}`",
					night::learn_functions, lexer.loc);
			}
		}

		if (token = lexer_eat(lexer, true); token.type != TokenType::OPEN_CURLY) {
			throw NIGHT_COMPILE_ERROR(
				"expected opening curly bracket after closing bracket",
				"function definitions must be in this format: `def function (parameters) {}`",
				night::learn_functions, lexer.loc);
		}

		// define function before parsing body to ensure function is defined for recursion
		// and return types

		Parser::check_funcs[func_name] = CheckFunction();
		auto check_func = Parser::check_funcs.find(func_name);
		check_func->second.params.resize(param_names.size());

		// parsing body

		parse_body(lexer, func_scope);

		// set check function to be void or not, and assign check function
		// parameters' types

		check_func->second.is_void = check_func->second.rtn_types.empty();

		for (std::size_t a = 0; a < check_func->second.params.size(); ++a)
			check_func->second.params[a] = func_scope->vars[param_names[a]].types;

		return true;
	}
	case TokenType::RETURN: {
		// a note on the function scope:
		/*
		// make sure the check function is pushed to the check functions array
		// before making the recursive call to extract the body
		//
		// if that doesn't happen then this won't work, since here it climbs up
		// scopes in order to find the function scope
		*/

		const Scope* function_scope = scope->upper_scope.get();
		while (function_scope->upper_scope != nullptr)
			function_scope = function_scope->upper_scope.get();

		if (function_scope->stmts.back().type != StmtType::) {
			throw NIGHT_COMPILE_ERROR(
				"return statement is outside of a function",
				"return statements must be inside of a function",
				night::learn_functions);
		}



		// a note on void functions:
		/*
		// to determine if a check function is void or not, the presence of
		// return statements are analyzed
		//
		// if a function doesn't have a return statement, then it is void
		*/

		const FunctionDef& function_def =
			std::get<FunctionDef>(function_scope->stmts.back().data);

		// parsing return expression

		auto [expr, types] = parse_expression(lexer);

		if (is_reachable(scope))
			Parser::check_funcs[function_def.name].rtn_types.insert(types);

		scope->stmts.push_back(
			Stmt{ lexer.loc, StmtType::RETURN, StmtReturn{ expr } });
	}

	case TokenType::WHILE: {
		// validating statement

		if (token = lexer_eat(lexer, false); token.type != TokenType::OPEN_BRACKET) {
			throw NIGHT_COMPILE_ERROR(
				"expected opening bracket after 'while' keyword",
				"while loop statements must be in this format: 'while (condition) {}'",
				night::learn_loops, lexer.loc);
		}

		// parsing condition

		auto [condition_expr, condition_types] =
			parse_expression(lexer, { ValueType::BOOL });

		if (!night::contains(condition_types, ValueType::BOOL)) {
				throw NIGHT_COMPILE_ERROR(
					"while loop condition must contain type 'bool'",
					"condition currently contains " + get_types_as_str(condition_types),
					night::learn_loops, lexer.loc);
			}

		std::shared_ptr<Scope> while_scope = std::make_shared<Scope>(nullptr);
		parse_body(lexer, while_scope);
		
		scope->stmts.push_back(Stmt{
			lexer.loc, StmtType::WHILE,
			StmtWhile{ condition_expr, while_scope }
		});
	}
	case TokenType::FOR: {
		// validating statement

		if (token = lexer_eat(lexer, false); token.type != TokenType::OPEN_BRACKET) {
			throw NIGHT_COMPILE_ERROR(
				"expected opening bracket after 'for' keyword",
				"for loops must be in this format: 'for (iterator : range) {}'",
				night::learn_loops, lexer.loc);
		}
		if (token = lexer_eat(lexer, false); token.type != TokenType::VARIABLE) {
			throw NIGHT_COMPILE_ERROR(
				"expected iterator after opening bracket",
				"for loops must be in this format: 'for (iterator : range) {}'",
				night::learn_loops, lexer.loc);
		}
		
		std::string const it_name = token.data;
		
		if (token = lexer_eat(lexer, false); token.type != TokenType::COLON) {
			throw NIGHT_COMPILE_ERROR(
				"expected colon after iterator",
				"for loops must be in this format: 'for (iterator : range) {}'",
				night::learn_loops, lexer.loc);
		}

		// parsing iterator and range

		auto [range_expr, range_types] = parse_expression(
			lexer,
			{
				ExprValue{ ValueType::STR },
				ExprValue{ ValueType::ARR, {}, Parser::all_types }
			}
		);

		if (range_expr == nullptr) {
			throw NIGHT_COMPILE_ERROR(
				"expected range after colon",
				"for loops must be in this format: 'for (iterator : range) {}'",
				night::learn_loops, lexer.loc);
		}

		if (!night::contains(range_types, ValueType::STR, ValueType::ARR)) {
			throw NIGHT_COMPILE_ERROR(
				"for loop range must contain type 'str' or 'arr'",
				"range currently contains " + get_types_as_str(range_types),
				night::learn_loops, lexer.loc);
		}

		// parsing statement

		std::shared_ptr<Scope> for_scope = std::make_shared<Scope>(scope);
		for_scope->vars[it_name] = CheckVariable(range_types);

		parse_body(lexer, for_scope);

		scope->stmts.push_back(Stmt{
			lexer.loc, StmtType::FOR,
			StmtFor{ it_name, range_expr, for_scope }
		});
	}

	case TokenType::CLOSE_CURLY: {
		//if (token_peek(lexer, false).type != TokenType::EOL) {

		//}
		
		return scope->upper_scope != nullptr;
	}
	case TokenType::EOL: {
		return true;
	}

	default: {
		throw NIGHT_COMPILE_ERROR(
			"unknown syntax",
			"no clue what you did here sorry :/",
			night::learn_learn);
	}
	}

	/*
	// delete these
	if (tokens.size() >= 2 && tokens[0].type == TokenType::VARIABLE && tokens[1].type == TokenType::ASSIGN)
	{
		if (tokens.size() == 2)
			throw NIGHT_COMPILE_ERROR("expected expression after assignment operator", "variable assignments must be in this format: 'variable_name = expression'", Learn::VARIABLES);

		// definition and type checking variable

		auto* const check_variable = get_variable(current_scope, tokens[0].data);
		if (check_variable == nullptr)
			throw NIGHT_COMPILE_ERROR("variable '" + tokens[0].data + "' is undefined", "variables must be defined before they are used", Learn::VARIABLES);

		if (!check_variable->second.types.empty())
		{
			if (tokens[1].data == "+=" && !find_num_types(check_variable->second.types) &&
				!check_variable->second.types.contains(VariableType::STRING))
				throw NIGHT_COMPILE_ERROR("assignment operator '+=' can only be used on variables of type 'int', 'float', or 'str'", "variable '" + check_variable->first + " currently contains " + get_var_types_as_str(check_variable->second.types), Learn::VARIABLES);
			if (tokens[1].data != "=" && tokens[1].data != "+=" && !find_num_types(check_variable->second.types))
				throw NIGHT_COMPILE_ERROR("assignment operator '" + tokens[1].data + "' can only be used on variables of type 'int' or 'float'", "variable '" + tokens[0].data + " currently contains " + get_var_types_as_str(check_variable->second.types), Learn::VARIABLES);
		}

		// extracting and type checking expression

		const std::vector<Value> values = TokensToValues(
			std::vector<Token>(tokens.begin() + 2, tokens.end()));

		const std::shared_ptr<Expression> assign_expr =
			ValuesToExpression(values);

		const VariableTypeContainer assign_types =
			type_check_expr(current_scope, assign_expr);

		if (tokens[1].data == "+=" && !find_num_types(assign_types) &&
			!assign_types.contains(VariableType::STRING))
			throw NIGHT_COMPILE_ERROR("assignment operator '+=' can only assign expressions of type 'int', 'float', or 'str'", "expression has " + get_var_types_as_str(assign_types), Learn::VARIABLES);
		if (tokens[1].data != "=" && tokens[1].data != "+=" && !find_num_types(assign_types) &&
			!assign_types.contains(VariableType::STRING))
			throw NIGHT_COMPILE_ERROR("assignment operator '" + tokens[1].data + "' can only assign expressions of type 'int' or 'float'", "expression has " + get_var_types_as_str(assign_types), Learn::VARIABLES);

		// determining assignment type

		Assignment assignment{ {}, check_variable->first, assign_expr };
		switch (tokens[1].data[0])
		{
		case '=':
			assignment.assign_type = Assignment::ASSIGN;
			break;
		case '+':
			assignment.assign_type = Assignment::PLUS;
			break;
		case '-':
			assignment.assign_type = Assignment::MINUS;
			break;
		case '*':
			assignment.assign_type = Assignment::TIMES;
			break;
		case '/':
			assignment.assign_type = Assignment::DIVIDE;
			break;
		case '%':
			assignment.assign_type = Assignment::MOD;
			break;
		default:
			assert(false);
		}

		// adding types to check variable and pushing statement

		if (assignment.assign_type != Assignment::PLUS ||
			(assignment.assign_type == Assignment::PLUS &&
				!check_variable->second.types.contains(VariableType::STRING)))
		{
			if (assign_types.contains(VariableType::INT))
				check_variable->second.types.insert({ VariableType::INT });
			if (assign_types.contains(VariableType::FLOAT))
				check_variable->second.types.insert({ VariableType::FLOAT });
		}

		current_scope->statements.push_back(Statement{
			loc, StatementType::ASSIGNMENT, assignment });
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VARIABLE && tokens[1].type == TokenType::OPEN_SQUARE)
	{
		// extracting and type checking subscript indices

		VariableTypeContainer index_types;
		std::vector<std::shared_ptr<Expression> > index_exprs;

		int open_square_count = 0;
		auto it = tokens.begin() + 1;
		for (auto start = tokens.begin() + 2; it != tokens.end(); ++it)
		{
			if (it->type == TokenType::OPEN_SQUARE)
			{
				open_square_count++;
			}
			else if (it->type == TokenType::CLOSE_SQUARE)
			{
				open_square_count--;
				if (open_square_count == 0)
				{
					if (!index_types.empty() &&
						!index_types.contains(VariableType::ARRAY) &&
						!index_types.contains(VariableType::STRING))
						throw NIGHT_COMPILE_ERROR("subscript operator can only be used on variables of type 'str' or 'arr'", "variable currently contains " + get_var_types_as_str(index_types), Learn::ARRAYS);

					const std::vector<Value> values =
						TokensToValues(std::vector<Token>(start, it));

					index_exprs.push_back(ValuesToExpression(values));
					index_types = type_check_expr(current_scope, index_exprs.back());

					if (!find_num_types(index_types))
						throw NIGHT_COMPILE_ERROR("array indices must be of type 'int'", "index currently contains " + get_var_types_as_str(index_types), Learn::ARRAYS);

					start = std::next(it, 2);
				}
			}
			else if (it->type == TokenType::ASSIGN)
			{
				if (open_square_count < 0)
					throw NIGHT_COMPILE_ERROR("missing " + std::to_string(open_square_count) + " open square bracket(s)");
				if (open_square_count > 0)
					throw NIGHT_COMPILE_ERROR("missing " + std::to_string(open_square_count) + " close square bracket(s)");

				break;
			}
		}

		if (it == tokens.end())
			throw NIGHT_COMPILE_ERROR("expected assignment operator in element assignment", "element assignments must be in this form: 'variable[index] = expression'", Learn::ARRAYS);

		// definition checking variable

		auto* const check_variable = get_variable(current_scope, tokens[0].data);
		if (check_variable == nullptr)
			throw NIGHT_COMPILE_ERROR("variable '" + tokens[1].data + "' is undefined", "variables have to be defined before they are used", Learn::VARIABLES);

		// type checking variable

		//  !night::contains(check_variable->second.types, VariableType::ARRAY, VariableType::STRING)
		if (!check_variable->second.types.contains(VariableType::ARRAY) &&
			!check_variable->second.types.contains(VariableType::STRING))
			throw NIGHT_COMPILE_ERROR("subscript operator '[]' can only be used on variables of type 'str' or 'arr'", "variable '" + check_variable->first + "' currently contains " + get_var_types_as_str(check_variable->second.types), Learn::ARRAYS);

		// parsing expression

		const std::vector<Value> values = TokensToValues(
			std::vector<Token>(std::next(it, 1), tokens.end()));

		const std::shared_ptr<Expression> assign_expr =
			ValuesToExpression(values);

		const VariableTypeContainer assign_types =
			type_check_expr(current_scope, assign_expr);

		check_variable->second.types.insert(
			assign_types.begin(), assign_types.end());

		// pushing statement

		current_scope->statements.push_back(Statement{
			loc, StatementType::ELEMENT,
			Element{ tokens[0].data, index_exprs, assign_expr }
			});
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VARIABLE && tokens[1].type == TokenType::OPERATOR && tokens[1].data == ".")
	{
		auto* const check_variable = get_variable(current_scope, tokens[0].data);
		if (check_variable == nullptr)
			throw NIGHT_COMPILE_ERROR("variable '" + tokens[0].data + "' is undefined", "variables must be defined before they are used", Learn::VARIABLES);

		// methods on arrays!!
		// create a function to parse methods and arrays??

		for (auto it = tokens.begin() + 2; it != tokens.end(); ++it)
		{
			if (it->type != TokenType::VARIABLE)
				throw NIGHT_COMPILE_ERROR("expected variable name after operator '.'", "method calls must be in this format: 'object.method()'");

			// make sure to also add types to check_variable when using push() method



			if (std::next(it, 1) == tokens.end())
				break;
			if (std::next(it, 1)->type != TokenType::OPERATOR)
				throw NIGHT_COMPILE_ERROR("expected dot operator after method '" + it->data + "'", "method calls must be in this format: 'object.method()'");
		}

		const std::shared_ptr<Expression> assign_expr =
			ValuesToExpression(TokensToValues(tokens));

		//type_check_expr(current_scope, assign_expr);

		// pushing statement

		current_scope->statements.push_back(Statement{
			loc, StatementType::METHOD_CALL,
			MethodCall{ tokens[0].data, assign_expr }
			});
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VARIABLE && tokens[1].type == TokenType::OPEN_BRACKET)
	{
		// extracting types and expressions for arguments

		std::vector<VariableTypeContainer>   argument_types;
		std::vector<std::shared_ptr<Expression> > argument_expressions;

		int open_bracket_count = 0;
		auto it = tokens.begin() + 1;
		for (auto start = tokens.begin() + 2; it != tokens.end(); ++it)
		{
			if (it->type == TokenType::OPEN_BRACKET)
				open_bracket_count++;
			else if (it->type == TokenType::CLOSE_BRACKET)
				open_bracket_count--;

			if ((it->type == TokenType::COMMA && open_bracket_count == 1) ||
				(it->type == TokenType::CLOSE_BRACKET && open_bracket_count == 0))
			{
				if (it == tokens.begin() + 2 && it->type == TokenType::CLOSE_BRACKET)
					break;

				const std::vector<Value> values =
					TokensToValues(std::vector<Token>(start, it));

				argument_expressions.push_back(ValuesToExpression(values));
				argument_types.push_back(type_check_expr(current_scope, argument_expressions.back()));

				start = std::next(it, 1);

				if (it->type == TokenType::CLOSE_BRACKET)
					break;
			}
		}

		if (open_bracket_count < 0)
			throw NIGHT_COMPILE_ERROR("missing opening bracket in function call", std::to_string(open_bracket_count) + " closing bracket(s) do not have an associated opening bracket", Learn::FUNCTIONS);
		if (open_bracket_count > 0)
			throw NIGHT_COMPILE_ERROR("missing closing bracket in function call", std::to_string(open_bracket_count) + " opening bracket(s) do not have an associated closing bracket", Learn::FUNCTIONS);
		if (it == tokens.end())
			throw NIGHT_COMPILE_ERROR("unexpected token(s) after function call", "function calls must be in this format: 'function(parameters)'", Learn::FUNCTIONS);

		// fetching check function

		auto check_function = check_functions.find(tokens[0].data);
		if (check_function == check_functions.end())
			throw NIGHT_COMPILE_ERROR("function '" + tokens[0].data + "' is undefined", "functions must be defined before they are called", Learn::FUNCTIONS);

		// definition and type checking arguments with parameters

		if (argument_types.size() != check_function->second.parameters.size())
			throw NIGHT_COMPILE_ERROR("function '" + check_function->first + "' can only be called with '" + std::to_string(check_function->second.parameters.size()) + "' argument(s)", "function '" + check_function->first + "' was called with '" + std::to_string(argument_types.size()) + "' argument(s)", Learn::FUNCTIONS);

		for (std::size_t a = 0; a < argument_types.size(); ++a)
		{
			if (argument_types[a].empty())
				continue;

			/*
			bool match = false;
			for (auto param_type : check_function->second.parameters[a])
			{
				if (argument_types[a].find(param_type) != argument_types[a].end())
				{
					match = true;
					break;
				}
			}
			//

			bool match = std::all_of(
				check_function->second.parameters[a].begin(),
				check_function->second.parameters[a].end(),
				[&](const auto& param_type) {
					return argument_types[a].contains(param_type);
				}
			);

			if (!match)
				throw NIGHT_COMPILE_ERROR("argument number '" + std::to_string(a + 1) + "' for function call '" + check_function->first + "' can only contain " + get_var_types_as_str(check_function->second.parameters[a]) + "'", "argument number '" + std::to_string(a + 1) + "' currently contains " + get_var_types_as_str(argument_types[a]), Learn::TYPE_CHECKING);
		}

		// pushing statement

		current_scope->statements.push_back(Statement{
			loc, StatementType::FUNCTION_CALL,
			FunctionCall{ tokens[0].data, argument_expressions }
			});
	}	
	*/
}

void parse_body(Lexer& lexer, std::shared_ptr<Scope>& scope)
{
	if (lexer_peek(lexer, true).type == TokenType::OPEN_CURLY)
	{
		bool stmt_parsed = true;
		while (stmt_parsed)
			stmt_parsed = parse_statement(lexer, scope);
	}
	else
	{
		parse_statement(lexer, scope);
	}
}

std::tuple<std::shared_ptr<ExprNode>, TypeContainer>
parse_expression(Lexer& lexer, const std::vector<ValueType>& required_types)
{
	Token token = lexer_eat(lexer, false);
	if (token.type == TokenType::EOL)
		return std::make_tuple(nullptr, TypeContainer());

	std::shared_ptr<ExprNode> root =
		std::make_shared<ExprNode>(lexer.loc, token.type, token.data);

	std::shared_ptr<ExprNode> protect = nullptr;

	int open_bracket = 0, open_square = 0;

	token = lexer_eat(lexer, false);
	while (!expr_reached_end())
	{
		if (token.type == TokenType::OPEN_BRACKET)
		{
			// RECURSION!!!!!!!!

			// protect
		}

		std::shared_ptr<ExprNode> curr = root;
		std::shared_ptr<ExprNode> prev = curr;
		
		std::shared_ptr<ExprNode> node =
			std::make_shared<ExprNode>(lexer.loc, token.type, token.data);

		if (node->type == ExprNode::VALUE || node->type == ExprNode::VAR)
		{
			while (curr != nullptr)
			{
				prev = curr;

				switch (curr->type)
				{
				case ExprNode::UNARY_OP: {
					curr = std::get<ExprUnaryOP>(curr->data).value;
					break;
				}
				case ExprNode::BINARY_OP: {
					curr = std::get<ExprBinaryOP>(curr->data).left;
					break;
				}
				case ExprNode::VALUE: {
					throw NIGHT_COMPILE_ERROR(
						"expected operator before value'" + std::get<ExprValue>(curr->data) + "' in expression",
						"",
						night::learn_learn, lexer.loc);
				}
				default: {
					throw NIGHT_COMPILE_ERROR(
						"unexpected " + (curr->type == ExprNode::VAR ? "variable" : "value") + " '" + );
				}
				}
			}

			prev 
		}
	}

	return std::make_tuple(root, type_check_expr(root));
}

std::tuple<std::vector<std::shared_ptr<ExprNode> >, std::vector<TypeContainer> >
parse_arguments(
	Lexer& lexer,
	std::shared_ptr<Scope>& curr_scope
) {
	std::vector<std::shared_ptr<ExprNode> > call_exprs;
	std::vector<TypeContainer> call_types;

	while (true)
	{
		const auto [expr, types] = parse_expression(lexer);
		if (expr == nullptr)
			throw night::error_wrapper("arg");

		call_exprs.push_back(expr);
		call_types.push_back(types);

		const Token token = lexer_curr(lexer, true);

		if (token.type == TokenType::CLOSE_BRACKET)
			return std::make_tuple(call_exprs, call_types);

		if (token.type == TokenType::EOL)
			throw night::error_wrapper("eol");
		if (token.type != TokenType::COMMA)
			throw night::error_wrapper(token.data);
	}
}

TypeContainer parser_type_check_expr(
	std::shared_ptr<Scope> const& curr_scope,
	std::shared_ptr<ExprNode> const& node,
	TypeContainer const& required_types
) {
	switch (node->type)
	{
	case ExprNode::VALUE: {
		ExprValue const& expr_val = std::get<ExprValue>(node->data);

		if (expr_val.type == ValueType::ARR)
		{
			TypeContainer elem_types;
			for (auto const& elem : expr_val.elem_types)
			{
				TypeContainer const elem_type =
					parser_type_check_expr(curr_scope, elem);

				elem_types.insert(elem_type.begin(), elem_type.end());
			}

			return elem_types;
		}

		return { expr_val };
	}
	case ExprNode::VAR: {
		std::string const& var_name = std::get<ExprVar>(node->data).name;

		auto* const check_var = get_variable(curr_scope, var_name);
		if (check_var == nullptr) {
			throw NIGHT_COMPILE_ERROR(
				"variable '" + var_name + "' is undefined",
				"variables must be defined before they are used",
				night::learn_variables, node->loc);
		}

		// if the variable is a function parameter
		if (check_var->second.types.empty())
		{
			if (!is_reachable(curr_scope))
				return required_types;

			check_var->second.types = required_types;
		}

		return check_var->second.types;
	}
	case ExprNode::CALL: {
		auto const& func_node = std::get<ExprCall>(node->data);

		auto const check_func = Parser::check_funcs.find(func_node.name);
		if (check_func == Parser::check_funcs.end()) {
			throw NIGHT_COMPILE_ERROR(
				"function '" + func_name + "' is undefined",
				"function must be defined before they are used",
				night::learn_functions, node->loc);
		}

		if (check_func->second.is_void) {
			throw NIGHT_COMPILE_ERROR(
				"function '" + check_func->first + "' does not have a return value",
				"functions must have a return value to be used in expression",
				night::learn_functions, node->loc);
		}
		if (func_node.params.size() != check_func->second.params.size()) {
			throw NIGHT_COMPILE_ERROR(
				"function '" + check_func->first + "' is called with '" + std::to_string(func_node.params.size()) + "' argument(s)",
				"function '" + check_func->first + "' can only be called with '" + std::to_string(check_func->second.params.size()) + "' argument(s)",
				night::learn_functions, node->loc);
		}

		// type checking function parameters

		for (std::size_t a = 0; a < check_func->second.params.size(); ++a)
		{
			// parameter can be any type, so skip type checking
			if (check_func->second.params[a].empty())
				continue;

			for (ExprValue const& check_param : check_func->second.params[a])
			{
				if (std::find(func_node.params[a].begin(), func_node.params[a].end(),
					check_param) == func_node.params[a].end()) {
					throw NIGHT_COMPILE_ERROR(
						"argument number " + std::to_string(a + 1) + " must contain " + get_types_as_str(check_func->second.params[a]),
						"argument number " + std::to_string(a + 1) + " currently contains " + get_types_as_str(func_node.params[a]),
						night::learn_functions, node->loc);
				}
			}
		}

		// a note about empty return types:
		/*
		// if a function's return types can't be deduced when the function is
		// defined, then the return type is treated as if it were any type
		//
		// returning 'all_types' here wouldn't be wrong, but to keep the types
		// to a minimum (only containing necessary types), 'required_types' is
		// returned instead
		*/
		return check_func->second.rtn_types.empty()
			? required_types
			: check_func->second.rtn_types;
	}
	case ExprNode::UNARY_OP: {
		ExprUnaryOP const& unary_op = std::get<ExprUnaryOP>(node->data);

		switch (unary_op.type)
		{
		case UnaryOPType::NEGATIVE: {
			TypeContainer const types = 
				parser_type_check_expr(curr_scope, unary_op.value,
				{ ExprValue{ ValueType::INT }, ExprValue{ ValueType::FLOAT } });

			if (!night::contains(types, ValueType::INT, ValueType::FLOAT))
				THROW_ERR_TYPE_CHECK_UNARY("types 'int' or 'float'");

			return {
				ExprValue{ ValueType::INT },
				ExprValue{ValueType::FLOAT } };
		}
		case UnaryOPType::NOT: {
			TypeContainer const types =
				parser_type_check_expr(curr_scope, unary_op.value,
				{ ExprValue{ ValueType::BOOL } });

			if (!night::contains(types, ValueType::BOOL))
				THROW_ERR_TYPE_CHECK_UNARY("type 'bool'");

			return { ExprValue{ ValueType::BOOL } };
		}
		case UnaryOPType::SUBSCRIPT: {
			TypeContainer const types =
				parser_type_check_expr(curr_scope, unary_op.value,
				{ ExprValue{ ValueType::INT }, ExprValue{ ValueType::FLOAT } });

			TypeContainer rtn_types;
			if (night::contains(types, ValueType::STR))
				rtn_types.push_back(ExprValue{ ValueType::STR });
			if (auto pos = night::find(types, ValueType::ARR); pos != nullptr)
				rtn_types.insert(rtn_types.end(), pos->elem_types.begin(), pos->elem_types.end());

			if (rtn_types.empty())
				THROW_ERR_TYPE_CHECK_UNARY("types 'str' or 'arr'");

			return rtn_types;
		}
		}
	}
	case ExprNode::BINARY_OP: {
		ExprBinaryOP const& binary_op = std::get<ExprBinaryOP>(node->data);

		switch (binary_op.type)
		{
		case BinaryOPType::PLUS:
		case BinaryOPType::MINUS:
		case BinaryOPType::TIMES:
		case BinaryOPType::DIVIDE:
		case BinaryOPType::MOD: {
			parser_type_check_num_expr(curr_scope, node);
			return {
				ExprValue{ ValueType::INT },
				ExprValue{ ValueType::FLOAT } };
		}

		case BinaryOPType::GREATER:
		case BinaryOPType::GREATER_EQ:
		case BinaryOPType::SMALLER:
		case BinaryOPType::SMALLER_EQ: {
			parser_type_check_num_expr(curr_scope, node);
			return { ExprValue{ ValueType::BOOL } };
		}

		case BinaryOPType::AND:
		case BinaryOPType::OR: {
			TypeContainer types = parser_type_check_expr(
				curr_scope, binary_op.left, { ExprValue{ ValueType::BOOL } });

			if (!night::contains(types, ValueType::BOOL))
				THROW_ERR_TYPE_CHECK_BINARY("left", "type 'bool'");

			types = parser_type_check_expr(curr_scope, binary_op.right,
				{ ExprValue{ ValueType::BOOL } });

			if (!night::contains(types, ValueType::BOOL))
				THROW_ERR_TYPE_CHECK_BINARY("right", "type 'bool'");

			return { ExprValue{ ValueType::BOOL } };
		}

		case BinaryOPType::EQUAL:
		case BinaryOPType::NOT_EQUAL: {
			TypeContainer const& lhs = parser_type_check_expr(
				curr_scope, binary_op.left, { ExprValue{ ValueType::BOOL } });

			TypeContainer const& rhs = parser_type_check_expr(
				curr_scope, binary_op.right, { ExprValue{ ValueType::BOOL } });

			bool const match = std::any_of(lhs.begin(), lhs.end(),
				[&](const ValueType& type) { return night::contains(lhs, type); });

			if (!match) {
				throw NIGHT_COMPILE_ERROR(
					"operator '" + binary_op.data + "' can only be used to compare two equivalent non-class types",
					"left hand value of operator '" + binary_op.data + "' currently contains " + get_types_as_str(lhs) + ", but right hand value currently contains " + get_types_as_str(rhs),
					night::learn_operators, node->loc);
			}

			return { ExprValue{ ValueType::BOOL } };
		}
		
		case BinaryOPType::DOT: {
			// find required types and return types based on method

			// a note about empty method types:
			/*
			// if a method's return types can't be deduced when the method is
			// defined, then the return type is treated as if it where any type
			//
			// returning 'all_types' here wouldn't be wrong, but to keep the
			// types to a minimum (only containing necessary types),
			// 'required_types' is returned instead
			*/
			bool return_required_types = false;

			VariableTypeContainer required_t, return_types;
			for (const auto& check_class : check_classes)
			{
				auto it = check_class.second.methods.find(node->right->data);
				if (it == check_class.second.methods.end())
					continue;

				// find required types

				if (check_class.first == "array")
					required_t.insert(VariableType::ARRAY);
				else if (check_class.first == "string")
					required_t.insert(VariableType::STRING);
				else
					required_t.insert(check_class.first);

				// find return types

				if (it->second.return_types.empty())
					return_required_types = true;
				else if (!return_required_types)
					return_types.insert(it->second.return_types);
			}

			const VariableTypeContainer object_types =
				type_check_expr(current_scope, node->left, required_t);

			if (!required_t.empty() &&
				!object_types.contains(VariableType::STRING) &&
				!object_types.contains(VariableType::ARRAY) &&
				!object_types.contains(VariableType::CLASS))
				throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used on types: 'str', 'arr', and 'obj'", "left hand value of operator '" + node->data + "' currently contains " + get_var_types_as_str(object_types), Learn::TYPE_CHECKING);

			if (required_t.empty())
				throw NIGHT_COMPILE_ERROR("method '" + node->right->data + "' does not exist within any class");

			// TO DO:
			// type check method parameters
			//
			//


			return return_required_types ? required_types : return_types;
		}
		}
	}
	}
}

std::pair<std::string const, CheckVariable>* get_variable(
	std::shared_ptr<Scope> const& scope,
	std::string const& var_name
) {
	for (Scope* current_scope = scope.get(); current_scope != nullptr;)
	{
		for (auto& variable : current_scope->variables)
		{
			if (variable.first == var_name)
				return &variable;
		}

		current_scope = current_scope->upper_scope.get();
	}

	return nullptr;
}

bool lower_precedence(
	std::string const& op1,
	std::string const& op2
) {
	const std::vector<std::unordered_set<std::string> > operators{
		{ "[]", "." },
		{ "!" },
		{ "*", "/", "%" },
		{ "+", "-" },
		{ ">", "<", ">=", "<=" },
		{ "==", "!=" },
		{ "||", "&&" }
	};

	std::size_t pre1, pre2;
	for (std::size_t a = 0; a < operators.size(); ++a)
	{
		if (operators[a].contains(op1))
			pre1 = a;
		if (operators[a].contains(op2))
			pre2 = a;
	}

	return pre1 < pre2;
}

void parser_type_check_num_expr(
	std::shared_ptr<Scope> const& curr_scope,
	std::shared_ptr<ExprNode> const& node
) {
	ExprBinaryOP const& binary_op = std::get<ExprBinaryOP>(node->data);

	TypeContainer types = parser_type_check_expr(curr_scope, binary_op.left,
		{ ExprValue{ ValueType::INT }, ExprValue{ ValueType::FLOAT } });

	if (!night::contains(types, ValueType::INT, ValueType::FLOAT))
		THROW_ERR_TYPE_CHECK_BINARY("left", "types 'int' or 'float'");

	types = parser_type_check_expr(curr_scope, binary_op.right,
		{ ExprValue{ ValueType::INT }, ExprValue{ ValueType::FLOAT } });

	if (!night::contains(types, ValueType::INT, ValueType::FLOAT))
		THROW_ERR_TYPE_CHECK_BINARY("right", "types 'int' or 'float'");
}


bool is_reachable(std::shared_ptr<Scope> const& scope)
{
	return scope->upper_scope != nullptr &&
		scope->upper_scope->stmts.back().type == StmtType::FUNC_DEF;
}

TypeContainer const Parser::all_types{
	ExprValue{ ValueType::BOOL }, ExprValue{ ValueType::INT }, ExprValue{ ValueType::FLOAT },
	ExprValue{ ValueType::STR }, ExprValue{ ValueType::ARR }
};

CheckFunctionContainer Parser::check_funcs{
	make_check_function("print", { all_types }),
	make_check_function("input", {}, { ExprValue{ ValueType::STR } }),

	make_check_function("range",
		{
			{ ExprValue{ ValueType::INT } },
			{ ExprValue{ ValueType::INT } }
		},
		{
			ExprValue{ ValueType::ARR }
		}),

	make_check_function("int",
		{
			{ ExprValue{ ValueType::INT }, ExprValue{ ValueType::FLOAT }, ExprValue{ ValueType::STR } }
		},
		{
			ExprValue{ ValueType::INT }
		}),
	make_check_function("float",
		{ { ExprValue{ ValueType::INT }, ExprValue{ ValueType::FLOAT }, ExprValue{ ValueType::STR } } },
		{ ExprValue{ ValueType::FLOAT } }),
	make_check_function("str",
		{ { ExprValue{ ValueType::BOOL }, ExprValue{ ValueType::INT}, ExprValue{ ValueType::FLOAT}, ExprValue{ ValueType::STR } } },
		{ ExprValue{ ValueType::STR } }),

	make_check_function("system",
		{ { ExprValue{ ValueType::STR } } })
};

CheckClassContainer Parser::check_classes{
	make_check_class("array",
		CheckVariableContainer(),
		CheckFunctionContainer{
			make_check_function("len", {}, { VariableType::INT }),

			make_check_function("push", { all_types }),
			make_check_function("push", { { VariableType::INT } }, all_types),

			make_check_function("pop"),
			make_check_function("pop", { { VariableType::INT } })
		}
	),
	make_check_class("string",
		CheckVariableContainer(),
		CheckFunctionContainer{
			make_check_function("len", {}, { VariableType::INT })
		}
	)
};
