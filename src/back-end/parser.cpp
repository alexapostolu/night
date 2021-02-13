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
	: file(tokens[0].file), line(tokens[0].line)
{
	if (tokens[0].type == TokenType::SET)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::VAR)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "expected variable name after 'set' keyword", "variable initializations are required to be in this format: 'set variable_name = expression'");
		if (tokens.size() == 2 || tokens[2].type != TokenType::ASSIGN)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "expected assignment operator '=' after variable name", "variable initializations are required to be in this format: 'set variable_name = expression'");
		if (tokens.size() == 3)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "expected expression after assignment operator '='", "variable initializations are required to be in this format: 'set variable_name = expression'");
		if (current_scope->variables.find(tokens[1].data) != current_scope->variables.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, file, line, "variable '" + tokens[1].data + "' has already been defined", "variables can only be defined once, regardless of their scope");
		if (check_functions.find(tokens[1].data) != check_functions.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, file, line, "variable '" + tokens[1].data + "' has the same name as a function", "variable and function names are required to be unique");
		if (check_functions.find(tokens[1].data) != check_functions.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, file, line, "variable '" + tokens[1].data + "' has the same name as a class", "variable and class names are required to be unique");

		// extracting expression

		const std::vector<Value> values = TokensToValues(
			std::vector<Token>(tokens.begin() + 3, tokens.end()));

		const std::shared_ptr<Expression> expression = ValuesToExpression(values);

		bool is_array = false;
		const std::vector<VariableType> expression_types = TypeCheckExpression(current_scope, expression, {}, {}, &is_array);

		// pushing check variable and statement

		current_scope->variables[tokens[1].data] =
			CheckVariable{ expression_types, is_array };

		current_scope->statements.push_back(Statement{
			file, line, StatementType::VARIABLE,
			Variable{ tokens[1].data, expression }
		});
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VAR && tokens[1].type == TokenType::ASSIGN)
	{
		if (tokens.size() == 2)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "expected expression after assignment operator '='", "variable assignments are required to be in this format: '{variable name} = {expression}'");

		// definition and type checking variable

		const auto check_variable = get_variable(current_scope, tokens[0].data);
		if (check_variable == current_scope->variables.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, file, line, "variable '" + tokens[0].data + "' is not defined", "variables have to be defined before they are used");

		if (!check_variable->second.types.empty())
		{
			if (tokens[1].data == "+=" && !find_num_types(check_variable->second.types) &&
				!find_type(check_variable->second.types, VariableType::STR))
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, file, line, "variable '" + check_variable->first + "' can not be used with assignment operator '+='", "variable does not contain type 'int', 'float', or 'string'");
			if (tokens[1].data != "=" && tokens[1].data != "+=" && !find_num_types(check_variable->second.types))
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, file, line, "variable '" + check_variable->first + "' can not be used with assignment operator '" + tokens[1].data + "'", "variable does not contain type 'int' or 'float'");
		}

		// extracting and type checking expression

		const std::vector<Value> values = TokensToValues(
			std::vector<Token>(tokens.begin() + 2, tokens.end())
		);

		const std::shared_ptr<Expression> assign_expr = ValuesToExpression(values);
		const std::vector<VariableType> assign_types = TypeCheckExpression(current_scope, assign_expr);

		if (tokens[1].data == "+=" && !find_num_types(assign_types))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, file, line, "expression can not be assigned using the assignment operator '+='", "the assignment operator can only assign expressions of type 'int', 'float', or 'string'");
		if (tokens[1].data != "=" && tokens[1].data != "+=" && !find_num_types(assign_types) &&
			!find_type(assign_types, VariableType::STR))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, file, line, "expression can not be assigned using the assignment operator '" + tokens[1].data + "'", "the assignment operator can only assign expressions of type 'int' or 'float'");

		// determining assignment type

		Assignment assignment{ {}, tokens[0].data, assign_expr };
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

		check_variable->second.types.insert(check_variable->second.types.end(),
			assign_types.begin(), assign_types.end());

		current_scope->statements.push_back(Statement{
			file, line, StatementType::ASSIGNMENT, assignment });
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VAR && tokens[1].type == TokenType::OPEN_SQUARE)
	{
		// extracting and type checking subscript indices

		int open_bracket_count = 0;
		std::vector<VariableType> index_types;
		std::vector<std::shared_ptr<Expression> > index_exprs;
		auto it = tokens.begin() + 1;
		for (auto start = tokens.begin() + 2; it != tokens.end(); ++it)
		{
			if (it->type == TokenType::OPEN_BRACKET)
			{
				open_bracket_count++;
			}
			else if (it->type == TokenType::CLOSE_BRACKET)
			{
				if (--open_bracket_count == 0)
				{
					if (!index_types.empty() && !find_type(index_types, VariableType::ARRAY))
						throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, file, line, "subscript operator can only be used on arrays", "variable has type(s): " + get_var_types_as_str(index_types));

					const std::vector<Value> values = TokensToValues(std::vector<Token>(start, it));

					index_exprs.push_back(ValuesToExpression(values));
					index_types = TypeCheckExpression(current_scope, index_exprs.back());

					if (!find_num_types(index_types))
						throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, file, line, "index for array '" + tokens[0].data + "' is required to be of type 'int'", "index currently contains type(s): " + get_var_types_as_str(index_types));

					start = std::next(it, 2);
				}
			}
			else if (it->type == TokenType::ASSIGN)
			{
				if (open_bracket_count < 0)
					throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "expected opening square bracket in element assignment", "there are '" + std::to_string(open_bracket_count) + "unmatched opening square brackets");
				if (open_bracket_count > 0)
					throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "expected closing square bracket in element assignment", "there are '" + std::to_string(open_bracket_count) + "unmatched closing square brackets");

				break;
			}
		}

		if (it == tokens.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "expected assignment operator in element assignment", "element assignments must be in this form: 'variable[index] = expression'");

		// definition checking variable

		const auto check_variable = get_variable(current_scope, tokens[0].data);
		if (check_variable == current_scope->variables.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, file, line, "variable '" + tokens[1].data + "' is undefined", "variables have to be defined before they are used");

		// type checking variable

		if (!check_variable->second.is_array && find_type(check_variable->second.types, VariableType::STR))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, file, line, "subscript operator '[]' can only be used on variables of type 'string' or 'array'", "variable '" + check_variable->first+ "' currently has type(s): " + get_var_types_as_str(check_variable->second.types));

		// extracting expression

		const std::vector<Value> values = TokensToValues(
			std::vector<Token>(std::next(it, 1), tokens.end()));

		const std::shared_ptr<Expression> assign_expr = ValuesToExpression(values);
		const std::vector<VariableType> assign_types = TypeCheckExpression(current_scope, assign_expr);

		check_variable->second.types.insert(check_variable->second.types.end(),
			assign_types.begin(), assign_types.end());

		if (!check_variable->second.is_array && (assign_expr->type != ValueType::STR || assign_expr->data.length() >= 1))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, file, line, "a string element can only be assigned to a string of length 1", "individual characters can only be assigned to other individual characters");

		// pushing statement

		current_scope->statements.push_back(Statement{
			file, line, StatementType::ELEMENT,
			Element{ tokens[0].data, index_exprs, assign_expr }
		});
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VAR && tokens[1].type == TokenType::OPERATOR)
	{
		if (tokens[1].data != ".")
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_grammar, file, line, "grammar is invalid", "read documentation :)");

		const std::shared_ptr<Expression> object_expr = ValuesToExpression(TokensToValues(tokens));
		const std::vector<VariableType> object_types = TypeCheckExpression(current_scope, object_expr);

		// pushing statement

		current_scope->statements.push_back(Statement{
			file, line, StatementType::METHOD_CALL,
			MethodCall{ tokens[0].data, object_expr }
		});
	}

	else if (tokens[0].type == TokenType::IF)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::OPEN_BRACKET)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "expected an opening bracket after 'if' keyword", "if statements are required to be in this format: 'if ( {bool expression} ) {statement}'");
		if (tokens.size() == 2)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "expected a bool expression after opening bracket", "if statements are required to be in this format: 'if ( {bool expression} ) {statement}'");

		// extracting and type checking condition

		auto close_bracket_it = std::next(tokens.begin(), 1);
		advance_to_close_bracket(tokens, close_bracket_it);

		if (close_bracket_it == tokens.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "missing closing bracket for if statement", "if statements are required to be in this format: 'if ( {bool expression} ) {statement}'");

		const std::vector<Value> values = TokensToValues(
			std::vector<Token>(tokens.begin() + 2, close_bracket_it)
		);

		const std::shared_ptr<Expression> condition_expr = ValuesToExpression(values);
		const std::vector<VariableType> condition_types = TypeCheckExpression(current_scope, condition_expr);

		if (!find_type(condition_types, VariableType::BOOL))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, file, line, "if statement condition is required to be of type 'bool'", "the condition currently has type(s): " + get_var_types_as_str(condition_types));

		// extracting body

		close_bracket_it++;
		const std::vector<std::vector<Token> > split_tokens = close_bracket_it->type == TokenType::OPEN_CURLY
			? SplitCode(std::vector<Token>(std::next(close_bracket_it, 2), tokens.end() - 1))
			: SplitCode(std::vector<Token>(std::next(close_bracket_it, 1), tokens.end()));

		std::shared_ptr<Scope> body = std::make_shared<Scope>(current_scope);
		for (const std::vector<Token>& split_token : split_tokens)
			Parser parse(body, split_token);

		// pushing statement

		current_scope->statements.push_back(Statement{
			file, line, StatementType::IF_STATEMENT,
			IfStatement{ { Conditional{ condition_expr, body } } }
		});
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::ELSE && tokens[1].type == TokenType::IF)
	{
		IfStatement* const if_stmt = &std::get<IfStatement>(current_scope->statements.back().stmt);

		if (tokens.size() == 2 || tokens[2].type != TokenType::OPEN_BRACKET)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "expected an opening bracket after 'if' keyword", "else if statements are required to be in this format: 'else if ( {bool expression} ) {statement}'");
		if (current_scope->statements.empty() || current_scope->statements.back().type != StatementType::IF_STATEMENT)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_grammar, file, line, "else if statement does not precede an if or else if statement", "else if statements must come after an if or an else if statement");
		if (if_stmt->chains.back().condition == nullptr) // else statement
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_grammar, file, line, "else if statement can not precede an else statement", "else if statements are required to come after an if or else if statement");

		// extracting and type checking condition

		auto close_bracket_it = std::next(tokens.begin(), 2);
		advance_to_close_bracket(tokens, close_bracket_it);

		if (close_bracket_it == tokens.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "missing closing bracket for else if statement", "else if statements are required to be in this format: 'else if ( {bool expression} ) {statement}'");

		const std::vector<Value> values = TokensToValues(
			std::vector<Token>(tokens.begin() + 3, close_bracket_it));

		const std::shared_ptr<Expression> condition_expr = ValuesToExpression(values);
		const std::vector<VariableType> condition_types = TypeCheckExpression(current_scope, condition_expr);

		if (!find_type(condition_types, VariableType::BOOL))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, file, line, "else if statement condition is required to be of type 'bool'", "the condition currently has type(s): " + get_var_types_as_str(condition_types));

		// extracting body

		close_bracket_it++;
		const std::vector<std::vector<Token> > split_tokens = close_bracket_it->type == TokenType::OPEN_CURLY
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
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_grammar, file, line, "else statement does not precede an if or else if statement", "else statements must come after an if or an else if statement");
		if (if_stmt->chains.back().condition == nullptr) // else statement
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_grammar, file, line, "else statement can not precede another else statement", "else statements are required to come after an if or else if statement");

		// extracting body

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
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "expected function name after 'def' keyword", "function definitions are required to be in this format: 'def {function name} ( {parameters} ) {}'");
		if (tokens.size() == 2 || tokens[2].type != TokenType::OPEN_BRACKET)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "expected opening bracket after function name", "function definitions are required to be in this format: 'def {function name} ( {parameters} ) {}'");
		if (current_scope->variables.find(tokens[1].data) != current_scope->variables.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, file, line, "function '" + tokens[1].data + "' can not have the same name as a variable", "function and variable names are required to be unique");
		if (check_functions.find(tokens[1].data) != check_functions.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, file, line, "function '" + tokens[1].data + "' has already been define", "functions can only be defined once");
		if (check_classes.find(tokens[1].data) != check_classes.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, file, line, "function '" + tokens[1].data + "' can not have the same name as a class", "function and class names are required to be unique");
		if (current_scope->upper_scope != nullptr)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, file, line, "function '" + tokens[1].data + "' can not be defined inside of a local scope", "functions have to be defined in the global scope");

		// function parameters

		std::shared_ptr<Scope> function_scope = std::make_shared<Scope>(current_scope);

		std::vector<std::string> parameter_names;

		std::size_t close_bracket_index = 3;
		for (; close_bracket_index < tokens.size(); close_bracket_index += 2)
		{
			if (tokens[close_bracket_index].type != TokenType::VAR)
				CompileError(__FILE__, __LINE__, CompileError::invalid_grammar, file, line, "expected variable names as function parameters", "function parameters are required to be variables separated by spaces");

			if (function_scope->variables.find(tokens[1].data) != function_scope->variables.end())
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, file, line, "function parameter can not have the same name as another variable", "function parameters are required to be unique");
			if (check_functions.find(tokens[close_bracket_index].data) != check_functions.end())
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, file, line, "function parameter can not have the same name as a function", "function parameter names are required to be unique");
			if (check_classes.find(tokens[close_bracket_index].data) != check_classes.end())
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, file, line, "function parameter can not have the same name as a class", "function parameter names are required to be unique");

			parameter_names.push_back(tokens[close_bracket_index].data);
			function_scope->variables[tokens[close_bracket_index].data] = {};

			if (tokens[close_bracket_index + 1].type == TokenType::CLOSE_BRACKET)
			{
				close_bracket_index++;
				break;
			}

			if (tokens[close_bracket_index + 1].type != TokenType::COMMA)
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "expected comma or closing bracket after function parameter", "function parameters have to be followed by a comma or closing bracket");
		}

		if (tokens[close_bracket_index].type != TokenType::CLOSE_BRACKET)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "expected closing bracket after function parameters", "function parameters are required to end with a closing bracket");

		// define function before extracting body to ensure function is defined for recursion
		// and return types

		check_functions[tokens[1].data] = {};
		auto check_function = check_functions.find(tokens[1].data);

		check_function->second.parameters.resize(parameter_names.size());

		current_scope->statements.push_back(Statement{
			file, line, StatementType::FUNCTION_DEF,
			FunctionDef{ tokens[1].data, parameter_names }
		});

		FunctionDef* const function_def = &std::get<FunctionDef>(current_scope->statements.back().stmt);

		// extracting function body

		const std::vector<std::vector<Token> > split_tokens = tokens[close_bracket_index + 1].type == TokenType::OPEN_CURLY
			? SplitCode(std::vector<Token>(tokens.begin() + close_bracket_index + 2, tokens.end() - 1))
			: SplitCode(std::vector<Token>(tokens.begin() + close_bracket_index + 1, tokens.end()));

		for (const std::vector<Token>& split_token : split_tokens)
			Parser(function_scope, split_token);

		// check function

		check_function->second.is_void = !function_def->return_types.empty();

		for (std::size_t a = 0; a < function_def->parameters.size(); ++a)
		{
			check_function->second.parameters[a] = function_scope->variables[function_def->parameters[a]].types.empty()
				? all_types
				: function_scope->variables[function_def->parameters[a]].types;
		}

		check_function->second.return_types = function_def->return_types;
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VAR && tokens[1].type == TokenType::OPEN_BRACKET)
	{
		// extracting types and expressions for arguments

		std::vector<std::vector<VariableType> >   argument_types;
		std::vector<std::shared_ptr<Expression> > argument_expressions;

		int open_bracket_index = 0;
		std::size_t a = 1;
		for (std::size_t start = 2; a < tokens.size(); ++a)
		{
			if (tokens[a].type == TokenType::OPEN_BRACKET)
				open_bracket_index++;
			else if (tokens[a].type == TokenType::CLOSE_BRACKET)
				open_bracket_index--;

			if ((tokens[a].type == TokenType::COMMA && open_bracket_index == 1) ||
				(tokens[a].type == TokenType::CLOSE_BRACKET && open_bracket_index == 0))
			{
				if (a == 2 && tokens[a].type == TokenType::CLOSE_BRACKET)
					break;

				const std::vector<Value> values = TokensToValues(
					std::vector<Token>(tokens.begin() + start, tokens.begin() + a)
				);

				argument_expressions.push_back(ValuesToExpression(values));
				argument_types.push_back(TypeCheckExpression(current_scope, argument_expressions.back()));

				start = a + 1;

				if (tokens[a].type == TokenType::CLOSE_BRACKET)
					break;
			}
		}

		if (open_bracket_index < 0)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "missing opening bracket in function call", std::to_string(open_bracket_index) + " closing bracket(s) do(es) not have an associated opening bracket");
		if (open_bracket_index > 0)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "missing closing bracket in function call", std::to_string(open_bracket_index) + " opening bracket(s) do(es) not have an associated closing bracket");
	
		if (a < tokens.size() - 1)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "unexpected token(s) after statement function call", "each statement is required to be on its own line");

		// fetching check function

		auto check_function = check_functions.find(tokens[0].data);

		if (check_function == check_functions.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, file, line, "function '" + tokens[0].data + "' is undefined", "functions are required to be defined before they are called");

		// definition and type checking arguments with parameters

		if (argument_types.size() != check_function->second.parameters.size())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, file, line, "function '" + tokens[0].data + "' does not have the correct number of arguments", "the function was defined with '" + std::to_string(check_function->second.parameters.size()) + "' parameter(s), but is called with '" + std::to_string(argument_types.size()) + "' argument(s)");

		for (std::size_t a = 0; a < argument_types.size(); ++a)
		{
			bool match = false;
			for (std::size_t b = 0; b < check_function->second.parameters[a].size() && !match; ++b)
			{
				if (find_type(argument_types[a], check_function->second.parameters[a][b]))
					match = true;
			}

			if (!match)
			{
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, file, line, "argument number '" + std::to_string(a + 1) + "' for function call '" + tokens[0].data + "' is not the required type", "the argument can only be of type(s): " + get_var_types_as_str(check_function->second.parameters[a]));
			}
		}

		// pushing statement

		current_scope->statements.push_back(Statement{
			file, line, StatementType::FUNCTION_CALL,
			FunctionCall{ tokens[0].data, argument_expressions }
		});
	}
	else if (tokens[0].type == TokenType::RETURN)
	{
		// a note on the function scope:
		//
		// make sure the function statement is pushed to the statements array
		// before making the recursive call to extract the body
		//
		// if that doesn't happen then this won't work, since here it climbs up
		// scopes in order to find the function scope

		Scope* function_scope = current_scope->upper_scope.get();
		for (; function_scope->upper_scope != nullptr; function_scope = function_scope->upper_scope.get());

		if (function_scope->statements.back().type != StatementType::FUNCTION_DEF)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_grammar, file, line, "return statement can not be outside of a function", "return statements are required to be inside of functions");
		if (tokens.size() == 1)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "expected expression after 'return' keyword", "return statements are required to be in this format: 'return {expression}'");

		// extracting return expression

		const std::vector<Value> values = TokensToValues(
			std::vector<Token>(tokens.begin() + 1, tokens.end())
		);

		const std::shared_ptr<Expression> return_expr = ValuesToExpression(values);
		const std::vector<VariableType> return_expr_types = TypeCheckExpression(current_scope, return_expr);

		// adding types to return types and pushing statement

		FunctionDef* const function_def = &std::get<FunctionDef>(function_scope->statements.back().stmt);
		function_def->return_types.insert(function_def->return_types.end(),
			return_expr_types.begin(), return_expr_types.end() );

		current_scope->statements.push_back(Statement{
			file, line, StatementType::RETURN,
			Return{ return_expr }
		});
	}
	
	else if (tokens[0].type == TokenType::WHILE)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::OPEN_BRACKET)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "expected an opening bracket after 'while' keyword", "while loop statements are required to be in this format: 'while ( {bool expression} ) {statement}'");

		// extracting and type checking condition

		auto close_bracket_index = std::next(tokens.begin(), 2);
		advance_to_close_bracket(tokens, close_bracket_index);

		if (close_bracket_index == tokens.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "missing closing bracket for while loop condition", "while loop statements are required to be in this format: 'while ( {bool expression} ) {statement}'");

		const std::vector<Value> values = TokensToValues(
			std::vector<Token>(tokens.begin() + 2, close_bracket_index));

		const std::shared_ptr<Expression> condition_expr = ValuesToExpression(values);
		const std::vector<VariableType> condition_types = TypeCheckExpression(current_scope, condition_expr);

		if (!find_type(condition_types, VariableType::BOOL))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, file, line, "while loop condition can not be used", "while loop condition contains type(s): " + get_var_types_as_str(condition_types) + "; however, while loop conditions are required to contain type 'bool'");

		// extracting body

		const std::vector<std::vector<Token> > tokens_split = std::next(close_bracket_index, 1)->type == TokenType::OPEN_CURLY
			? SplitCode(std::vector<Token>(std::next(close_bracket_index, 2), tokens.end() - 1))
			: SplitCode(std::vector<Token>(std::next(close_bracket_index, 1), tokens.end()));

		std::shared_ptr<Scope> scope_body = std::make_shared<Scope>(current_scope);
		for (const std::vector<Token>& token_split : tokens_split)
			Parser(scope_body, token_split);

		// pushing statement

		current_scope->statements.push_back(Statement{
			file, line, StatementType::WHILE_LOOP,
			WhileLoop{ condition_expr, scope_body }
		});
	}
	else if (tokens[0].type == TokenType::FOR)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::OPEN_BRACKET)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "expected opening bracket after 'for' keyword", "for loops must be in this format: 'for (iterator : range) {}'");
		if (tokens.size() == 2 || tokens[2].type != TokenType::VAR)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "expected iterator after opening bracket", "for loops must be in this format: 'for (iterator : range) {}'");
		if (tokens.size() == 3 || tokens[3].type != TokenType::COLON)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "expected colon after iterator", "for loops must be in this format: 'for (iterator : range) {}'");
		if (tokens.size() == 4)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "expected range after colon", "for loops must be in this format: 'for (iterator : range) {}'");

		// evaluating iterator and range

		auto close_bracket_index = std::next(tokens.begin(), 1);
		advance_to_close_bracket(tokens, close_bracket_index);

		if (close_bracket_index == tokens.end())
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "missing closing bracket in for loop", "for loops must be in this format: 'for (condition) {}'");

		const std::vector<Value> range_values = TokensToValues(std::vector<Token>(tokens.begin() + 4, close_bracket_index));
		const std::shared_ptr<Expression> range_expr = ValuesToExpression(range_values);

		// also makes sure range is type array or string
		bool is_array = false;
		const std::vector<VariableType> range_types = TypeCheckExpression(current_scope, range_expr, "", {}, &is_array);

		if (!is_array && !find_type(range_types, VariableType::STR))
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, file, line, "for loop range is required to be of type 'array'", "range currently has type(s): " + get_var_types_as_str(range_types));

		// extracting scope and constructing statement

		const std::vector<std::vector<Token> > split_tokens = std::next(close_bracket_index, 1)->type == TokenType::OPEN_CURLY
			? SplitCode(std::vector<Token>(std::next(close_bracket_index, 2), tokens.end() - 1))
			: SplitCode(std::vector<Token>(std::next(close_bracket_index, 1), tokens.end()));

		std::shared_ptr<Scope> for_scope = std::make_shared<Scope>(current_scope);
		for_scope->variables[tokens[2].data] = { range_types }; // iterator

		for (const std::vector<Token>& split_token : split_tokens)
			Parser(for_scope, split_token);

		current_scope->statements.push_back(Statement{
			file, line, StatementType::FOR_LOOP,
			ForLoop{ tokens[2].data, range_expr, for_scope }
		});
	}
	
	else
	{
		throw CompileError(__FILE__, __LINE__, CompileError::invalid_grammar, file, line, "grammar is invalid", "read documentation :)");
	}
}

