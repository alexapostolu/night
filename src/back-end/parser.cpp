#include "../../include/back-end/parser.hpp"
#include "../../include/back-end/utils.hpp"
#include "../../include/back-end/token.hpp"
#include "../../include/error.hpp"

#include <memory>
#include <algorithm>
#include <iterator>
#include <string>
#include <vector>
#include <unordered_set>

/* methods */

Parser::Parser(
	const std::shared_ptr<Scope>& current_scope,
	const std::vector<Token>& tokens
)
	: loc(tokens[0].loc)
{
	if (tokens[0].type == TokenType::SET)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::VAR)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "expected variable name after 'set' keyword", "variable initializations are required to be in this format: 'set variable_name = expression'");
		if (tokens.size() == 2 || tokens[2].type != TokenType::ASSIGN)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "expected assignment operator '=' after variable name", "variable initializations are required to be in this format: 'set variable_name = expression'");
		if (tokens.size() == 3)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "expected expression after assignment operator '='", "variable initializations are required to be in this format: 'set variable_name = expression'");
		if (current_scope->variables.find(tokens[1].data) != current_scope->variables.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, loc, "variable '" + tokens[1].data + "' has already been defined", "variables can only be defined once, regardless of their scope");
		if (check_functions.find(tokens[1].data) != check_functions.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, loc, "variable '" + tokens[1].data + "' has the same name as a function", "variable and function names are required to be unique");
		if (check_functions.find(tokens[1].data) != check_functions.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, loc, "variable '" + tokens[1].data + "' has the same name as a class", "variable and class names are required to be unique");

		// parsing expression

		const std::vector<Value> values = TokensToValues(
			std::vector<Token>(tokens.begin() + 3, tokens.end()));

		const std::shared_ptr<Expression> expression = ValuesToExpression(values);

		bool is_array = false;
		const VariableTypeContainer expression_types =
			type_check_expr(current_scope, expression, {}, {}, &is_array);

		// creating check variable and statement

		current_scope->variables[tokens[1].data] =
			CheckVariable(expression_types, is_array);

		current_scope->statements.push_back(Statement{
			loc, StatementType::VARIABLE,
			Variable{ tokens[1].data, expression }
		});
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VAR && tokens[1].type == TokenType::ASSIGN)
	{
		if (tokens.size() == 2)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "expected expression after assignment operator '='", "variable assignments must be in this format: 'variable_name = expression'");

		// definition and type checking variable

		auto* const check_variable = get_variable(current_scope, tokens[0].data);
		if (check_variable == nullptr)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, loc, "variable '" + tokens[0].data + "' is undefined", "variables must be defined before they are used");

		if (!check_variable->second.is_param())
		{
			if (tokens[1].data == "+=" && !find_num_types(check_variable->second.types) &&
				check_variable->second.types.find(VariableType::STRING) == check_variable->second.types.end())
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "assignment operator '+=' can only be used on variables of type 'int' or 'str'", "variable '" + check_variable->first + "has " + get_var_types_as_str(check_variable->second.types));
			if (tokens[1].data != "=" && tokens[1].data != "+=" && !find_num_types(check_variable->second.types))
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "assignment operator '" + tokens[1].data + "' can only be used on variables of type 'int' or 'float'", "variable '" + tokens[0].data + "has " + get_var_types_as_str(check_variable->second.types));
		}

		// extracting and type checking expression

		const std::vector<Value> values = TokensToValues(
			std::vector<Token>(tokens.begin() + 2, tokens.end()));

		const std::shared_ptr<Expression> assign_expr = ValuesToExpression(values);
		const VariableTypeContainer assign_types = type_check_expr(current_scope, assign_expr);

		if (tokens[1].data == "+=" && !find_num_types(assign_types) &&
			assign_types.find(VariableType::STRING) == assign_types.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "assignment operator '+=' can only assign expressions of type 'int', 'float', or 'str'", "expression has " + get_var_types_as_str(assign_types));
		if (tokens[1].data != "=" && tokens[1].data != "+=" && !find_num_types(assign_types) &&
			assign_types.find(VariableType::STRING) == assign_types.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "assignment operator '" + tokens[1].data + "' can only assign expressions of type 'int' or 'float'", "expression has " + get_var_types_as_str(assign_types));

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

		check_variable->second.types.insert(
			assign_types.begin(), assign_types.end());

		current_scope->statements.push_back(Statement{
			loc, StatementType::ASSIGNMENT, assignment });
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VAR && tokens[1].type == TokenType::OPEN_SQUARE)
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
						index_types.find(VariableType::ARRAY) == index_types.end() &&
						index_types.find(VariableType::STRING) == index_types.end())
						throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "subscript operator can only be used on variables of type 'str' or 'arr'", "variable has " + get_var_types_as_str(index_types));

					const std::vector<Value> values =
						TokensToValues(std::vector<Token>(start, it));

					index_exprs.push_back(ValuesToExpression(values));
					index_types = type_check_expr(current_scope, index_exprs.back());

					if (!find_num_types(index_types))
						throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "array indices must be of type 'int'", "index has " + get_var_types_as_str(index_types));

					start = std::next(it, 2);
				}
			}
			else if (it->type == TokenType::ASSIGN)
			{
				if (open_square_count < 0)
					throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "missing " + std::to_string(open_square_count) + " open bracket(s)");
				if (open_square_count > 0)
					throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "missing " + std::to_string(open_square_count) + " close bracket(s)");

				break;
			}
		}

		if (it == tokens.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "expected assignment operator in element assignment", "element assignments must be in this form: 'variable[index] = expression'");

		// definition checking variable

		auto* const check_variable = get_variable(current_scope, tokens[0].data);
		if (check_variable == nullptr)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, loc, "variable '" + tokens[1].data + "' is undefined", "variables have to be defined before they are used");

		// type checking variable

		if (!check_variable->second.is_array && check_variable->second.find_type(VariableType::STRING))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "subscript operator '[]' can only be used on variables of type 'str' or 'arr'", "variable '" + check_variable->first + "' has " + get_var_types_as_str(check_variable->second.types));

		// parsing expression

		const std::vector<Value> values = TokensToValues(
			std::vector<Token>(std::next(it, 1), tokens.end()));

		const std::shared_ptr<Expression> assign_expr =
			ValuesToExpression(values);

		const VariableTypeContainer assign_types =
			type_check_expr(current_scope, assign_expr);

		check_variable->second.types.insert(
			assign_types.begin(), assign_types.end());

		if (!check_variable->second.is_array && (assign_expr->type != ValueType::STRING || assign_expr->data.length() >= 1))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "subscript operator '[]' used on a string can only assign expressions of strings length 1");

		// pushing statement

		current_scope->statements.push_back(Statement{
			loc, StatementType::ELEMENT,
			Element{ tokens[0].data, index_exprs, assign_expr }
		});
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VAR && tokens[1].type == TokenType::OPERATOR)
	{
		if (tokens[1].data != ".")
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "object methods must be in this format: 'object.method()'");
		if (current_scope->variables.find(tokens[0].data) == current_scope->variables.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, loc, "variable '" + tokens[0].data + "' is undefined", "variables must be defined before they are used");

		const std::shared_ptr<Expression> object_expr = ValuesToExpression(TokensToValues(tokens));
		const VariableTypeContainer object_types = type_check_expr(current_scope, object_expr);

		// pushing statement

		current_scope->statements.push_back(Statement{
			loc, StatementType::METHOD_CALL,
			MethodCall{ tokens[0].data, object_expr }
		});
	}

	else if (tokens[0].type == TokenType::IF)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::OPEN_BRACKET)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "expected an opening bracket after 'if' keyword", "if statements are required to be in this format: 'if ( {bool expression} ) {statement}'");
		if (tokens.size() == 2)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "expected a bool expression after opening bracket", "if statements are required to be in this format: 'if ( {bool expression} ) {statement}'");

		// extracting and type checking condition

		auto close_bracket_it = std::next(tokens.begin(), 1);
		advance_to_close_bracket(tokens, close_bracket_it);

		if (close_bracket_it == tokens.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "missing closing bracket for if statement", "if statements are required to be in this format: 'Yankerdoodle8839 if ( {bool expression} ) {statement}'");

		const std::vector<Value> values = TokensToValues(
			std::vector<Token>(tokens.begin() + 2, close_bracket_it));

		const std::shared_ptr<Expression> condition_expr = ValuesToExpression(values);
		const VariableTypeContainer condition_types = type_check_expr(current_scope, condition_expr);

		if (condition_types.find(VariableType::BOOL) == condition_types.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "if statement condition is required to be of type 'bool'", "the condition currently has type(s): " + get_var_types_as_str(condition_types));

		// extracting body
		
		const std::vector<std::vector<Token> > split_tokens = std::next(close_bracket_it, 1)->type == TokenType::OPEN_CURLY
			? SplitCode(std::vector<Token>(std::next(close_bracket_it, 2), tokens.end() - 1))
			: SplitCode(std::vector<Token>(std::next(close_bracket_it, 1), tokens.end()));

		std::shared_ptr<Scope> body = std::make_shared<Scope>(current_scope);
		for (const std::vector<Token>& split_token : split_tokens)
			Parser(body, split_token);

		// pushing statement

		current_scope->statements.push_back(Statement{
			loc, StatementType::IF_STATEMENT,
			IfStatement{ { Conditional{ condition_expr, body } } }
		});
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::ELSE && tokens[1].type == TokenType::IF)
	{
		IfStatement* const if_stmt = &std::get<IfStatement>(current_scope->statements.back().stmt);

		if (tokens.size() == 2 || tokens[2].type != TokenType::OPEN_BRACKET)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "expected an opening bracket after 'if' keyword", "else if statements are required to be in this format: 'else if ( {bool expression} ) {statement}'");
		if (current_scope->statements.empty() || current_scope->statements.back().type != StatementType::IF_STATEMENT)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_grammar, loc, "else if statement does not precede an if or else if statement", "else if statements must come after an if or an else if statement");
		if (if_stmt->chains.back().condition == nullptr) // else statement
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_grammar, loc, "else if statement can not precede an else statement", "else if statements are required to come after an if or else if statement");

		// extracting and type checking condition

		auto close_bracket_it = std::next(tokens.begin(), 2);
		advance_to_close_bracket(tokens, close_bracket_it);

		if (close_bracket_it == tokens.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "missing closing bracket for else if statement", "else if statements are required to be in this format: 'else if ( {bool expression} ) {statement}'");

		const std::vector<Value> values = TokensToValues(
			std::vector<Token>(tokens.begin() + 3, close_bracket_it));

		const std::shared_ptr<Expression> condition_expr =
			ValuesToExpression(values);

		const VariableTypeContainer condition_types =
			type_check_expr(current_scope, condition_expr);

		if (condition_types.find(VariableType::BOOL) == condition_types.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "else if statement condition is required to be of type 'bool'", "the condition currently has type(s): " + get_var_types_as_str(condition_types));

		// extracting body

		const std::vector<std::vector<Token> > split_tokens = std::next(close_bracket_it, 1)->type == TokenType::OPEN_CURLY
			? SplitCode(std::vector<Token>(std::next(close_bracket_it, 2), tokens.end() - 1))
			: SplitCode(std::vector<Token>(std::next(close_bracket_it, 1), tokens.end()));

		std::shared_ptr<Scope> body = std::make_shared<Scope>(current_scope);
		for (const std::vector<Token>& split_token : split_tokens)
			Parser(body, split_token);

		// pushing statement

		if_stmt->chains.push_back(Conditional{ condition_expr, body });
	}
	else if (tokens[0].type == TokenType::ELSE)
	{
		IfStatement* const if_stmt = &std::get<IfStatement>(current_scope->statements.back().stmt);

		if (current_scope->statements.empty() || current_scope->statements.back().type != StatementType::IF_STATEMENT)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_grammar, loc, "else statement does not precede an if or else if statement", "else statements must come after an if or an else if statement");
		if (if_stmt->chains.back().condition == nullptr) // else statement
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_grammar, loc, "else statement can not precede another else statement", "else statements are required to come after an if or else if statement");

		// parsing body

		const std::vector<std::vector<Token> > split_tokens = tokens[1].type == TokenType::OPEN_CURLY
			? SplitCode(std::vector<Token>(tokens.begin() + 2, tokens.end() - 1))
			: SplitCode(std::vector<Token>(tokens.begin() + 1, tokens.end()));

		std::shared_ptr<Scope> body = std::make_shared<Scope>(current_scope);
		for (const std::vector<Token>& split_token : split_tokens)
			Parser parse(body, split_token);

		// pushing statement

		if_stmt->chains.push_back(Conditional{ nullptr, body });
	}
	
	else if (tokens[0].type == TokenType::DEF)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::VAR)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "expected function name after 'def' keyword", "function definitions are required to be in this format: 'def {function name} ( {parameters} ) {}'");
		if (tokens.size() == 2 || tokens[2].type != TokenType::OPEN_BRACKET)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "expected opening bracket after function name", "function definitions are required to be in this format: 'def {function name} ( {parameters} ) {}'");
		if (current_scope->variables.find(tokens[1].data) != current_scope->variables.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, loc, "function '" + tokens[1].data + "' can not have the same name as a variable", "function and variable names must be unique");
		if (check_functions.find(tokens[1].data) != check_functions.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, loc, "function '" + tokens[1].data + "' has already been defined", "functions can only be defined once");
		if (check_classes.find(tokens[1].data) != check_classes.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, loc, "function '" + tokens[1].data + "' can not have the same name as a class", "function and class names must be unique");
		if (current_scope->upper_scope != nullptr)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, loc, "function '" + tokens[1].data + "' can not be defined inside of a local scope", "functions must be defined in the global scope");

		// parse function parameters
		// also turns parameters into variables for definition checking the function body

		std::shared_ptr<Scope> function_scope = std::make_shared<Scope>(current_scope);

		std::vector<std::string> parameter_names;

		auto close_bracket_it = std::next(tokens.begin(), 3);
		for (; close_bracket_it != tokens.end(); ++close_bracket_it)
		{
			if (close_bracket_it->type == TokenType::CLOSE_BRACKET)
				break;

			if (close_bracket_it->type != TokenType::VAR)
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_grammar, loc, "expected variable names as function parameters", "function parameters must be variables separated by spaces");

			if (check_functions.find(close_bracket_it->data) != check_functions.end())
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, loc, "function parameter can not have the same name as a function", "function parameter names must be unique");
			if (check_classes.find(close_bracket_it->data) != check_classes.end())
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, loc, "function parameter can not have the same name as a class", "function parameter names must be unique");

			parameter_names.push_back(close_bracket_it->data);
			function_scope->variables[close_bracket_it->data] = CheckVariable();

			//if (std::next(close_bracket_it, 1) == tokens.end())
				//throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "expected closing bracket after parameter");

			if (std::next(close_bracket_it, 1)->type == TokenType::COMMA)
				++close_bracket_it;
		}

		if (close_bracket_it == tokens.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "expected closing bracket after function parameter");
		if (std::next(close_bracket_it, 1) == tokens.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "expected function body");

		// define function before parsing body to ensure function is defined for recursion
		// and return types

		check_functions[tokens[1].data] = CheckFunction();
		auto check_function = check_functions.find(tokens[1].data);

		check_function->second.parameters.resize(parameter_names.size());

		current_scope->statements.push_back(Statement{
			loc, StatementType::FUNCTION_DEF,
			FunctionDef{ tokens[1].data, parameter_names, function_scope }
		});

		FunctionDef* const function_def =
			&std::get<FunctionDef>(current_scope->statements.back().stmt);

		// extracting function body

		const std::vector<std::vector<Token> > split_tokens = std::next(close_bracket_it, 1)->type == TokenType::OPEN_CURLY
			? SplitCode(std::vector<Token>(std::next(close_bracket_it, 2), tokens.end() - 1))
			: SplitCode(std::vector<Token>(std::next(close_bracket_it, 1), tokens.end()));

		for (const std::vector<Token>& split_token : split_tokens)
			Parser(function_scope, split_token);

		// set check function to be void or not, and
		// assign check function parameters' types

		check_function->second.is_void = check_function->second.return_types.empty();

		for (std::size_t a = 0; a < function_def->parameters.size(); ++a)
		{
			check_function->second.parameters[a] = function_scope->variables[function_def->parameters[a]].types.empty()
				? all_types
				: function_scope->variables[function_def->parameters[a]].types;
		}
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VAR && tokens[1].type == TokenType::OPEN_BRACKET)
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
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "missing opening bracket in function call", std::to_string(open_bracket_count) + " closing bracket(s) do(es) not have an associated opening bracket");
		if (open_bracket_count > 0)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "missing closing bracket in function call", std::to_string(open_bracket_count) + " opening bracket(s) do(es) not have an associated closing bracket");
	
		if (it == tokens.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "unexpected token(s) after statement function call", "each statement is required to be on its own line");

		// fetching check function

		auto check_function = check_functions.find(tokens[0].data);
		if (check_function == check_functions.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, loc, "function '" + tokens[0].data + "' is undefined", "functions are required to be defined before they are called");

		// definition and type checking arguments with parameters

		if (argument_types.size() != check_function->second.parameters.size())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, loc, "function '" + tokens[0].data + "' does not have the correct number of arguments", "the function was defined with '" + std::to_string(check_function->second.parameters.size()) + "' parameter(s), but is called with '" + std::to_string(argument_types.size()) + "' argument(s)");

		for (std::size_t a = 0; a < argument_types.size(); ++a)
		{
			bool match = false;
			for (auto param_type : check_function->second.parameters[a])
			{
				if (argument_types[a].find(param_type) != argument_types[a].end())
				{
					match = true;
					break;
				}
			}

			if (!match)
			{
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "argument number '" + std::to_string(a + 1) + "' for function call '" + tokens[0].data + "' can only be of " + get_var_types_as_str(check_function->second.parameters[a]) + "'", "argument 1 currently has " + get_var_types_as_str(argument_types[a]));
			}
		}

		// pushing statement

		current_scope->statements.push_back(Statement{
			loc, StatementType::FUNCTION_CALL,
			FunctionCall{ tokens[0].data, argument_expressions }
		});
	}
	else if (tokens[0].type == TokenType::RETURN)
	{
		// a note on the function scope:
		//
		// make sure the check function is pushed to the check functions array
		// before making the recursive call to extract the body
		//
		// if that doesn't happen then this won't work, since here it climbs up
		// scopes in order to find the function scope

		const Scope* function_scope = current_scope->upper_scope.get();
		for (; function_scope->upper_scope != nullptr; function_scope = function_scope->upper_scope.get());

		if (function_scope->statements.back().type != StatementType::FUNCTION_DEF)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_grammar, loc, "return statement can not be outside of a function", "return statements are required to be inside of functions");

		// extracting return expression

		std::shared_ptr<Expression> return_expr(nullptr);
		if (tokens.size() > 1)
		{
			const std::vector<Value> values = TokensToValues(
				std::vector<Token>(tokens.begin() + 1, tokens.end()));

			return_expr = ValuesToExpression(values);
			const VariableTypeContainer return_expr_types =
				type_check_expr(current_scope, return_expr);

			// adding types to return types and pushing statement

			const FunctionDef* const function_def =
				&std::get<FunctionDef>(function_scope->statements.back().stmt);

			check_functions[function_def->name].return_types.insert(
				return_expr_types.begin(), return_expr_types.end());
		}
		
		current_scope->statements.push_back(Statement{
			loc, StatementType::RETURN, Return{ return_expr } });
	}
	
	else if (tokens[0].type == TokenType::WHILE)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::OPEN_BRACKET)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "expected an opening bracket after 'while' keyword", "while loop statements are required to be in this format: 'while ( {bool expression} ) {statement}'");

		// extracting and type checking condition

		auto close_bracket_it = std::next(tokens.begin(), 1);
		advance_to_close_bracket(tokens, close_bracket_it);

		if (close_bracket_it == tokens.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "missing closing bracket for while loop condition", "while loop statements are required to be in this format: 'while ( {bool expression} ) {statement}'");

		const std::vector<Value> values = TokensToValues(
			std::vector<Token>(tokens.begin() + 2, close_bracket_it));

		const std::shared_ptr<Expression> condition_expr = ValuesToExpression(values);
		const VariableTypeContainer condition_types = type_check_expr(current_scope, condition_expr);

		if (condition_types.find(VariableType::BOOL) == condition_types.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "while loop condition must contain type 'bool'", "condition currently contains " + get_var_types_as_str(condition_types));

		// extracting body

		const std::vector<std::vector<Token> > split_tokens = std::next(close_bracket_it, 1)->type == TokenType::OPEN_CURLY
			? SplitCode(std::vector<Token>(std::next(close_bracket_it, 2), tokens.end() - 1))
			: SplitCode(std::vector<Token>(std::next(close_bracket_it, 1), tokens.end()));

		std::shared_ptr<Scope> scope_body = std::make_shared<Scope>(current_scope);
		for (const std::vector<Token>& token_split : split_tokens)
			Parser(scope_body, token_split);

		// pushing statement

		current_scope->statements.push_back(Statement{
			loc, StatementType::WHILE_LOOP,
			WhileLoop{ condition_expr, scope_body }
		});
	}
	else if (tokens[0].type == TokenType::FOR)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::OPEN_BRACKET)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "expected opening bracket after 'for' keyword", "for loops must be in this format: 'for (iterator : range) {}'");
		if (tokens.size() == 2 || tokens[2].type != TokenType::VAR)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "expected iterator after opening bracket", "for loops must be in this format: 'for (iterator : range) {}'");
		if (tokens.size() == 3 || tokens[3].type != TokenType::COLON)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "expected colon after iterator", "for loops must be in this format: 'for (iterator : range) {}'");
		if (tokens.size() == 4)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "expected range after colon", "for loops must be in this format: 'for (iterator : range) {}'");

		// evaluating iterator and range

		auto close_bracket_it = std::next(tokens.begin(), 1);
		advance_to_close_bracket(tokens, close_bracket_it);

		if (close_bracket_it == tokens.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "missing closing bracket in for loop", "for loops must be in this format: 'for (condition) {}'");

		const std::vector<Value> range_values = TokensToValues(std::vector<Token>(tokens.begin() + 4, close_bracket_it));
		const std::shared_ptr<Expression> range_expr = ValuesToExpression(range_values);

		// also makes sure range is type array or string
		bool is_array = false;
		const VariableTypeContainer range_types = type_check_expr(current_scope, range_expr, "", {}, &is_array);

		if (!is_array && range_types.find(VariableType::STRING) == range_types.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "for loop range is required to be of type 'array'", "range currently has type(s): " + get_var_types_as_str(range_types));

		// extracting scope and constructing statement

		const std::vector<std::vector<Token> > split_tokens = std::next(close_bracket_it, 1)->type == TokenType::OPEN_CURLY
			? SplitCode(std::vector<Token>(std::next(close_bracket_it, 2), tokens.end() - 1))
			: SplitCode(std::vector<Token>(std::next(close_bracket_it, 1), tokens.end()));

		std::shared_ptr<Scope> for_scope = std::make_shared<Scope>(current_scope);

		for_scope->variables[tokens[2].data] = CheckVariable(range_types, false);

		for (const std::vector<Token>& split_token : split_tokens)
			Parser(for_scope, split_token);

		current_scope->statements.push_back(Statement{
			loc, StatementType::FOR_LOOP,
			ForLoop{ tokens[2].data, range_expr, for_scope }
		});
	}
	
	else
	{
		throw CompileError(__FILE__, __LINE__, CompileError::invalid_grammar, loc, "sorry but i have absolutely no idea what you're trying to do here :/", "please go and read the docs :)");
	}
}

