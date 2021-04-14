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
		if (tokens.size() == 1 || tokens[1].type != TokenType::VARIABLE)
			throw NIGHT_COMPILE_ERROR("expected variable name after 'set' keyword", "variable initializations must be in this format: 'set variable = expression'", Learn::VARIABLES);
		if (tokens.size() == 2 || tokens[2].type != TokenType::ASSIGN)
			throw NIGHT_COMPILE_ERROR("expected assignment operator after variable name", "variable initializations must be in this format: 'set variable = expression'", Learn::VARIABLES);
		if (tokens.size() == 3)
			throw NIGHT_COMPILE_ERROR("expected expression after assignment operator", "variable initializations must be in this format: 'set variable = expression'", Learn::VARIABLES);
		
		if (current_scope->variables.contains(tokens[1].data))
			throw NIGHT_COMPILE_ERROR("variable '" + tokens[1].data + "' has already been defined", "variables can only be defined once, regardless of their scope", Learn::VARIABLES);
		if (check_functions.contains(tokens[1].data))
			throw NIGHT_COMPILE_ERROR("variable '" + tokens[1].data + "' has the same name as a function", "variable and function names must be unique", Learn::VARIABLES);
		if (check_classes.contains(tokens[1].data))
			throw NIGHT_COMPILE_ERROR("variable '" + tokens[1].data + "' has the same name as a class", "variable and class names must be unique", Learn::VARIABLES);

		// parsing expression

		const std::vector<Value> values = TokensToValues(
			std::vector<Token>(tokens.begin() + 3, tokens.end()));

		const std::shared_ptr<Expression> assign_expr =
			ValuesToExpression(values);

		const VariableTypeContainer assign_types =
			type_check_expr(current_scope, assign_expr);

		// creating check variable and statement

		current_scope->variables[tokens[1].data] =
			CheckVariable(assign_types);

		current_scope->statements.push_back(Statement{
			loc, StatementType::VARIABLE,
			Variable{ tokens[1].data, assign_expr }
		});
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VARIABLE && tokens[1].type == TokenType::ASSIGN)
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
	
	
	// delete these
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

	else if (tokens[0].type == TokenType::VARIABLE)
	{
		if (tokens.size() == 1)
			throw NIGHT_COMPILE_ERROR("variable statement ");

		auto* const check_variable = get_variable(current_scope, tokens[0].data);
		if (check_variable == nullptr)
			throw NIGHT_COMPILE_ERROR("variable '" + tokens[0].data + "' is undefined", "variables must be defined before they are used", Learn::VARIABLES);

		VariableTypeContainer prev_types = check_variable->second.types;
		std::unordered_set<VariableType, HashVariableType>* array_types = nullptr;
		
		auto it = check_variable->second.types.find(VariableType::ARRAY);
		if (it != check_variable->second.types.end())
			array_types = it->get_elem_types();

		for (auto it = tokens.begin() + 1; it != tokens.end(); ++it)
		{
			if (it->type == TokenType::OPEN_SQUARE)
			{
				auto start = it + 1;
				for (int open_square_count = 0; it != tokens.end(); ++it)
				{
					if (it->type == TokenType::OPEN_SQUARE)
					{
						open_square_count++;
					}
					else if (it->type == TokenType::CLOSE_SQUARE)
					{
						open_square_count--;
						if (open_square_count == 0)
							break;
					}
				}

				if (it == tokens.end())
					throw NIGHT_COMPILE_ERROR("missing closing square bracket in subscript operator", "subscript operator must be used in this format: 'array[index]'", Learn::ARRAYS);
				if (array_types.empty() && !prev_types.contains(VariableType::STRING))
					throw NIGHT_COMPILE_ERROR("subscript operator can only be used on types: 'str' or 'arr'", "left hand value of subscript operator currently contains " + get_var_types_as_str(prev_types), Learn::ARRAYS);

				const std::vector<Value> values =
					TokensToValues(std::vector<Token>(start, it));

				const std::shared_ptr<Expression> index_expr =
					ValuesToExpression(values);

				const VariableTypeContainer index_types =
					type_check_expr(current_scope, index_expr);

				if (!index_types.contains(VariableType::INT))
					throw NIGHT_COMPILE_ERROR("subscript index must contain type: 'int'", "index currently contains " + get_var_types_as_str(index_types), Learn::ARRAYS);

				for (auto it = array_types.begin(), end = array_types.end(); it != end; ++it)
				{
					auto pos = (*it)->find(VariableType::ARRAY);
					if (pos != )
					for (auto pos = (*it)->begin(); pos != (*it)->end(); ++it)
					{
						if (*pos == VariableType::ARRAY)
					}
				}
			}
			else if (it->type == TokenType::OPERATOR && it->data == ".")
			{
				
			}
			else
			{
				// throw NIGHT_COMPILE_ERROR
			}
		}

		current_scope->statements.push_back(Statement{
			loc, StatementType::ELEMENT,
			Element{ tokens[0].data, index_exprs, assign_expr }
		});
	}

	else if (tokens[0].type == TokenType::IF)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::OPEN_BRACKET)
			throw NIGHT_COMPILE_ERROR("expected opening bracket after 'if' keyword", "if statements must be in this format: 'if (condition) {}'", Learn::CONDITIONALS);
		if (tokens.size() == 2)
			throw NIGHT_COMPILE_ERROR("expected an expression after opening bracket", "if statements must be in this format: 'if (condition) {}'", Learn::CONDITIONALS);

		// parsing and type checking condition

		auto close_bracket_it = std::next(tokens.begin(), 1);
		advance_to_close_bracket(tokens, close_bracket_it);

		if (close_bracket_it == tokens.end())
			throw NIGHT_COMPILE_ERROR("missing closing bracket for if statement", "if statements must be in this format: 'if (condition) {}'", Learn::CONDITIONALS);

		const std::vector<Value> values = TokensToValues(
			std::vector<Token>(tokens.begin() + 2, close_bracket_it));

		const std::shared_ptr<Expression> condition_expr =
			ValuesToExpression(values);

		const VariableTypeContainer condition_types = type_check_expr(
			current_scope, condition_expr, { VariableType::BOOL });

		if (!condition_types.contains(VariableType::BOOL))
			throw NIGHT_COMPILE_ERROR("if statement condition must contain type: 'bool'", "condition currently contains " + get_var_types_as_str(condition_types), Learn::CONDITIONALS);

		// extracting body
		
		const std::vector<std::vector<Token> > split_tokens = SplitCode(
			std::vector<Token>(std::next(close_bracket_it, 1), tokens.end()));

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
		if (tokens.size() == 2 || tokens[2].type != TokenType::OPEN_BRACKET)
			throw NIGHT_COMPILE_ERROR("expected opening bracket after 'if' keyword", "else if statements must be in this format: 'else if (expression) {}'", Learn::CONDITIONALS);
		if (current_scope->statements.empty() || current_scope->statements.back().type != StatementType::IF_STATEMENT)
			throw NIGHT_COMPILE_ERROR("else if statement does not precede an if or else if statement", "else if statements must come after an if or an else if statement", Learn::CONDITIONALS);

		IfStatement* const if_stmt =
			&std::get<IfStatement>(current_scope->statements.back().stmt);

		if (if_stmt->chains.back().condition == nullptr) // else statement
			throw NIGHT_COMPILE_ERROR("else if statement can not precede an else statement", "else if statements are required to come after an if or else if statement", Learn::CONDITIONALS);

		// extracting and type checking condition

		auto close_bracket_it = std::next(tokens.begin(), 2);
		advance_to_close_bracket(tokens, close_bracket_it);

		if (close_bracket_it == tokens.end())
			throw NIGHT_COMPILE_ERROR("missing closing bracket for else if statement", "else if statements must be in this format: 'else if (condition) {}'", Learn::CONDITIONALS);

		const std::vector<Value> values = TokensToValues(
			std::vector<Token>(tokens.begin() + 3, close_bracket_it));

		const std::shared_ptr<Expression> condition_expr =
			ValuesToExpression(values);

		const VariableTypeContainer condition_types = type_check_expr(
			current_scope, condition_expr, { VariableType::BOOL });

		if (!condition_types.contains(VariableType::BOOL))
			throw NIGHT_COMPILE_ERROR("else if condition must contain type 'bool'", "condition currently contains " + get_var_types_as_str(condition_types), Learn::CONDITIONALS);

		// extracting body

		const std::vector<std::vector<Token> > split_tokens = SplitCode(
			std::vector<Token>(std::next(close_bracket_it, 1), tokens.end()));

		std::shared_ptr<Scope> body = std::make_shared<Scope>(current_scope);
		for (const std::vector<Token>& split_token : split_tokens)
			Parser(body, split_token);

		// pushing statement

		if_stmt->chains.push_back(Conditional{ condition_expr, body });
	}
	else if (tokens[0].type == TokenType::ELSE)
	{
		if (current_scope->statements.empty() || current_scope->statements.back().type != StatementType::IF_STATEMENT)
			throw NIGHT_COMPILE_ERROR("else statement does not precede an if or else if statement", "else statements must come after an if or an else if statement", Learn::CONDITIONALS);

		IfStatement* const if_stmt = &std::get<IfStatement>(current_scope->statements.back().stmt);
		if (if_stmt->chains.back().condition == nullptr) // else statement
			throw NIGHT_COMPILE_ERROR("else statement can not precede another else statement", "else statements must come after an if or else if statement", Learn::CONDITIONALS);

		// parsing body

		const std::vector<std::vector<Token> > split_tokens =
			SplitCode(std::vector<Token>(tokens.begin() + 1, tokens.end()));

		std::shared_ptr<Scope> body = std::make_shared<Scope>(current_scope);
		for (const std::vector<Token>& split_token : split_tokens)
			Parser parse(body, split_token);

		// pushing statement

		if_stmt->chains.push_back(Conditional{ nullptr, body });
	}
	
	else if (tokens[0].type == TokenType::DEF)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::VARIABLE)
			throw NIGHT_COMPILE_ERROR("expected function name after 'def' keyword", "function definitions must be in this format: 'def function (parameters) {}'", Learn::FUNCTIONS);
		if (tokens.size() == 2 || tokens[2].type != TokenType::OPEN_BRACKET)
			throw NIGHT_COMPILE_ERROR("expected opening bracket after function name", "function definitions must be in this format: 'def function(parameters) {}'", Learn::FUNCTIONS);
		if (current_scope->variables.contains(tokens[1].data))
			throw NIGHT_COMPILE_ERROR("function '" + tokens[1].data + "' can not have the same name as a variable", "function and variable names must be unique", Learn::FUNCTIONS);
		if (check_functions.contains(tokens[1].data))
			throw NIGHT_COMPILE_ERROR("function '" + tokens[1].data + "' has already been defined", "functions can only be defined once", Learn::FUNCTIONS);
		if (check_classes.contains(tokens[1].data))
			throw NIGHT_COMPILE_ERROR("function '" + tokens[1].data + "' can not have the same name as a class", "function and class names must be unique", Learn::FUNCTIONS);
		if (current_scope->upper_scope != nullptr)
			throw NIGHT_COMPILE_ERROR("function '" + tokens[1].data + "' can not be defined inside of a local scope", "functions must be defined in the global scope", Learn::FUNCTIONS);

		// parsing function parameters
		// also turns parameters into variables for definition checking the function body

		std::shared_ptr<Scope> function_scope = std::make_shared<Scope>(current_scope);

		std::vector<std::string> parameter_names;

		auto close_bracket_it = std::next(tokens.begin(), 3);
		for (; close_bracket_it != tokens.end(); ++close_bracket_it)
		{
			if (close_bracket_it->type == TokenType::CLOSE_BRACKET)
				break;

			if (close_bracket_it->type != TokenType::VARIABLE)
				throw NIGHT_COMPILE_ERROR("expected variable names as function parameters", "function parameters must be variables separated by commas", Learn::FUNCTIONS);
			if (check_functions.contains(close_bracket_it->data))
				throw NIGHT_COMPILE_ERROR("function parameter can not have the same name as a function", "function parameter names must be unique", Learn::FUNCTIONS);
			if (check_classes.contains(close_bracket_it->data))
				throw NIGHT_COMPILE_ERROR("function parameter can not have the same name as a class", "function parameter names must be unique", Learn::FUNCTIONS);

			parameter_names.push_back(close_bracket_it->data);
			function_scope->variables[close_bracket_it->data] = CheckVariable();

			if (std::next(close_bracket_it, 1)->type == TokenType::COMMA)
				++close_bracket_it;
		}

		if (close_bracket_it == tokens.end())
			throw NIGHT_COMPILE_ERROR("expected closing bracket after function parameter", "function definitions must be in this format: 'def function (parameters) {}'", Learn::FUNCTIONS);
		if (std::next(close_bracket_it, 1) == tokens.end())
			throw NIGHT_COMPILE_ERROR("expected function body", "function definitions must be in this format: 'def function (parameters) {}'", Learn::FUNCTIONS);

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

		const std::vector<std::vector<Token> > split_tokens =
			SplitCode(std::vector<Token>(std::next(close_bracket_it, 1), tokens.end()));

		for (const std::vector<Token>& split_token : split_tokens)
			Parser(function_scope, split_token);

		// set check function to be void or not, and
		// assign check function parameters' types

		check_function->second.is_void =
			check_function->second.return_types.empty();

		for (std::size_t a = 0; a < function_def->parameters.size(); ++a)
		{
			check_function->second.parameters[a] = function_scope->variables[function_def->parameters[a]].types.empty()
				? all_types
				: function_scope->variables[function_def->parameters[a]].types;
		}
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
			*/

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
	else if (tokens[0].type == TokenType::RETURN)
	{
		// a note on the function scope:
		/*
		// make sure the check function is pushed to the check functions array
		// before making the recursive call to extract the body
		//
		// if that doesn't happen then this won't work, since here it climbs up
		// scopes in order to find the function scope
		*/

		const Scope* function_scope = current_scope->upper_scope.get();
		while (function_scope->upper_scope != nullptr)
			function_scope = function_scope->upper_scope.get();

		if (function_scope->statements.back().type != StatementType::FUNCTION_DEF)
			throw NIGHT_COMPILE_ERROR("return statement can not be outside of a function", "return statements must be inside of functions", Learn::FUNCTIONS);

		// a note on void functions:
		/*
		// to determine if a check function is void or not, the presence of
		// return statements are analyzed
		//
		// if a function doesn't have a return statement, then it is void
		*/

		const FunctionDef& function_def =
			std::get<FunctionDef>(function_scope->statements.back().stmt);

		check_functions[function_def.name].is_void = false;

		// parsing return expression

		std::shared_ptr<Expression> return_expr(nullptr);
		if (tokens.size() > 1)
		{
			const std::vector<Value> values = TokensToValues(
				std::vector<Token>(tokens.begin() + 1, tokens.end()));

			return_expr = ValuesToExpression(values);

			const VariableTypeContainer return_expr_types =
				type_check_expr(current_scope, return_expr);

			// adding types to return types and pushing statement

			if (is_reachable(current_scope))
				check_functions[function_def.name].return_types.insert(return_expr_types);
		}
		
		current_scope->statements.push_back(Statement{
			loc, StatementType::RETURN, Return{ return_expr } });
	}
	
	else if (tokens[0].type == TokenType::WHILE)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::OPEN_BRACKET)
			throw NIGHT_COMPILE_ERROR("expected opening bracket after 'while' keyword", "while loop statements must be in this format: 'while (condition) {}'", Learn::LOOPS);

		// extracting and type checking condition

		auto close_bracket_it = std::next(tokens.begin(), 1);
		advance_to_close_bracket(tokens, close_bracket_it);

		if (close_bracket_it == tokens.end())
			throw NIGHT_COMPILE_ERROR("missing closing bracket for while loop condition", "while loop statements must be in this format: 'while (condition) {}'", Learn::LOOPS);

		const std::vector<Value> values = TokensToValues(
			std::vector<Token>(tokens.begin() + 2, close_bracket_it));

		const std::shared_ptr<Expression> condition_expr =
			ValuesToExpression(values);

		const VariableTypeContainer condition_types = type_check_expr(
			current_scope, condition_expr, { VariableType::BOOL });

		if (!condition_types.contains(VariableType::BOOL))
			throw NIGHT_COMPILE_ERROR("while loop condition must contain type: 'bool'", "condition currently contains " + get_var_types_as_str(condition_types), Learn::LOOPS);

		// extracting body

		const std::vector<std::vector<Token> > split_tokens = SplitCode(
			std::vector<Token>(std::next(close_bracket_it, 1), tokens.end()));

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
			throw NIGHT_COMPILE_ERROR("expected opening bracket after 'for' keyword", "for loops must be in this format: 'for (iterator : range) {}'", Learn::LOOPS);
		if (tokens.size() == 2 || tokens[2].type != TokenType::VARIABLE)
			throw NIGHT_COMPILE_ERROR("expected iterator after opening bracket", "for loops must be in this format: 'for (iterator : range) {}'", Learn::LOOPS);
		if (tokens.size() == 3 || tokens[3].type != TokenType::COLON)
			throw NIGHT_COMPILE_ERROR("expected colon after iterator", "for loops must be in this format: 'for (iterator : range) {}'", Learn::LOOPS);
		if (tokens.size() == 4)
			throw NIGHT_COMPILE_ERROR("expected range after colon", "for loops must be in this format: 'for (iterator : range) {}'", Learn::LOOPS);

		// evaluating iterator and range

		auto close_bracket_it = std::next(tokens.begin(), 1);
		advance_to_close_bracket(tokens, close_bracket_it);

		if (close_bracket_it == tokens.end())
			throw NIGHT_COMPILE_ERROR("missing closing bracket in for loop", "for loops must be in this format: 'for (condition) {}'", Learn::LOOPS);

		const std::vector<Value> range_values = TokensToValues(std::vector<Token>(tokens.begin() + 4, close_bracket_it));
		const std::shared_ptr<Expression> range_expr = ValuesToExpression(range_values);

		// also makes sure range is type array or string

		const VariableTypeContainer range_types =
			type_check_expr(current_scope, range_expr);

		if (!range_types.contains(VariableType::ARRAY) &&
			!range_types.contains(VariableType::STRING))
			throw NIGHT_COMPILE_ERROR("for loop range must contain type: 'arr'", "range currently contains " + get_var_types_as_str(range_types), Learn::LOOPS);

		// extracting scope and constructing statement

		const std::vector<std::vector<Token> > split_tokens = SplitCode(
			std::vector<Token>(std::next(close_bracket_it, 1), tokens.end()));

		std::shared_ptr<Scope> for_scope = std::make_shared<Scope>(current_scope);
		for_scope->variables[tokens[2].data] = CheckVariable(range_types);

		for (const std::vector<Token>& split_token : split_tokens)
			Parser(for_scope, split_token);

		current_scope->statements.push_back(Statement{
			loc, StatementType::FOR_LOOP,
			ForLoop{ tokens[2].data, range_expr, for_scope }
		});
	}
	
	else
	{
		throw NIGHT_COMPILE_ERROR("unknown syntax", "no clue what you did here sorry :/", Learn::LEARN);
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
		if ((tokens[a].type == TokenType::VARIABLE || tokens[a].type == TokenType::STRING) &&
			a < tokens.size() - 1 && tokens[a + 1].type == TokenType::OPEN_SQUARE)
		{
			if (tokens[a].type == TokenType::VARIABLE)
				values.push_back(Value{ ValueType::VARIABLE, tokens[a].data });
			else
				values.push_back(Value{ ValueType::STRING, tokens[a].data });

			while (a < tokens.size() - 1 && tokens[a + 1].type == TokenType::OPEN_SQUARE)
			{
				if (a == tokens.size() - 2)
					throw NIGHT_COMPILE_ERROR(CompileError::invalid_syntax, loc, "closing square bracket missing for subscript operator");

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
		else if (a < tokens.size() - 1 && tokens[a].type == TokenType::VARIABLE && tokens[a + 1].type == TokenType::OPEN_BRACKET)
		{
			Value value_call{ ValueType::CALL, tokens[a].data };

			a += 2;

			if (a >= tokens.size())
				throw NIGHT_COMPILE_ERROR(CompileError::invalid_syntax, loc, "missing closing bracket for function call '" + tokens[a - 2].data + "'");

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
				throw NIGHT_COMPILE_ERROR(CompileError::invalid_syntax, loc, "expecting closing bracket for function call '", "function call '" + tokens[a].data + "' does not have a closing bracket");

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
				throw NIGHT_COMPILE_ERROR("missing closing square bracket for array", "opening square bracket does not have an associated closing square bracket");

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
			case TokenType::VARIABLE:
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
				throw NIGHT_COMPILE_ERROR(CompileError::invalid_grammar, loc, "unexpected token '" + tokens[a].data + "' in expression", "expressions can only contain values, variables, arrays, functions, and operators");
			}
		}
	}

	// check expression

	for (auto it = values.begin(); it != values.end(); ++it)
	{
		if (it->type != ValueType::OPEN_BRACKET && it->type != ValueType::CLOSE_BRACKET &&
			it->type != ValueType::OPERATOR && std::next(it, 1)->type != ValueType::OPEN_BRACKET &&
			std::next(it, 1)->type != ValueType::CLOSE_BRACKET && std::next(it, 1)->type != ValueType::OPERATOR)
			throw NIGHT_COMPILE_ERROR("expected operator in between values", "expression must contains binary operators between two values", Learn::LEARN);
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
		throw NIGHT_COMPILE_ERROR("unary operators can not be adjacent to one another", "unary operators must be next to a value", Learn::LEARN);

	// increments index to next operator
	std::size_t start = index;
	for (int open_bracket_count = 0; index < values.size(); ++index)
	{
		if (values[index].type == ValueType::OPEN_BRACKET)
			open_bracket_count++;
		else if (values[index].type == ValueType::CLOSE_BRACKET)
			open_bracket_count--;

		if (open_bracket_count == 0 && values[index].type == ValueType::OPERATOR)
		{
			break;
		}
		else if (open_bracket_count == 0 && values[index].type == ValueType::CLOSE_BRACKET)
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
		if (operators[a].contains(value))
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
	const VariableTypeContainer& required_types
) {
	assert(node != nullptr);

	// if node->left and node->right is NULL, then node must be a value
	if (node->left == nullptr && node->right == nullptr)
	{
		switch (node->type)
		{
		case ValueType::VARIABLE: {
			auto* const check_variable = get_variable(current_scope, node->data);
			if (check_variable == nullptr)
				throw NIGHT_COMPILE_ERROR("variable '" + node->data + "' is undefined", "variables must be defined before they are used", Learn::VARIABLES);

			// if the variable is a function parameter
			if (check_variable->second.types.empty())
			{
				if (!is_reachable(current_scope))
					return required_types;

				check_variable->second.types = required_types;
			}

			return check_variable->second.types;
		}
		case ValueType::CALL: {
			auto check_function = check_functions.find(node->data);

			if (check_function == check_functions.end())
				throw NIGHT_COMPILE_ERROR("function '" + node->data + "' is undefined", "functions must be defined before they are called", Learn::FUNCTIONS);
			if (check_function->second.is_void)
				throw NIGHT_COMPILE_ERROR("function '" + check_function->first + "' does not have a return value", "functions must have a return value to be used in expression", Learn::FUNCTIONS);
			if (node->extras.size() != check_function->second.parameters.size())
				throw NIGHT_COMPILE_ERROR("function '" + check_function->first + "' can only be called with '" + std::to_string(check_function->second.parameters.size()) + "' argument(s)", "function '" + check_function->first + "' was called with '" + std::to_string(node->extras.size()) + "' argument(s)", Learn::FUNCTIONS);

			// type checking function parameters
			for (std::size_t a = 0; a < check_function->second.parameters.size(); ++a)
			{
				if (check_function->second.parameters[a].empty())
					continue;

				const VariableTypeContainer argument_types =
					type_check_expr(current_scope, node->extras[a]);

				for (const VariableType& argument_type : argument_types)
				{
					if (!check_function->second.parameters[a].contains(argument_type))
						throw NIGHT_COMPILE_ERROR("argument number '" + std::to_string(a + 1) + "' for function '" + check_function->first + "' can only be " + get_var_types_as_str(check_function->second.parameters[a]) + "'", "argument number '" + std::to_string(a + 1) + "' currently contains " + get_var_types_as_str(argument_types), Learn::TYPE_CHECKING);
				}
			}

			// a note about empty return types:
			/*
			// if a function's return types can't be deduced when the
			// function is defined, then the return type is treated as if it
			// were any type
			//
			// returning 'all_types' here wouldn't be wrong, but to keep
			// the types to a minimum (only containing necessary types),
			// 'required_types' is returned instead
			*/
			return check_function->second.return_types.empty()
				? required_types
				: check_function->second.return_types;
		}
		case ValueType::ARRAY: {
			VariableTypeContainer element_types;
			for (const std::shared_ptr<Expression>& element : node->extras)
			{
				const VariableTypeContainer element_type =
					type_check_expr(current_scope, element);

				element_types.insert(element_type.begin(), element_type.end());
			}

			return { VariableType(element_types) };
		}
		case ValueType::STRING: {
			return { VariableType::STRING };
		}
		case ValueType::FLOAT: {
			return { VariableType::FLOAT };
		}
		case ValueType::INT: {
			return { VariableType::INT };
		}
		case ValueType::BOOL: {
			return { VariableType::BOOL };
		}
		default: {
			assert(false);
		}
		}
	}

	assert(node->type == ValueType::OPERATOR);

	if (node->data == "-")
	{
		// unary operator

		if (node->left == nullptr)
		{
			const VariableTypeContainer right =
				type_check_expr(current_scope, node->right,
				{ VariableType::INT, VariableType::FLOAT });

			if (!find_num_types(right))
				throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used on types: 'int' or 'float'", "right hand value of operator '" + node->data + "' currently contains " + get_var_types_as_str(right), Learn::TYPE_CHECKING);

			return { VariableType::INT, VariableType::FLOAT };
		}

		// binary operator

		const VariableTypeContainer left =
			type_check_expr(current_scope, node->left,
			{ VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(left))
			throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used on types: 'int' or 'float'", "left hand value of operator '" + node->data + "' currently contains " + get_var_types_as_str(left), Learn::TYPE_CHECKING);

		const VariableTypeContainer right =
			type_check_expr(current_scope, node->right,
			{ VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(right))
			throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used on types: 'int' or 'float'", "right hand value of operator '" + node->data + "' currently contains " + get_var_types_as_str(right), Learn::TYPE_CHECKING);
		
		if ((!left.contains(VariableType::INT) && left.contains(VariableType::FLOAT)) ||
			(!right.contains(VariableType::INT) && right.contains(VariableType::FLOAT)))
			return { VariableType::FLOAT };
		else
			return { VariableType::INT };
	}
	if (node->data == "!")
	{
		const VariableTypeContainer right = type_check_expr(
			current_scope, node->right, { VariableType::BOOL });

		if (!find_num_types(right))
			throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used on type: 'bool'", "right hand value of operator '" + node->data + "' currently contains " + get_var_types_as_str(right), Learn::TYPE_CHECKING);

		return { VariableType::BOOL };
	}
	if (node->data == "[]")
	{
		const VariableTypeContainer left =
			type_check_expr(current_scope, node->right,
			{ VariableType::STRING, VariableType::ARRAY });

		if (!left.contains(VariableType::STRING) &&	!left.contains(VariableType::ARRAY))
			throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used on types: 'str' or 'arr'", "left hand value of operator '" + node->data + "' currently contains " + get_var_types_as_str(left), Learn::TYPE_CHECKING);

		const VariableTypeContainer right = type_check_expr(
			current_scope, node->extras[0], { VariableType::INT });

		if (!right.contains(VariableType::INT))
			throw NIGHT_COMPILE_ERROR("index for subscript operator can only have type 'int'", "index of operator '" + node->data + "' currently contains " + get_var_types_as_str(left), Learn::TYPE_CHECKING);
 
		VariableTypeContainer rtn_types;
		if (left.contains(VariableType::STRING))
			rtn_types.insert(VariableType::STRING);
		if (auto it = left.find(VariableType::ARRAY); it != left.end())
			rtn_types.insert(it->element_types);

		return rtn_types;
	}

	assert(node->left != nullptr && node->right != nullptr);

	if (node->data == "+")
	{
		const VariableTypeContainer left =
			type_check_expr(current_scope, node->left,
			{ VariableType::INT, VariableType::FLOAT, VariableType::STRING });

		const VariableTypeContainer right =
			type_check_expr(current_scope, node->right,
			{ VariableType::INT, VariableType::FLOAT, VariableType::STRING });

		VariableTypeContainer add_types;
		if (left.contains(VariableType::INT) && right.contains(VariableType::INT))
			add_types.insert(VariableType::INT);
		if ((left.contains(VariableType::FLOAT) && find_num_types(right)) ||
			(right.contains(VariableType::FLOAT) && find_num_types(right)))
			add_types.insert(VariableType::FLOAT);
		if (left.contains(VariableType::STRING) || right.contains(VariableType::STRING))
			add_types.insert(VariableType::STRING);

		if (add_types.empty())
			throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used on types: 'int' and 'float', or 'str'");

		return add_types;
	}
	if (node->data == "*")
	{
		const VariableTypeContainer left =
			type_check_expr(current_scope, node->left,
			{ VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(left))
			throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used on types: 'int' or 'float'", "left hand value of operator '" + node->data + "' currently contains " + get_var_types_as_str(left), Learn::TYPE_CHECKING);

		const VariableTypeContainer right =
			type_check_expr(current_scope, node->right,
			{ VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(right))
			throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used on types: 'int' or 'float'", "right hand value of operator '" + node->data + "' currently contains " + get_var_types_as_str(right), Learn::TYPE_CHECKING);

		if ((!left.contains(VariableType::INT) && left.contains(VariableType::FLOAT)) ||
			(!right.contains(VariableType::INT) && right.contains(VariableType::FLOAT)))
			return { VariableType::FLOAT };
		else
			return { VariableType::INT };
	}
	if (node->data == "/")
	{
		const VariableTypeContainer left =
			type_check_expr(current_scope, node->left,
			{ VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(left))
			throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used on types: 'int' or 'float'", "left hand value of operator '" + node->data + "' currently contains " + get_var_types_as_str(left), Learn::TYPE_CHECKING);

		const VariableTypeContainer right =
			type_check_expr(current_scope, node->right,
			{ VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(right))
			throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used on types: 'int' or 'float'", "right hand value of operator '" + node->data + "' currently contains " + get_var_types_as_str(right), Learn::TYPE_CHECKING);

		if ((!left.contains(VariableType::INT) && left.contains(VariableType::FLOAT)) ||
			(!right.contains(VariableType::INT) && right.contains(VariableType::FLOAT)))
			return { VariableType::FLOAT };
		else
			return { VariableType::INT };
	}
	if (node->data == "%")
	{
		const VariableTypeContainer left =
			type_check_expr( current_scope, node->left,
			{ VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(left))
			throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used on types: 'int' or 'float'", "left hand value of operator '" + node->data + "' currently contains " + get_var_types_as_str(left), Learn::TYPE_CHECKING);

		const VariableTypeContainer right =
			type_check_expr(current_scope, node->right,
			{ VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(right))
			throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used on types: 'int' or 'float'", "right hand value of operator '" + node->data + "' currently contains " + get_var_types_as_str(right), Learn::TYPE_CHECKING);

		if ((!left.contains(VariableType::INT) && left.contains(VariableType::FLOAT)) ||
			(!right.contains(VariableType::INT) && right.contains(VariableType::FLOAT)))
			return { VariableType::FLOAT };
		else
			return { VariableType::INT };
	}
	if (node->data == ">")
	{
		const VariableTypeContainer left =
			type_check_expr(current_scope, node->left,
			{ VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(left))
			throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used on types: 'int' or 'float'", "left hand value of operator '" + node->data + "' currently contains " + get_var_types_as_str(left), Learn::TYPE_CHECKING);

		const VariableTypeContainer right =
			type_check_expr(current_scope, node->right,
			{ VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(right))
			throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used on types: 'int' or 'float'", "right hand value of operator '" + node->data + "' currently contains " + get_var_types_as_str(right), Learn::TYPE_CHECKING);

		return { VariableType::BOOL };
	}
	if (node->data == "<")
	{
		const VariableTypeContainer left =
			type_check_expr(current_scope, node->left,
			{ VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(left))
			throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used on types: 'int' or 'float'", "left hand value of operator '" + node->data + "' currently contains " + get_var_types_as_str(left), Learn::TYPE_CHECKING);

		const VariableTypeContainer right =
			type_check_expr(current_scope, node->right,
			{ VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(right))
			throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used on types: 'int' or 'float'", "right hand value of operator '" + node->data + "' currently contains " + get_var_types_as_str(right), Learn::TYPE_CHECKING);

		return { VariableType::BOOL };
	}
	if (node->data == ">=")
	{
		const VariableTypeContainer left =
			type_check_expr(current_scope, node->left,
			{ VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(left))
			throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used on types: 'int' or 'float'", "left hand value of operator '" + node->data + "' currently contains " + get_var_types_as_str(left), Learn::TYPE_CHECKING);

		const VariableTypeContainer right =
			type_check_expr(current_scope, node->right,
			{ VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(right))
			throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used on types: 'int' or 'float'", "right hand value of operator '" + node->data + "' currently contains " + get_var_types_as_str(right), Learn::TYPE_CHECKING);

		return { VariableType::BOOL };
	}
	if (node->data == "<=")
	{
		const VariableTypeContainer left =
			type_check_expr(current_scope, node->left,
			{ VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(left))
			throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used on types: 'int' or 'float'", "left hand value of operator '" + node->data + "' currently contains " + get_var_types_as_str(left), Learn::TYPE_CHECKING);

		const VariableTypeContainer right =
			type_check_expr(current_scope, node->right,
			{ VariableType::INT, VariableType::FLOAT });

		if (!find_num_types(right))
			throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used on types: 'int' or 'float'", "right hand value of operator '" + node->data + "' currently contains " + get_var_types_as_str(right), Learn::TYPE_CHECKING);

		return { VariableType::BOOL };
	}
	if (node->data == "||")
	{
		const VariableTypeContainer left =
			type_check_expr(current_scope, node->left, { VariableType::BOOL });

		if (!left.contains(VariableType::BOOL))
			throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used on type: 'bool'", "left hand value of operator '" + node->data + "' currently contains " + get_var_types_as_str(left), Learn::TYPE_CHECKING);

		const VariableTypeContainer right =
			type_check_expr(current_scope, node->right, { VariableType::BOOL });

		if (!right.contains(VariableType::BOOL))
			throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used on type: 'bool'", "right hand value of operator '" + node->data + "' currently contains " + get_var_types_as_str(right), Learn::TYPE_CHECKING);

		return { VariableType::BOOL };
	}
	if (node->data == "&&")
	{
		const VariableTypeContainer left =
			type_check_expr(current_scope, node->left, { VariableType::BOOL });
		
		if (!left.contains(VariableType::BOOL))
			throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used on type: 'bool'", "left hand value of operator '" + node->data + "' currently contains " + get_var_types_as_str(left), Learn::TYPE_CHECKING);

		const VariableTypeContainer right =
			type_check_expr(current_scope, node->right, { VariableType::BOOL });

		if (!right.contains(VariableType::BOOL))
			throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used on type: 'bool'", "right hand value of operator '" + node->data + "' currently contains " + get_var_types_as_str(left), Learn::TYPE_CHECKING);

		return { VariableType::BOOL };
	}
	if (node->data == "==")
	{
		const VariableTypeContainer left =
			type_check_expr(current_scope, node->left, all_types);

		const VariableTypeContainer right =
			type_check_expr(current_scope, node->right, all_types);

		const bool match = std::any_of(left.begin(), left.end(),
			[&](const VariableType& type) { return right.contains(type); });
		
		if (!match)
			throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used to compare two equivalent non-class types", "left hand value of operator '" + node->data + "' currently contains " + get_var_types_as_str(left) + ", but right hand value currently contains " + get_var_types_as_str(right), Learn::TYPE_CHECKING);

		return { VariableType::BOOL };
	}
	if (node->data == "!=")
	{
		const VariableTypeContainer left =
			type_check_expr(current_scope, node->left, all_types);

		const VariableTypeContainer right =
			type_check_expr(current_scope, node->right, all_types);

		const bool match = std::any_of(left.begin(), left.end(),
			[&](const VariableType& type) { return right.contains(type); });

		if (!match)
			throw NIGHT_COMPILE_ERROR("operator '" + node->data + "' can only be used to compare two equivalent non-class types", "left hand value of operator '" + node->data + "' currently contains " + get_var_types_as_str(left) + ", but right hand value currently contains " + get_var_types_as_str(right), Learn::TYPE_CHECKING);

		return { VariableType::BOOL };
	}
	if (node->data == ".")
	{
		// find required types and return types based on method

		// a note about empty method types:
		/*
		// if a method's return types can't be deduced when the method is
		// defined, then the return type is treated as if it where any
		// type
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

	assert(false);
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

bool Parser::is_reachable(const std::shared_ptr<Scope>& scope) const
{
	return scope->upper_scope != nullptr &&
		scope->upper_scope->statements.back().type ==
		StatementType::FUNCTION_DEF;;
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