std::unordered_map<std::string, CheckVariable>::iterator Parser::get_variable(
	const std::shared_ptr<Scope>& scope,
	const std::string& variable_name
) {
	for (Scope* current_scope = scope.get(); current_scope != nullptr;)
	{
		auto it = current_scope->variables.find(variable_name);
		if (it != current_scope->variables.end())
			return it;

		current_scope = current_scope->upper_scope.get();
	}

	return scope->variables.end();
}

std::vector<Value> Parser::TokensToValues(const std::vector<Token>& tokens)
{
	assert(!tokens.empty() && "tokens shouldn't be empty");

	// extract expression

	std::vector<Value> values;
	for (std::size_t a = 0; a < tokens.size(); ++a)
	{
		if ((tokens[a].type == TokenType::VAR || tokens[a].type == TokenType::STR) &&
			a < tokens.size() - 1 && tokens[a + 1].type == TokenType::OPEN_SQUARE)
		{
			if (tokens[a].type == TokenType::VAR)
				values.push_back(Value{ ValueType::VARIABLE, tokens[a].data });
			else
				values.push_back(Value{ ValueType::STR, tokens[a].data });

			while (a < tokens.size() - 1 && tokens[a + 1].type == TokenType::OPEN_SQUARE)
			{
				a += 2;

				auto start = std::next(tokens.begin(), a);
				advance_to_close_bracket(tokens, start);

				if (a >= tokens.size())
					throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "closing square bracket missing for subscript operator", "subscript operator needs squaries");

				values.push_back(Value{
					ValueType::OPERATOR, "[]",
					{ TokensToValues(std::vector<Token>(start, tokens.end() + a)) }
				});
			}
		}
		else if (a < tokens.size() - 1 && tokens[a].type == TokenType::VAR && tokens[a + 1].type == TokenType::OPEN_BRACKET)
		{
			Value value_call{ ValueType::CALL, tokens[a].data };

			a += 2;

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
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "expecting closing bracket for function call '", "function call '" + tokens[a].data + "' does not have a closing bracket");

			values.push_back(value_call);
		}
		else if (tokens[a].type == TokenType::OPEN_SQUARE)
		{
			Value value_array{ ValueType::ARRAY, "[..]" };

			a++;

			if (tokens[a].type == TokenType::CLOSE_SQUARE)
			{
				values.push_back(value_array);
				continue;
			}

			// parse array elements
			int open_bracket_count = 0, open_square_count = 0;
			for (std::size_t start = a; a < tokens.size(); ++a)
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
						std::vector<Token>(tokens.begin() + start, tokens.begin() + a));

					value_array.extras.push_back(temp_values);

					start = a + 1;

					if (tokens[a].type == TokenType::CLOSE_SQUARE)
						break;

					continue;
				}
			}

			if (a >= tokens.size())
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "missing closing square bracket for array", "opening square bracket does not have an associated closing square bracket");

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
			case TokenType::STR:
				values.push_back(Value{ ValueType::STR, tokens[a].data });
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
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_grammar, file, line, "unexpected token '" + tokens[a].data + "' in expression", "expressions can only contain values, variables, arrays, functions, and operators");
			}
		}
	}

	// check expression

	for (std::size_t a = 0; a < values.size() - 1; ++a)
	{
		if (values[a].type != ValueType::OPEN_BRACKET && values[a].type != ValueType::CLOSE_BRACKET &&
			values[a].type != ValueType::OPERATOR && values[a + 1].type != ValueType::OPEN_BRACKET &&
			values[a + 1].type != ValueType::CLOSE_BRACKET && values[a + 1].type != ValueType::OPERATOR)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "expected operator in between values", "expression has to contains binary operators between two values");
	}

	return values;
}