std::pair<const std::string, CheckVariable>* Parser::get_variable(
	const std::shared_ptr<Scope>& scope,
	const std::string& var_name
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

std::vector<Value> Parser::TokensToValues(const std::vector<Token>& tokens)
{
	assert(!tokens.empty() && "tokens shouldn't be empty");

	// extract expression

	std::vector<Value> values;
	for (std::size_t a = 0; a < tokens.size(); ++a)
	{
		if ((tokens[a].type == TokenType::VAR || tokens[a].type == TokenType::STRING) &&
			a < tokens.size() - 1 && tokens[a + 1].type == TokenType::OPEN_SQUARE)
		{
			if (tokens[a].type == TokenType::VAR)
				values.push_back(Value{ ValueType::VARIABLE, tokens[a].data });
			else
				values.push_back(Value{ ValueType::STRING, tokens[a].data });

			while (a < tokens.size() - 1 && tokens[a + 1].type == TokenType::OPEN_SQUARE)
			{
				if (a == tokens.size() - 2)
					throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "closing square bracket missing for subscript operator");

				a++;

				auto start = std::next(tokens.begin(), a + 1);
				
				for (int open_bracket_count = 0; a < tokens.size(); ++a)
				{
					if (tokens[a].type == TokenType::OPEN_SQUARE)
					{
						open_bracket_count++;
					}
					else if (tokens[a].type == TokenType::CLOSE_SQUARE)
					{
						open_bracket_count--;
						if (open_bracket_count == 0)
							break;
					}
				}

				values.push_back(Value{
					ValueType::OPERATOR, "[]",
					{ TokensToValues(std::vector<Token>(start, tokens.begin() + a)) }
				});
			}
		}
		else if (a < tokens.size() - 1 && tokens[a].type == TokenType::VAR && tokens[a + 1].type == TokenType::OPEN_BRACKET)
		{
			Value value_call{ ValueType::CALL, tokens[a].data };

			a += 2;

			if (a >= tokens.size())
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "missing closing bracket for function call '" + tokens[a - 2].data + "'");

			if (tokens[a].type == TokenType::CLOSE_BRACKET)
			{
				values.push_back(value_call);
				continue;
			}

			int open_bracket_count = 0, open_square_count = 0;
			for (auto start = std::next(tokens.begin(), a); a < tokens.size(); ++a)
			{
				if (tokens[a].type == TokenType::OPEN_BRACKET)
					open_bracket_count++;
				else if (tokens[a].type == TokenType::CLOSE_BRACKET)
					open_bracket_count--;
				else if (tokens[a].type == TokenType::OPEN_SQUARE)
					open_square_count++;
				else if (tokens[a].type == TokenType::CLOSE_SQUARE)
					open_square_count--;

				if (open_square_count == 0 && ((tokens[a].type == TokenType::COMMA && open_bracket_count == 0) ||
					(tokens[a].type == TokenType::CLOSE_BRACKET && open_bracket_count == -1)))
				{
					value_call.extras.push_back(TokensToValues(
						std::vector<Token>(start, tokens.begin() + a)));

					start = std::next(tokens.begin(), a + 1);

					if (tokens[a].type == TokenType::CLOSE_BRACKET)
						break;
				}
			}

			if (a == tokens.size())
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "expecting closing bracket for function call '", "function call '" + tokens[a].data + "' does not have a closing bracket");

			values.push_back(value_call);
		}
		else if (tokens[a].type == TokenType::OPEN_SQUARE)
		{
			Value value_array{ ValueType::ARRAY };

			a++;

			if (tokens[a].type == TokenType::CLOSE_SQUARE)
			{
				values.push_back(value_array);
				continue;
			}

			// parse array elements
			int open_bracket_count = 0, open_square_count = 0;
			for (auto start = std::next(tokens.begin(), a); a < tokens.size(); ++a)
			{
				if (tokens[a].type == TokenType::OPEN_BRACKET)
					open_bracket_count++;
				else if (tokens[a].type == TokenType::CLOSE_BRACKET)
					open_bracket_count--;
				else if (tokens[a].type == TokenType::OPEN_SQUARE)
					open_square_count++;
				else if (tokens[a].type == TokenType::CLOSE_SQUARE)
					open_square_count--;

				// end of element reached; add it to array
				if (open_bracket_count == 0 && ((tokens[a].type == TokenType::COMMA && open_square_count == 0) ||
					(tokens[a].type == TokenType::CLOSE_SQUARE && open_square_count == -1)))
				{
					const std::vector<Value> temp_values = TokensToValues(
						std::vector<Token>(start, tokens.begin() + a));

					value_array.extras.push_back(temp_values);

					start = std::next(tokens.begin(), a + 1);

					if (tokens[a].type == TokenType::CLOSE_SQUARE)
						break;

					continue;
				}
			}

			if (a >= tokens.size())
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "missing closing square bracket for array", "opening square bracket does not have an associated closing square bracket");

			values.push_back(value_array);
		}
		else
		{
			switch (tokens[a].type)
			{
			case TokenType::BOOL:
				values.push_back(Value{ ValueType::BOOL, tokens[a].data });
				break;
			case TokenType::INT:
				values.push_back(Value{ ValueType::INT, tokens[a].data });
				break;
			case TokenType::FLOAT:
				values.push_back(Value{ ValueType::FLOAT, tokens[a].data });
				break;
			case TokenType::STRING:
				values.push_back(Value{ ValueType::STRING, tokens[a].data });
				break;
			case TokenType::VAR:
				values.push_back(Value{ ValueType::VARIABLE, tokens[a].data });
				break;
			case TokenType::OPEN_BRACKET:
				values.push_back(Value{ ValueType::OPEN_BRACKET, tokens[a].data });
				break;
			case TokenType::CLOSE_BRACKET:
				values.push_back(Value{ ValueType::CLOSE_BRACKET, tokens[a].data });
				break;
			case TokenType::OPERATOR:
				values.push_back(Value{ ValueType::OPERATOR, tokens[a].data });
				break;
			default:
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_grammar, loc, "unexpected token '" + tokens[a].data + "' in expression", "expressions can only contain values, variables, arrays, functions, and operators");
			}
		}
	}

	// check expression

	for (std::size_t a = 0; a < values.size() - 1; ++a)
	{
		if (values[a].type != ValueType::OPEN_BRACKET && values[a].type != ValueType::CLOSE_BRACKET &&
			values[a].type != ValueType::OPERATOR && values[a + 1].type != ValueType::OPEN_BRACKET &&
			values[a + 1].type != ValueType::CLOSE_BRACKET && values[a + 1].type != ValueType::OPERATOR)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "expected operator in between values", "expression has to contains binary operators between two values");
	}

	return values;
}

std::shared_ptr<Expression> Parser::new_expression(
	const Value& value,
	const std::shared_ptr<Expression>& left,
	const std::shared_ptr<Expression>& right
) {
	std::shared_ptr<Expression> expression = std::make_shared<Expression>(Expression{
		loc,
		value.type, value.data,
		{},
		left, right
	});

	expression->extras.reserve(value.extras.size());
	for (const std::vector<Value>& values : value.extras)
		expression->extras.push_back(ValuesToExpression(values));

	return expression;
}

std::shared_ptr<Expression> Parser::GetNextGroup(
	const std::vector<Value>& values, std::size_t& index
) {
	const bool isFrontUnary = values[index].type == ValueType::OPERATOR &&
		(values[index].data == "!" || values[index].data == "-");
	if (isFrontUnary)
		index++;

	if (values[index].type == ValueType::OPERATOR && (values[index].data == "!" || values[index].data == "-"))
		throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, loc, "unary operators cannot be adjacent to one another", "unary operators can only be next to a value");

	// increments index to next operator
	std::size_t start = index;
	for (int open_bracket_count = 0; index < values.size(); ++index)
	{
		if (values[index].type == ValueType::OPEN_BRACKET)
			open_bracket_count++;
		else if (values[index].type == ValueType::CLOSE_BRACKET)
			open_bracket_count--;

		if (open_bracket_count == 0 && values[index].type == ValueType::OPERATOR)
			break;

		if (open_bracket_count == 0 && values[index].type == ValueType::CLOSE_BRACKET)
		{
			index++;
			break;
		}
	}

	// evaluate brackets
	std::shared_ptr<Expression> group_expression = values[start].type == ValueType::OPEN_BRACKET
		? ValuesToExpression(std::vector<Value>(values.begin() + start + 1, values.begin() + index))
		: new_expression(values[start], nullptr, nullptr);

	// evaluate subscript operator
	while (index < values.size() && values[index].type == ValueType::OPERATOR && values[index].data == "[]")
		group_expression = new_expression(values[index++], nullptr, group_expression);

	// evaluate unary operator
	return isFrontUnary
		? new_expression(values[start - 1], nullptr, group_expression)
		: group_expression;
}