std::shared_ptr<Expression> Parser::new_expression(
	const Value& value,
	const std::shared_ptr<Expression>& left,
	const std::shared_ptr<Expression>& right
) {
	std::shared_ptr<Expression> expression = std::make_shared<Expression>(Expression{
		file, line,
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
		throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "unary operators cannot be adjacent to one another", "unary operators can only be next to a value");

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

std::vector<VariableType> Parser::TypeCheckExpression(
	const std::shared_ptr<Scope>& current_scope, 
	const std::shared_ptr<Expression>& node,
	const std::string& op_name,
	const std::vector<VariableType>& required_types,
	bool* is_for_loop_range
) {
	assert(node != nullptr && "node cannot be NULL");

	// if node->left and node->right is NULL, then node must be a value
	if (node->left == nullptr && node->right == nullptr)
	{
		if (node->type == ValueType::ARRAY)
		{
			if (is_for_loop_range == nullptr)
				return std::vector<VariableType>{ VariableType::ARRAY };

			*is_for_loop_range = true;

			std::vector<VariableType> element_types;
			for (const std::shared_ptr<Expression>& element : node->extras)
			{
				const std::vector<VariableType> element_type = TypeCheckExpression(current_scope, element);
				element_types.insert(element_types.end(), element_type.begin(), element_type.end());
			}

			return element_types;
		}
		if (node->type == ValueType::VARIABLE)
		{
			const auto check_variable = get_variable(current_scope, node->data);
			if (check_variable == current_scope->variables.end())
			{
				auto check_object = std::find(check_classes.cbegin(),
					check_classes.cend(), node->data );

				if (check_object == check_classes.end())
					throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, file, line, "variable '" + node->data + "' is undefined", "variables have to be defined before they are used");
				
				return { { VariableType::CLASS, check_object->first } };
			}

			if (is_for_loop_range != nullptr)
			{
				*is_for_loop_range = true;
				return check_variable->second.types;
			}

			if (check_variable->second.types.empty()) // is a parameter
				check_variable->second.types = required_types;

			return check_variable->second.types;
		}
		if (node->type == ValueType::CALL)
		{
			auto check_function = std::find(check_functions.begin(),
				check_functions.end(), node->data );

			if (check_function == check_functions.end())
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_definition, file, line, "function '" + node->data + "' is undefined", "functions have to be defined before they are used");
			if (check_function->second.is_void)
				throw BackError(file, line, "function '" + node->data + "' doesn't return a value; functions used in expressions must return a value");
			if (node->extras.size() != check_function->second.parameters.size())
				throw BackError(file, line, "function '" + node->data + "' was called with '" + std::to_string(node->extras.size()) + "' arguments, but was defined with '" + std::to_string(check_function->second.parameters.size()) + "' parameters");

			// type checking function parameters
			for (std::size_t a = 0; a < check_function->second.parameters.size(); ++a)
			{
				const std::vector<VariableType> argumentTypes = TypeCheckExpression(current_scope, node->extras[a], "", {});

				if (check_function->parameters[a].empty())
				{
					check_function->parameters[a] = argumentTypes;
					continue;
				}

				for (const VariableType& argumentType : argumentTypes)
				{
					if (!find_type(check_function->parameters[a], argumentType))
						throw BackError(file, line, "argument number '" + std::to_string(a + 1) + "' for function call '" + check_function->name + "' cannot be used because of a type mismatch");
				}
			}

			// type checking return values
			if (check_function->second.return_types.empty())
			{
				check_function->second.return_types = required_types;
			}
			else if (!required_types.empty())
			{
				for (std::size_t a = 0; a < required_types.size(); ++a)
				{
					if (find_type(check_function->second.return_types, required_types[a]))
						return check_function->second.return_types;
				}

				throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, file, line, "function '" + check_function->first + "' can not be used with operator '" + op_name + "'", "operator requires type(s): " + get_var_types_as_str(required_types) + ", but function has return type(s): " + get_var_types_as_str(check_function->second.return_types));
			}

			return check_function->second.return_types;
		}

		switch (node->type)
		{
		case ValueType::BOOL:
			return { VariableType::BOOL };
		case ValueType::INT:
			return { VariableType::INT };
		case ValueType::FLOAT:
			return { VariableType::FLOAT };
		case ValueType::STR:
			return { VariableType::STR };
		default:
			assert(false);
		}
	}

	assert(node->type == ValueType::OPERATOR);

	if (node->data == "+")
	{
		const std::vector<VariableType> left =
			TypeCheckExpression(current_scope, node->left, node->data, { VariableType::INT, VariableType::FLOAT, VariableType::STR });
		const std::vector<VariableType> right =
			TypeCheckExpression(current_scope, node->right, node->data, { VariableType::INT, VariableType::FLOAT, VariableType::STR });

		std::vector<VariableType> add_types;
		if (find_type(left, VariableType::INT) && find_type(right, VariableType::INT))
			add_types.push_back(VariableType::INT);
		if ((find_type(left, VariableType::FLOAT) && find_num_types(right)) ||
			(find_type(right, VariableType::FLOAT) && find_num_types(right)))
			add_types.push_back(VariableType::FLOAT);
		if (find_type(left, VariableType::STR) || find_type(right, VariableType::STR))
			add_types.push_back(VariableType::STR);

		if (add_types.empty())
			throw BackError(file, line, "binary operator '" + node->data + "' requires two types of 'num', or at least one type of 'str'");

		return add_types;
	}
	if (node->data == "-")
	{
		// unary operator (negative)
		if (node->left == nullptr)
		{
			const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, { VariableType::INT, VariableType::FLOAT });

			if (!find_num_types(right))
				throw BackError(file, line, "unary operator '" + node->data + "' requires to be used on a type of 'num'");

			return { VariableType::INT, VariableType::FLOAT };
		}

		// binary operator (minus)

		const std::vector<VariableType> left  = TypeCheckExpression(current_scope, node->left,  node->data, { VariableType::INT, VariableType::FLOAT });
		const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, { VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(left) || !find_num_types(right))
			throw BackError(file, line, "binary operator '" + node->data + "' can only be used on two numbers");
		
		return { VariableType::INT, VariableType::FLOAT };
	}
	if (node->data == "*")
	{
		const std::vector<VariableType> left = TypeCheckExpression(
			current_scope, node->left, node->data, { VariableType::INT, VariableType::FLOAT });
		const std::vector<VariableType> right = TypeCheckExpression(
			current_scope, node->right, node->data, { VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(left) || !find_num_types(right))
			throw BackError(file, line, "binary operator '" + node->data + "' can only be used on two numbers");
		
		return { VariableType::INT, VariableType::FLOAT };
	}
	if (node->data == "/")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(current_scope, node->left,  node->data, { VariableType::INT, VariableType::FLOAT });
		const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, { VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(left) || !find_num_types(right))
			throw BackError(file, line, "binary operator '" + node->data + "' can only be used on two numbers");
		
		return { VariableType::INT, VariableType::FLOAT };
	}
	if (node->data == "%")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(current_scope, node->left,  node->data, { VariableType::INT });
		const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, { VariableType::INT });

		if (!find_num_types(left) || !find_num_types(right))
			throw BackError(file, line, "binary operator '" + node->data + "' can only be used on two integers");

		return { VariableType::INT };
	}
	if (node->data == ">")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(current_scope, node->left, node->data, { VariableType::INT, VariableType::FLOAT });
		const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, { VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(left) || !find_num_types(right))
			throw BackError(file, line, "binary operator '" + node->data + "' can only be used on two values of type 'num'");

		return { VariableType::BOOL };
	}
	if (node->data == "<")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(current_scope, node->left,  node->data, { VariableType::INT, VariableType::FLOAT });
		const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, { VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(left) || !find_num_types(right))
			throw BackError(file, line, "binary operator '" + node->data + "' can only be used on two values of type 'num'");

		return { VariableType::BOOL };
	}
	if (node->data == ">=")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(current_scope, node->left,  node->data, { VariableType::INT, VariableType::FLOAT });
		const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, { VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(left) || !find_num_types(right))
			throw BackError(file, line, "binary operator '" + node->data + "' can only be used on two values of type 'bool'");

		return { VariableType::BOOL };
	}
	if (node->data == "<=")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(current_scope, node->left,  node->data, { VariableType::INT, VariableType::FLOAT });
		const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, { VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(left) || !find_num_types(right))
			throw BackError(file, line, "binary operator '" + node->data + "' can only be used on two values of type 'bool'");

		return { VariableType::BOOL };
	}
	if (node->data == "!")
	{
		const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, { VariableType::BOOL });

		if (!find_type(right, VariableType::BOOL))
			throw BackError(file, line, "unary operator '" + node->data + "' can only be used on a value of type 'bool'");

		return { VariableType::BOOL };
	}
	if (node->data == "||")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(current_scope, node->left,  node->data, { VariableType::BOOL });
		const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, { VariableType::BOOL });

		if (!find_type(left, VariableType::BOOL) || !find_type(right, VariableType::BOOL))
			throw BackError(file, line, "binary operator '" + node->data + "' can only be used on two values of type 'bool'");

		return { VariableType::BOOL };
	}
	if (node->data == "&&")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(current_scope, node->left,  node->data, { VariableType::BOOL });
		const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, { VariableType::BOOL });

		if (!find_type(left, VariableType::BOOL) || !find_type(right, VariableType::BOOL))
			throw BackError(file, line, "binary operator '" + node->data + "' can only be used on two values of type 'bool'");

		return { VariableType::BOOL };
	}
	if (node->data == "==")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(current_scope, node->left,  node->data, all_types);
		const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, all_types);

		for (const VariableType& type : left)
		{
			if (!find_type(right, type))
				throw BackError(file, line, "binary operator '" + node->data + "' requires two values of the same type");
		}

		return { VariableType::BOOL };
	}
	if (node->data == "!=")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(current_scope, node->left,  node->data,  all_types);
		const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, all_types);

		for (const VariableType& type : left)
		{
			if (!find_type(right, type))
				throw BackError(file, line, "binary operator '" + node->data + "' requires two values of the same type");
		}

		return { VariableType::BOOL };
	}
	if (node->data == "[]")
	{
		// get individual array elements
		bool giae = false;
		const std::vector<VariableType> array_types = TypeCheckExpression(current_scope, node->right,     node->data, { VariableType::ARRAY }, &giae);
		const std::vector<VariableType> index_types = TypeCheckExpression(current_scope, node->extras[0], node->data, { VariableType::INT });

		if (!giae)
			throw CompileError(__FILE__, __LINE__, CompileError::invalid_type, file, line, "operator '" + node->data + "' can only be used on types 'string' or 'array'", "expression currently has type(s): " + get_var_types_as_str(array_types));
			throw BackError(file, line, "operator '" + node->data + "' requires to be used on a string or array");
		if (!find_num_types(index_types))
			throw BackError(file, line, "operator '" + node->data + "' requires subscript to be type 'num'");
 
		return array_types;
	}
	if (node->data == ".")
	{
		assert(node->right->left == nullptr && node->right->right == nullptr && "method can't be part of an expression");

		// the required_types argument is faulty; check to see what methods are present, then send them through to the function
		// so if object is a string, then only send VariableType::STR through
		const std::vector<VariableType> object_types = TypeCheckExpression(current_scope, node->left, node->data,
			{ VariableType::STR, VariableType::ARRAY, VariableType::CLASS });

		if (!find_type(object_types, VariableType::ARRAY) && !find_type(object_types, VariableType::STR) &&
			!find_type(object_types, VariableType::CLASS))
			throw BackError(file, line, "variable '" + node->left->data + "' is used with methods; only objects can be used with methods");

		// find return type of method, and convert arr and str to classes
		std::vector<decltype(check_classes.begin())> check_objects;
		for (const VariableType& object_type : object_types)
		{
			if (object_type == VariableType::ARRAY)
			{
				check_objects.push_back(check_classes.begin());
			}
			else if (object_type == VariableType::STR)
			{
				check_objects.push_back(++check_classes.begin());
			}
			else if (object_type == VariableType::CLASS)
			{
				check_objects.push_back(
					std::find(check_classes.begin(), check_classes.end(),
						object_type.class_name)
				);
			}

		}

		// test:
		//
		// is this good logic?
		// i too confusion
		// confuzzled squid
		//
		//
		std::vector<VariableType> rtn_types;
		for (auto check_object : check_objects)
		{
			for (const CheckFunction& method : check_object->methods)
			{
				if (method.name == node->right->data)
				{
					rtn_types.insert(rtn_types.end(),
						method.return_types.begin(), method.return_types.end());

					break;
				}
			}
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
	int open_bracket_count = 0;
	for (; index != units.end(); ++index)
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

const std::vector<VariableType> Parser::all_types{
	VariableType::BOOL, VariableType::INT, VariableType::FLOAT,
	VariableType::STR, VariableType::ARRAY
};

std::unordered_map<std::string, CheckFunction> Parser::check_functions{
	{ "print", CheckFunction{ { { all_types } }, std::vector<VariableType>()                    } },
	{ "input", CheckFunction{ {},                std::vector<VariableType>{ VariableType::STR } } }
};
std::unordered_map<std::string, CheckClass>    Parser::check_classes{
	{ "array", CheckClass{
		std::vector<CheckVariable>(),
		std::unordered_map<std::string, CheckFunction>{
			{ "len", {  {},                                                 std::vector<VariableType>{ VariableType::INT } } },
			{ "push", { { { VariableType::ARRAY } },                        std::vector<VariableType>()			           } },
			{ "push", { { { VariableType::INT }, { VariableType::ARRAY } }, std::vector<VariableType>()					   } },
			{ "pop", {  {},                                                 std::vector<VariableType>()			           } },
			{ "pop", {  { { VariableType::INT } },                          std::vector<VariableType>()					   } }
		}
	} },
	{ "string", CheckClass{
		std::vector<CheckVariable>(),
		std::unordered_map<std::string, CheckFunction>{
			{ "len", CheckFunction{ {}, std::vector<VariableType>{ VariableType::INT } } }
		}
	} }
};