int Parser::GetOperatorPrecedence(
	const ValueType& type, const std::string& value
) {
	assert(type == ValueType::OPERATOR);

	const std::vector<std::unordered_set<std::string> > operators{
		{ "[]", "." },
		{ "!" },
		{ "*", "/", "%" },
		{ "+", "-" },
		{ ">", "<", ">=", "<=" },
		{ "==", "!=" },
		{ "||", "&&" }
	};

	for (std::size_t a = 0; a < operators.size(); ++a)
	{
		if (operators[a].find(value) != operators[a].end())
			return (int)a;
	}

	assert(false && "operator missing");
	return {};
}

std::shared_ptr<Expression> Parser::ValuesToExpression(
	const std::vector<Value>& values
) {
	assert(!values.empty() && "values shouldn't be empty");

	std::size_t a = 0;

	std::shared_ptr<Expression> root = GetNextGroup(values, a);
	const Expression* protect = root.get();

	// parse expression

	while (a < values.size() - 1)
	{
		assert(values[a].type == ValueType::OPERATOR);

		// travel tree to find a nice spot to settle down

		std::shared_ptr<Expression> curr = root;
		Expression* prev = curr.get();
		while ((curr->left != nullptr || curr->right != nullptr) && curr.get() != protect &&
			GetOperatorPrecedence(curr->type, curr->data) > GetOperatorPrecedence(values[a].type, values[a].data))
		{
			prev = curr.get();
			curr = curr->right;
		}

		// create nodes

		const std::size_t op_index = a++;

		const std::shared_ptr<Expression> next_value = GetNextGroup(values, a);
		const std::shared_ptr<Expression> op_node = new_expression(values[op_index], curr, next_value);

		protect = next_value.get();

		if (curr == root)
			root = op_node;
		else
			prev->right = op_node;
	}

	return root;
}

VariableTypeContainer Parser::type_check_expr(
	const std::shared_ptr<Scope>& current_scope, 
	const std::shared_ptr<Expression>& node,
	const std::string& op_name,
	const VariableTypeContainer& required_types,
	bool* get_element_types
) {
	assert(node != nullptr);

	// if node->left and node->right is NULL, then node must be a value
	if (node->left == nullptr && node->right == nullptr)
	{
		switch (node->type) {
		case ValueType::ARRAY: {
			if (get_element_types == nullptr)
				return { VariableType::ARRAY };

			*get_element_types = true;

			VariableTypeContainer element_types;
			for (const std::shared_ptr<Expression>& element : node->extras)
			{
				const VariableTypeContainer element_type = type_check_expr(current_scope, element);
				element_types.insert(element_type.begin(), element_type.end());
			}

			return element_types;
		}
		case ValueType::VARIABLE: {
			auto* const check_variable = get_variable(current_scope, node->data);
			if (check_variable == nullptr)
			{
				if (check_classes.find(node->data) == check_classes.end())
					throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, loc, "variable '" + node->data + "' is undefined", "variables have to be defined before they are used");
				
				return { { VariableType::CLASS, node->data } };
			}

			// variable has to be an array
			if (get_element_types != nullptr)
			{
				if (check_variable->second.is_param())
				{
					check_variable->second.types = all_types;
					check_variable->second.is_array = true;

					*get_element_types = true;
				}
				else if (check_variable->second.is_array)
				{
					*get_element_types = true;
				}

				return check_variable->second.types;
			}

			if (check_variable->second.is_param())
			{
				check_variable->second.types = required_types;
				if (required_types.size() == 1 && required_types.find(VariableType::ARRAY) != required_types.end())
				{
					check_variable->second.is_array = true;
					return { VariableType::ARRAY };
				}

				return check_variable->second.types;
			}

			if (check_variable->second.is_array)
				return { VariableType::ARRAY };

			return check_variable->second.types;
		}
		case ValueType::CALL: {
			auto check_function = check_functions.find(node->data);

			if (check_function == check_functions.end())
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, loc, "function '" + node->data + "' is undefined", "functions have to be defined before they are called");
			if (check_function->second.is_void)
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "function '" + check_function->first + "' does not have a return value", "functions have to have a return value to be used in expression");
			if (node->extras.size() != check_function->second.parameters.size())
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, loc, "function '" + check_function->first + "' was called with '" + std::to_string(node->extras.size()) + "' arguments, but was defined with '" + std::to_string(check_function->second.parameters.size()) + "' parameters");

			// type checking function parameters
			for (std::size_t a = 0; a < check_function->second.parameters.size(); ++a)
			{
				const VariableTypeContainer argument_types =
					type_check_expr(current_scope, node->extras[a], "", {});

				if (check_function->second.parameters[a].empty())
				{
					check_function->second.parameters[a] = argument_types;
					continue;
				}

				for (const VariableType& argument_type : argument_types)
				{
					if (check_function->second.parameters[a].find(argument_type) ==
						check_function->second.parameters[a].end())
						throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "argument number '" + std::to_string(a + 1) + "' for function '" + check_function->first + "' must have " + get_var_types_as_str(check_function->second.parameters[a]) + ", but argument number '" + std::to_string(a + 1) + "' has " + get_var_types_as_str(argument_types));
				}
			}

			// type checking return values
			if (check_function->second.return_types.empty())
			{
				check_function->second.return_types = required_types;
			}
			else if (!required_types.empty())
			{
				for (const VariableType& required_type : required_types)
				{
					if (check_function->second.return_types.find(required_type) !=
						check_function->second.return_types.end())
						return check_function->second.return_types;
				}

				throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "function '" + check_function->first + "' can not be used with operator '" + op_name + "'", "operator requires type(s): " + get_var_types_as_str(required_types) + ", but function has return type(s): " + get_var_types_as_str(check_function->second.return_types));
			}

			return check_function->second.return_types;
		}
		case ValueType::BOOL: {
			return { VariableType::BOOL };
		}
		case ValueType::INT: {
			return { VariableType::INT };
		}
		case ValueType::FLOAT: {
			return { VariableType::FLOAT };
		}
		case ValueType::STRING: {
			return { VariableType::STRING };
		}
		default: {
			assert(false);
		}
		}
	}

	assert(node->type == ValueType::OPERATOR);

	if (node->data == "-")
	{
		// unary operator (negative)
		if (node->left == nullptr)
		{
			const VariableTypeContainer right = type_check_expr(current_scope, node->right, node->data, { VariableType::INT, VariableType::FLOAT });

			if (!find_num_types(right))
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "operator '-' can only be used on types 'int' or 'float'", "right hand value of operator '-' has " + get_var_types_as_str(right));

			return { VariableType::INT, VariableType::FLOAT };
		}

		// binary operator (minus)

		const VariableTypeContainer left  = type_check_expr(current_scope, node->left,  node->data, { VariableType::INT, VariableType::FLOAT });
		const VariableTypeContainer right = type_check_expr(current_scope, node->right, node->data, { VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(left))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "operator '-' can only be used on types 'int' or 'float'", "left hand value of operator '-' has " + get_var_types_as_str(left));
		if (!find_num_types(right))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "operator '-' can only be used on types 'int' or 'float'", "right hand value of operator '-' has " + get_var_types_as_str(right));
		
		return { VariableType::INT, VariableType::FLOAT };
	}
	if (node->data == "!")
	{
		const VariableTypeContainer right = type_check_expr(current_scope, node->right, node->data, { VariableType::BOOL });

		if (!find_num_types(right))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "operator '!' can only be used on type 'bool'", "right hand value of operator '!' has " + get_var_types_as_str(right));

		return { VariableType::BOOL };
	}
	if (node->data == "[]")
	{
		bool get_element_types = false;
		const VariableTypeContainer array_types =
			type_check_expr(current_scope, node->right, node->data, { VariableType::ARRAY }, &get_element_types);
		if (!get_element_types && array_types.find(VariableType::STRING) == array_types.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "operator '[]' can only be used on types 'str' or 'arr'", "expression currently has " + get_var_types_as_str(array_types));

		const VariableTypeContainer index_types = type_check_expr(current_scope, node->extras[0], node->data, { VariableType::INT });
		if (index_types.find(VariableType::INT) == index_types.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "index for subscript operator can only have type 'int'", "index has " + get_var_types_as_str(array_types));
 
		return array_types;
	}

	assert(node->left != nullptr && node->right != nullptr);

	if (node->data == "+")
	{
		const VariableTypeContainer left =
			type_check_expr(current_scope, node->left, node->data, { VariableType::INT, VariableType::FLOAT, VariableType::STRING });
		const VariableTypeContainer right =
			type_check_expr(current_scope, node->right, node->data, { VariableType::INT, VariableType::FLOAT, VariableType::STRING });

		VariableTypeContainer add_types;
		if (left.find(VariableType::INT) != left.end() && right.find(VariableType::INT) != right.end())
			add_types.insert(VariableType::INT);
		if ((left.find(VariableType::FLOAT) != left.end() && find_num_types(right)) ||
			(right.find(VariableType::FLOAT) != right.end() && find_num_types(right)))
			add_types.insert(VariableType::FLOAT);
		if (left.find(VariableType::STRING) != left.end() || right.find(VariableType::STRING) != right.end())
			add_types.insert(VariableType::STRING);

		if (add_types.empty())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "operator '+' can only be used on types 'int' and 'float', or 'str'");

		return add_types;
	}
	if (node->data == "*")
	{
		const VariableTypeContainer left = type_check_expr(
			current_scope, node->left, node->data, { VariableType::INT, VariableType::FLOAT });
		const VariableTypeContainer right = type_check_expr(
			current_scope, node->right, node->data, { VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(left))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "operator '*' can only be used on types 'int' or 'float'", "left hand value of operator '*' has " + get_var_types_as_str(left));
		if (!find_num_types(right))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "operator '*' can only be used on types 'int' or 'float'", "right hand value of operator '*' has " + get_var_types_as_str(right));
		
		return { VariableType::INT, VariableType::FLOAT };
	}
	if (node->data == "/")
	{
		const VariableTypeContainer left  = type_check_expr(current_scope, node->left,  node->data, { VariableType::INT, VariableType::FLOAT });
		const VariableTypeContainer right = type_check_expr(current_scope, node->right, node->data, { VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(left))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "operator '/' can only be used on types 'int' or 'float'", "left hand value of operator '/' has " + get_var_types_as_str(left));
		if (!find_num_types(right))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "operator '/' can only be used on types 'int' or 'float'", "right hand value of operator '/' has " + get_var_types_as_str(right));
		
		return { VariableType::INT, VariableType::FLOAT };
	}
	if (node->data == "%")
	{
		const VariableTypeContainer left  = type_check_expr(current_scope, node->left,  node->data, { VariableType::INT });
		const VariableTypeContainer right = type_check_expr(current_scope, node->right, node->data, { VariableType::INT });

		if (!find_num_types(left))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "operator '%' can only be used on types 'int' or 'float'", "left hand value of operator '%' has " + get_var_types_as_str(left));
		if (!find_num_types(right))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "operator '%' can only be used on types 'int' or 'float'", "right hand value of operator '%' has " + get_var_types_as_str(right));

		return { VariableType::INT, VariableType::FLOAT };
	}
	if (node->data == ">")
	{
		const VariableTypeContainer left  = type_check_expr(current_scope, node->left, node->data, { VariableType::INT, VariableType::FLOAT });
		const VariableTypeContainer right = type_check_expr(current_scope, node->right, node->data, { VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(left))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "operator '>' can only be used on types 'int' or 'float'", "left hand value of operator '>' has " + get_var_types_as_str(left));
		if (!find_num_types(right))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "operator '>' can only be used on types 'int' or 'float'", "right hand value of operator '>' has " + get_var_types_as_str(right));

		return { VariableType::BOOL };
	}
	if (node->data == "<")
	{
		const VariableTypeContainer left  = type_check_expr(current_scope, node->left,  node->data, { VariableType::INT, VariableType::FLOAT });
		const VariableTypeContainer right = type_check_expr(current_scope, node->right, node->data, { VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(left))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "operator '<' can only be used on types 'int' or 'float'", "left hand value of operator '<' has " + get_var_types_as_str(left));
		if (!find_num_types(right))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "operator '<' can only be used on types 'int' or 'float'", "right hand value of operator '<' has " + get_var_types_as_str(right));

		return { VariableType::BOOL };
	}
	if (node->data == ">=")
	{
		const VariableTypeContainer left  = type_check_expr(current_scope, node->left,  node->data, { VariableType::INT, VariableType::FLOAT });
		const VariableTypeContainer right = type_check_expr(current_scope, node->right, node->data, { VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(left))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "operator '>=' can only be used on types 'int' or 'float'", "left hand value of operator '>=' has " + get_var_types_as_str(left));
		if (!find_num_types(right))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "operator '>=' can only be used on types 'int' or 'float'", "right hand value of operator '>=' has " + get_var_types_as_str(right));

		return { VariableType::BOOL };
	}
	if (node->data == "<=")
	{
		const VariableTypeContainer left  = type_check_expr(current_scope, node->left,  node->data, { VariableType::INT, VariableType::FLOAT });
		const VariableTypeContainer right = type_check_expr(current_scope, node->right, node->data, { VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(left))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "operator '<=' can only be used on types 'int' or 'float'", "left hand value of operator '<=' has " + get_var_types_as_str(left));
		if (!find_num_types(right))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "operator '<=' can only be used on types 'int' or 'float'", "right hand value of operator '<=' has " + get_var_types_as_str(right));

		return { VariableType::BOOL };
	}
	if (node->data == "||")
	{
		const VariableTypeContainer left  = type_check_expr(current_scope, node->left,  node->data, { VariableType::BOOL });
		const VariableTypeContainer right = type_check_expr(current_scope, node->right, node->data, { VariableType::BOOL });

		if (left.find(VariableType::BOOL) == left.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "operator '||' can only be used on type 'bool'", "left hand value of operator '||' has " + get_var_types_as_str(left));
		if (right.find(VariableType::BOOL) == right.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "operator '||' can only be used on type 'bool'", "right hand value of operator '||' has " + get_var_types_as_str(right));

		return { VariableType::BOOL };
	}
	if (node->data == "&&")
	{
		const VariableTypeContainer left  = type_check_expr(current_scope, node->left,  node->data, { VariableType::BOOL });
		const VariableTypeContainer right = type_check_expr(current_scope, node->right, node->data, { VariableType::BOOL });

		if (left.find(VariableType::BOOL) == left.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "operator '&&' can only be used on type 'bool'", "left hand value of operator '&&' has " + get_var_types_as_str(left));
		if (right.find(VariableType::BOOL) == right.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "operator '&&' can only be used on type 'bool'", "right hand value of operator '&&' has " + get_var_types_as_str(right));

		return { VariableType::BOOL };
	}
	if (node->data == "==")
	{
		const VariableTypeContainer left = type_check_expr(current_scope, node->left, node->data, all_types);
		const VariableTypeContainer right = type_check_expr(current_scope, node->right, node->data, all_types);

		for (const VariableType& type : left)
		{
			if (right.find(type) != right.end())
				return { VariableType::BOOL };
		}

		throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "operator '==' can only be used to compare two equivalent types", "left hand value of operator '==' has " + get_var_types_as_str(left) + ", but right hand value has " + get_var_types_as_str(right));
	}
	if (node->data == "!=")
	{
		const VariableTypeContainer left = type_check_expr(current_scope, node->left, node->data, all_types);
		const VariableTypeContainer right = type_check_expr(current_scope, node->right, node->data, all_types);

		for (const VariableType& type : left)
		{
			if (right.find(type) != right.end())
				return { VariableType::BOOL };
		}
		
		throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "operator '!=' can only be used to compare two equivalent types", "left hand value of operator '!=' has " + get_var_types_as_str(left) + ", but right hand value has " + get_var_types_as_str(right));
	}
	if (node->data == ".")
	{
		// To Do:
		// type check method parameters
		//

		// find required types based on method

		VariableTypeContainer required_t;
		for (const auto& check_class : check_classes)
		{
			if (check_class.second.methods.find(node->right->data) == check_class.second.methods.end())
				continue;

			if (check_class.first == "array")
				required_t.insert({ VariableType::ARRAY });
			else if (check_class.first == "string")
				required_t.insert({ VariableType::STRING });
			else
				required_t.insert({ VariableType::CLASS, check_class.first });
		}

		const VariableTypeContainer object_types =
			type_check_expr(current_scope, node->left, node->data, required_t);

		if (object_types.find(VariableType::ARRAY) == object_types.end() &&
			object_types.find(VariableType::STRING) == object_types.end() &&
			object_types.find(VariableType::CLASS) == object_types.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "only objects can be used with methods", "variable '" + node->left->data + "' currently has " + get_var_types_as_str(object_types));

		// find return type of method, and convert arr and str to classes
		std::vector<decltype(check_classes.begin())> check_objects;
		for (const VariableType& object_type : object_types)
		{
			if (object_type == VariableType::ARRAY)
				check_objects.push_back(check_classes.begin());
			else if (object_type == VariableType::STRING)
				check_objects.push_back(std::next(check_classes.begin(), 1));
			else if (object_type == VariableType::CLASS)
				check_objects.push_back(check_classes.find(object_type.class_name));
		}

		VariableTypeContainer rtn_types;
		for (auto check_object : check_objects)
		{
			auto method = check_object->second.methods.find(node->right->data);
			if (method != check_object->second.methods.end())
			{
				rtn_types.insert(method->second.return_types.begin(),
					method->second.return_types.end());
			}
			else
			{
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, loc, "method '" + node->right->data + "' does not exit within class '" + check_object->first + "'");
			}
		}

		// when using .push() on an array, you're adding that type to the array
		// so that's why this piece of code is necessary
		if (node->right->data == "push")
		{
			auto* obj = get_variable(current_scope, node->left->data);
			obj->second.types.insert({ VariableType::BOOL, VariableType::INT, VariableType::FLOAT,
				VariableType::STRING, VariableType::ARRAY });
		}

		return rtn_types;
	}

	assert(false && "operator missing from list");
	return {};
}

void Parser::advance_to_close_bracket(
	const std::vector<Token>& units,
	std::vector<Token>::const_iterator& index
) {
	for (int open_bracket_count = 0; index != units.end(); ++index)
	{
		if (index->type == TokenType::OPEN_BRACKET)
		{
			open_bracket_count++;
		}
		else if (index->type == TokenType::CLOSE_BRACKET)
		{
			open_bracket_count--;
			if (open_bracket_count == 0)
				return;
		}
	}
}

/* private variables */

const VariableTypeContainer Parser::all_types{
	VariableType::BOOL, VariableType::INT, VariableType::FLOAT,
	VariableType::STRING, VariableType::ARRAY, VariableType::CLASS,
};

CheckFunctionContainer Parser::check_functions{
	make_check_function("print", { all_types }),
	make_check_function("input", {}, { VariableType::STRING }),

	make_check_function("range", { {VariableType::INT}, {VariableType::INT} }, { VariableType::ARRAY }),

	make_check_function("int", { { VariableType::INT }, { VariableType::FLOAT }, { VariableType::STRING } }, { VariableType::INT }),
	make_check_function("float", { { VariableType::INT }, { VariableType::FLOAT }, { VariableType::STRING } }, { VariableType::FLOAT }),

	make_check_function("system", { { VariableType::STRING } })
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