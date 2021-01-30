#include "../../include/back-end/parser.hpp"
#include "../../include/back-end/utils.hpp"
#include "../../include/back-end/token.hpp"
#include "../../include/error.hpp"

#include <memory>
#include <algorithm>
#include <string>
#include <vector>
#include <unordered_set>

/* methods */

Parser::Parser(Scope& current_scope, const std::vector<Token>& _tokens)
	: file(_tokens[0].file), line(_tokens[0].line), tokens(_tokens)
{
	if (tokens.size() >= 1 && tokens[0].type == TokenType::SET)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::VAR)
			throw BackError(file, line, "expected variable name after keyword 'set'");
		if (tokens.size() == 2 || tokens[2].type != TokenType::ASSIGN)
			throw BackError(file, line, "expected assignment symbol '=' after variable name");
		if (tokens.size() == 3)
			throw BackError(file, line, "expected expression after assignment symbol '='");

		if (night::find_variable(current_scope, tokens[1].data))
			throw BackError(file, line, "variable '" + tokens[1].data + "' is already defined");
		if (night::find_container(check_functions, tokens[1].data))
			throw BackError(file, line, "variable '" + tokens[1].data + "' has the same name as a function; variable and function names must be unique");
		if (night::find_container(check_classes, tokens[1].data))
			throw BackError(file, line, "variable '" + tokens[1].data + "' has the same name as a class; variable and class names must be unique");

		std::vector<VariableType> expression_types;
		const std::shared_ptr<Expression> expression = ParseTokenExpression(
			current_scope, 3, tokens.size(), expression_types
		);

		current_scope.check_variables.push_back(
			CheckVariable{ tokens[1].data, expression_types });

		current_scope.statements.push_back(Statement{
			file, line,
			StatementType::VARIABLE,
			Variable{ tokens[1].data, expression }
		});
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VAR && tokens[1].type == TokenType::ASSIGN)
	{
		if (tokens.size() == 2)
			throw BackError(file, line, "expected expression after assignment operator");

		CheckVariable* check_variable = night::get_variable(current_scope, tokens[0].data);
		if (check_variable == nullptr)
			throw BackError(file, line, "variable '" + tokens[0].data + "' is not defined");

		if (!check_variable->types.empty())
		{
			if (tokens[1].data == "+=" && !night::find_type(check_variable->types, VariableType::INT, VariableType::FLOAT) &&
				!night::find_type(check_variable->types, VariableType::STR))
				throw BackError(file, line, "variable '" + check_variable->name + "' does not contain type 'num' or 'str'; assignment '+=' requires variables to contain type 'num' or 'str'");
			if (tokens[1].data != "=" && tokens[1].data != "+=" && !night::find_type(check_variable->types, VariableType::INT, VariableType::FLOAT))
				throw BackError(file, line, "variable '" + check_variable->name + "' does not contain type 'num'; assignment '" + tokens[1].data + "' requires variables to contain type 'num'");
		}

		std::vector<VariableType> expression_types;
		const std::shared_ptr<Expression> expression = ParseTokenExpression(
			current_scope, 2, tokens.size(), expression_types
		);

		if (tokens[1].data != "=" && tokens[1].data != "+=" && !night::find_type(expression_types, VariableType::INT, VariableType::FLOAT) &&
			!night::find_type(expression_types, VariableType::STR))
			throw BackError(file, line, "expression does not contain type 'num' or 'str'; assignment '+=' requires expressions to contain type 'num' or 'str'");
		if (tokens[1].data != "=" && tokens[1].data == "+=" && !night::find_type(expression_types, VariableType::INT, VariableType::FLOAT))
			throw BackError(file, line, "expression does not contain type 'num'; assignment '" + tokens[1].data + "' requires expressions to contain type 'num'");

		check_variable->types.insert(check_variable->types.end(),
			expression_types.begin(), expression_types.end());

		current_scope.statements.push_back(Statement{
			file, line,
			StatementType::ASSIGNMENT,
			Assignment{ tokens[1].data[0], tokens[0].data, expression },
		});
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::IF)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::OPEN_BRACKET)
			throw BackError(file, line, "expected open bracket after 'if' keyword");
		if (tokens.size() == 2)
			throw BackError(file, line, "expected close bracket in if condition");
		if (tokens.size() == 3 && tokens[2].type == TokenType::CLOSE_BRACKET)
			throw BackError(file, line, "expected expression in between brackets");

		std::size_t close_bracket_index = 2;

		const std::shared_ptr<Expression> condition = ExtractCondition(current_scope, close_bracket_index, "if condition");
		const Scope body = ExtractBody(current_scope, close_bracket_index, "if statement");

		current_scope.statements.push_back(Statement{
			file, line,
			StatementType::IF_STATEMENT,
			IfStatement{ { Conditional{ condition, body } } }
		});
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::ELSE && tokens[1].type == TokenType::IF)
	{
		if (tokens.size() == 2 || tokens[2].type != TokenType::OPEN_BRACKET)
			throw BackError(file, line, "expected open bracket after 'if' keyword");
		if (tokens.size() == 3)
			throw BackError(file, line, "expected close bracket in if condition");
		if (tokens.size() == 4 && tokens[3].type == TokenType::CLOSE_BRACKET)
			throw BackError(file, line, "expected expression in between brackets");
		if (current_scope.statements.size() == 0 || current_scope.statements.back().type != StatementType::IF_STATEMENT ||
			std::get<IfStatement>(current_scope.statements.back().stmt).chains.back().is_else())
			throw BackError(file, line, "else if statements are required to come after an if or else if statement");

		std::size_t closeBracketIndex = 3;

		const std::shared_ptr<Expression> condition = ExtractCondition(current_scope, closeBracketIndex, "else if condition");
		const Scope body = ExtractBody(current_scope, closeBracketIndex, "else if statement");

		std::get<IfStatement>(current_scope.statements.back().stmt).chains.push_back(
			Conditional{ condition, body }
		);
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::ELSE)
	{
		if (current_scope.statements.size() == 0 || current_scope.statements.back().type != StatementType::IF_STATEMENT ||
			std::get<IfStatement>(current_scope.statements.back().stmt).chains.back().is_else())
			throw BackError(file, line, "else statement is required to come after an if or else if statement");

		std::get<IfStatement>(current_scope.statements.back().stmt).chains.push_back(
			Conditional{ nullptr, ExtractBody(current_scope, 0, "else statement") }
		);
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::DEF)
	{
		if (in_function)
			throw BackError(file, line, "function definition found within function definition; functions cannot contain functions");

		if (tokens.size() == 1 || tokens[1].type != TokenType::VAR)
			throw BackError(file, line, "expected function name after 'def' keyword");
		if (tokens.size() == 2 || tokens[2].type != TokenType::OPEN_BRACKET)
			throw BackError(file, line, "expected open bracket after function name");
		if (tokens.size() == 3)
			throw BackError(file, line, "expected open bracket after function name");
		if (tokens.size() == 4 && tokens[3].type != TokenType::CLOSE_BRACKET)
			throw BackError(file, line, "expected closing bracket for function parameters");
		if (tokens.size() == 4 || (tokens.size() == 5 && tokens[4].type != TokenType::OPEN_CURLY))
			throw BackError(file, line, "expected open curly bracket for function body");

		if (night::find_variable(current_scope, tokens[1].data))
			throw BackError(file, line, "function '" + tokens[1].data + "' has the same name as a variable; variable and function names are required to be unique");
		if (night::find_container(check_functions, tokens[1].data))
			throw BackError(file, line, "function '" + tokens[1].data + "' is already defined");
		if (night::find_container(check_classes, tokens[1].data))
			throw BackError(file, line, "function '" + tokens[1].data + "' has the same name as a class; function and class names are required to be unique");

		// function parameters

		Scope function_scope{ current_scope };

		std::vector<std::string> parameter_names;

		std::size_t closeBracketIndex = 3;
		for (; closeBracketIndex < tokens.size() && tokens[closeBracketIndex].type != TokenType::CLOSE_BRACKET; closeBracketIndex += 2)
		{
			if (tokens[closeBracketIndex].type != TokenType::VAR)
				throw BackError(file, line, "expected variable names as function parameters");

			if (night::find_variable(function_scope, tokens[closeBracketIndex].data))
				throw BackError(file, line, "function parameter cannot have the same name as another variable");
			if (night::find_container(check_functions, tokens[closeBracketIndex].data))
				throw BackError(file, line, "function parameter cannot have the same name as a function");
			if (night::find_container(check_classes, tokens[closeBracketIndex].data))
				throw BackError(file, line, "function parameter cannot have the same name as a class");

			parameter_names.push_back(tokens[closeBracketIndex].data);
			function_scope.check_variables.push_back({ tokens[closeBracketIndex].data });

			if (tokens[closeBracketIndex + 1].type == TokenType::CLOSE_BRACKET)
			{
				closeBracketIndex++;
				break;
			}

			if (tokens[closeBracketIndex + 1].type != TokenType::COMMA)
				throw BackError(file, line, "expected comma or closing bracket after function parameter");
		}

		if (tokens[closeBracketIndex].type != TokenType::CLOSE_BRACKET)
			throw BackError(file, line, "expected closing bracket after function parameters");
		if (tokens[closeBracketIndex + 1].type != TokenType::OPEN_CURLY)
			throw BackError(file, line, "expected opening curly after function parameters");

		// function body

		// define function before extracting body to ensure function is defined for recursion
		check_functions.push_back({ tokens[1].data });
		CheckFunction* check_function = &check_functions.back();
		check_function->parameters.resize(parameter_names.size());

		in_function = true;

		assert(return_types.empty());

		const FunctionDef function_def{
			tokens[1].data,
			parameter_names,
			ExtractBody(function_scope, closeBracketIndex, "function definitions"),
			return_types
		};

		in_function = false;
		return_types.clear();

		// check function

		// create parameters for check function
		// function_def just needs names, check_function needs types
		for (std::size_t a = 0; a < function_def.parameters.size(); ++a)
		{
			check_function->parameters[a] = function_scope.check_variables[a].types.empty()
				? all_types
				: function_scope.check_variables[a].types;
		}

		// return_types is the same for check_function and function_def
		check_function->return_types = function_def.return_types;

		// pushing statement

		current_scope.statements.push_back(Statement{
			file, line,
			StatementType::FUNCTION_DEF,
			function_def
		});
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VAR && tokens[1].type == TokenType::OPEN_BRACKET)
	{
		if (tokens.size() == 2 || (tokens.size() == 3 && tokens[2].type != TokenType::CLOSE_BRACKET))
			throw BackError(file, line, "missing closing bracket");

		const CheckFunction* check_function = night::get_container(check_functions, tokens[0].data);
		if (check_function == nullptr)
			throw BackError(file, line, "function " + tokens[0].data + "' is not defined");

		// function parameters

		std::vector<std::vector<VariableType> >   argument_types;
		std::vector<std::shared_ptr<Expression> > argument_expressions;

		if (tokens[2].type != TokenType::CLOSE_BRACKET)
		{
			for (std::size_t start = 2, a = 2, openBracketIndex = 0; a < tokens.size(); ++a)
			{
				if (tokens[a].type == TokenType::OPEN_BRACKET)
					openBracketIndex++;
				else if (tokens[a].type == TokenType::CLOSE_BRACKET)
					openBracketIndex--;

				if ((tokens[a].type == TokenType::COMMA && openBracketIndex == 0) ||
					(tokens[a].type == TokenType::CLOSE_BRACKET && openBracketIndex == -1))
				{
					if (tokens[a].type == TokenType::CLOSE_BRACKET && a < tokens.size() - 1)
						throw BackError(file, line, "unexpected tokens after function call; each statement must be on it's own line");

					std::vector<VariableType> get_argument_types;

					argument_expressions.push_back(
						ParseTokenExpression(current_scope, start, a, get_argument_types)
					);

					argument_types.push_back(get_argument_types);

					start = a + 1;
					continue;
				}
			}
		}

		// error checking arguments with function parameters

		if (argument_types.size() != check_function->parameters.size())
			throw BackError(file, line, "function '" + tokens[0].data + "' is called with '" + std::to_string(argument_types.size()) + "' arguments but is defined with '" + std::to_string(check_function->parameters.size()) + "' parameters");

		for (std::size_t a = 0; a < check_function->parameters.size(); ++a)
		{
			for (const VariableType& parameterType : argument_types[a])
			{
				if (!night::find_type(check_function->parameters[a], parameterType))
					throw BackError(file, line, "argument number '" + std::to_string(a + 1) + "' for function call '" + tokens[0].data + "' cannot be used because of a type mismatch");
			}
		}

		// pushing statement

		current_scope.statements.push_back(Statement{
			file, line,
			StatementType::FUNCTION_CALL,
			FunctionCall{ tokens[0].data, argument_expressions }
		});
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::RETURN)
	{
		if (!in_function)
			throw BackError(file, line, "return statement is outside of a function definition; return statements can only be inside functions");
		if (tokens.size() == 1)
			throw BackError(file, line, "expected expression after 'return' keyword");

		std::vector<VariableType> expr_types;
		const std::shared_ptr<Expression> expression = ParseTokenExpression(current_scope, 1, tokens.size(), expr_types);

		return_types.insert(return_types.end(), expr_types.begin(), expr_types.end());

		current_scope.statements.push_back(Statement{
			file, line,
			StatementType::RETURN,
			Return{ expression }
		});
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::WHILE)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::OPEN_BRACKET)
			throw BackError(file, line, "expected open bracket after 'while' keyword");
		if (tokens.size() == 2)
			throw BackError(file, line, "expected condition in while loop");
		if (tokens.size() == 3 && tokens[2].type != TokenType::CLOSE_BRACKET)
			throw BackError(file, line, "expected closing bracket in condition");
		if (tokens.size() == 3 && tokens[2].type == TokenType::CLOSE_BRACKET)
			throw BackError(file, line, "expected condition in between brackets");

		std::size_t closeBracketIndex = 2;

		const std::shared_ptr<Expression> condition = ExtractCondition(current_scope, closeBracketIndex, "while loop");
		const Scope body = ExtractBody(current_scope, closeBracketIndex, "while loop");

		current_scope.statements.push_back(Statement{
			file, line,
			StatementType::WHILE_LOOP,
			WhileLoop{ condition, body }
		});
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::FOR)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::OPEN_BRACKET)
			throw BackError(file, line, "expected open bracket after 'for' keyword");
		if (tokens.size() == 2 || tokens[2].type != TokenType::VAR)
			throw BackError(file, line, "expected iterator name after open bracket");
		if (tokens.size() == 3 || tokens[3].type != TokenType::COLON)
			throw BackError(file, line, "expected colon after iterator name");
		if (tokens.size() == 4)
			throw BackError(file, line, "expected array after colon");

		if (night::find_variable(current_scope, tokens[2].data))
			throw BackError(file, line, "variable '" + tokens[2].data + "' is already defined");

		// evaluating iterator and range

		std::size_t closeBracketIndex = 4;
		AdvanceToCloseBracket(tokens, TokenType::OPEN_BRACKET, TokenType::CLOSE_BRACKET, closeBracketIndex);

		if (closeBracketIndex >= tokens.size())
			throw BackError(file, line, "closing bracket is missing in for loop condition");

		const std::vector<Value> range_values = TokensToValues(std::vector<Token>(tokens.begin() + 4, tokens.begin() + closeBracketIndex));
		const std::shared_ptr<Expression> range_expr = ValuesToExpression(range_values);

		const std::vector<VariableType> range_types = TypeCheckExpression(current_scope, range_expr, "", {}, true);

		current_scope.check_variables.push_back(CheckVariable{ tokens[2].data, range_types });

		// extracting scope and constructing statement

		const Scope body = ExtractBody(current_scope, closeBracketIndex, "for loop");

		current_scope.check_variables.pop_back(); // remove the iterator variable

		current_scope.statements.push_back(Statement{
			file, line,
			StatementType::FOR_LOOP,
			ForLoop{ tokens[2].data, range_expr, body }
		});
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VAR && tokens[1].type == TokenType::OPEN_SQUARE)
	{
		if (tokens.size() == 2)
			throw BackError(file, line, "expected index after open square");

		CheckVariable* check_variable = night::get_variable(current_scope, tokens[0].data);
		if (check_variable == nullptr)
			throw BackError(file, line, "variable '" + tokens[1].data + "' is not defined");

		// check if variable is an array or a string

		const bool is_array = night::find_type(check_variable->types, VariableType::ARRAY);

		if (!is_array && night::find_type(check_variable->types, VariableType::STR))
			throw BackError(file, line, "variable '" + check_variable->name + "' doesn't contain a string or an array; to be access with an index, a variable must contain a string or an array");

		// extract and evaluate element

		std::size_t assignmentIndex = 2;
		for (; assignmentIndex < tokens.size() && tokens[assignmentIndex].type != TokenType::ASSIGN; ++assignmentIndex);

		if (tokens[assignmentIndex - 1].type != TokenType::CLOSE_SQUARE)
			throw BackError(file, line, "missing closing square");
		if (assignmentIndex == tokens.size())
			throw BackError(file, line, "missing assignment operator");

		std::vector<VariableType> elemType;
		const std::shared_ptr<Expression> elemExpr = ParseTokenExpression(
			current_scope, 2, assignmentIndex - 1, elemType
		);

		if (!night::find_type(elemType, VariableType::INT, VariableType::FLOAT))
			throw BackError(file, line, "index for array '" + tokens[0].data + "' is required to be of type 'int'");

		// extract expression

		std::vector<VariableType> tips;
		const std::shared_ptr<Expression> assignExpr = ParseTokenExpression(
			current_scope, assignmentIndex + 1, tokens.size(), tips
		);

		check_variable->types.insert(check_variable->types.end(), tips.begin(), tips.end());

		if (!is_array && (assignExpr->type != ValueType::STR || assignExpr->data.length() >= 1))
			throw BackError(file, line, "a string element can only be assigned to a string of length 1");

		// pushing statement

		current_scope.statements.push_back(Statement{
			file, line,
			StatementType::ELEMENT,
			Element{ tokens[0].data, elemExpr, assignExpr }
		});
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VAR && tokens[1].type == TokenType::OPERATOR)
	{
		if (tokens[1].data != ".")
			throw BackError(file, line, "invalid syntax");
		if (tokens.size() == 2)
			throw BackError(file, line, "expected method name after dot operator");

		std::vector<VariableType> temp;
		const std::shared_ptr<Expression> object = ParseTokenExpression(current_scope, 0, tokens.size(), temp);

		current_scope.statements.push_back(Statement{
			file, line,
			StatementType::METHOD_CALL,
			MethodCall{ tokens[0].data, object }
		});
	}
	else
	{
		throw BackError(file, line, "invalid syntax");
	}
}

std::vector<Value> Parser::TokensToValues(const std::vector<Token>& tokens)
{
	assert(!tokens.empty() && "tokens shouldn't be empty");

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
				const std::size_t start = a;

				AdvanceToCloseBracket(tokens, TokenType::OPEN_SQUARE, TokenType::CLOSE_SQUARE, a);
				if (a >= tokens.size())
					throw BackError(file, line, "closing square bracket missing for subscript operator");

				values.push_back({ ValueType::OPERATOR, "[]" });
				values.back().extras.push_back(TokensToValues(
					std::vector<Token>(std::begin(tokens) + start, std::begin(tokens) + a)
				));
			}
		}
		else if (tokens[a].type == TokenType::VAR)
		{
			if (a < tokens.size() - 1 && tokens[a + 1].type == TokenType::OPEN_BRACKET)
			{
				Value value_call{ ValueType::CALL, tokens[a].data };

				a += 2;

				if (tokens[a].type == TokenType::CLOSE_BRACKET)
				{
					values.push_back(value_call);
					continue;
				}

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

					if (open_square_count == 0 && ((tokens[a].type == TokenType::COMMA && open_bracket_count == 0) ||
						(tokens[a].type == TokenType::CLOSE_BRACKET && open_bracket_count == -1)))
					{
						value_call.extras.push_back(
							TokensToValues(std::vector<Token>(std::begin(tokens) + start, std::begin(tokens) + a))
						);

						start = a + 1;

						if (tokens[a].type == TokenType::CLOSE_BRACKET)
							break;

						continue;
					}
				}

				if (a >= tokens.size())
					throw BackError(file, line, "missing closing bracket for function call");

				values.push_back(value_call);
			}
			else
			{
				values.push_back(Value{ ValueType::VARIABLE, tokens[a].data });
			}
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
						std::vector<Token>(std::begin(tokens) + start, std::begin(tokens) + a)
					);

					value_array.extras.push_back(temp_values);

					start = a + 1;

					if (tokens[a].type == TokenType::CLOSE_SQUARE)
						break;

					continue;
				}
			}

			if (a >= tokens.size())
				throw BackError(file, line, "missing closing square bracket for array");

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
				throw BackError(file, line, "unexpected token '" + tokens[a].data + "' in expression");
			}
		}
	}

	for (std::size_t a = 0; a < values.size() - 1; ++a)
	{
		if (values[a].type != ValueType::OPEN_BRACKET && values[a].type != ValueType::CLOSE_BRACKET &&
			values[a].type != ValueType::OPERATOR && values[a + 1].type != ValueType::OPEN_BRACKET &&
			values[a + 1].type != ValueType::CLOSE_BRACKET && values[a + 1].type != ValueType::OPERATOR)
			throw BackError(file, line, "expected operator in between values");
	}

	return values;
}

std::shared_ptr<Expression> Parser::new_expression(const Value& value, const std::shared_ptr<Expression>& left,
	const std::shared_ptr<Expression>& right)
{
	std::shared_ptr<Expression> expression = std::make_shared<Expression>(Expression{
		file, line,
		value.type,
		value.data,
		{},
		left,
		right
	});

	for (const std::vector<Value>& values : value.extras)
		expression->extras.push_back(ValuesToExpression(values));

	return expression;
}

std::shared_ptr<Expression> Parser::GetNextGroup(const std::vector<Value>& values, std::size_t& index)
{
	const bool isFrontUnary = values[index].type == ValueType::OPERATOR && (values[index].data == "!" || values[index].data == "-");
	if (isFrontUnary)
		index++;

	if (values[index].type == ValueType::OPERATOR && (values[index].data == "!" || values[index].data == "-"))
		throw BackError(file, line, "unary operators cannot be adjacent to one another");

	// increments index to next operator
	std::size_t start = index;
	for (int open_bracket_count = 0; index < values.size(); ++index)
	{
		if (values[index].type == ValueType::OPEN_BRACKET)
			open_bracket_count++;
		else if (values[index].type == ValueType::CLOSE_BRACKET)
			open_bracket_count--;
		
		if (open_bracket_count != 0)
			continue;

		if (values[index].type == ValueType::OPERATOR)
			break;

		if (values[index].type == ValueType::CLOSE_BRACKET || values[index].type != ValueType::OPERATOR)
		{
			index++;
			break;
		}
	}

	// evaluate brackets
	std::shared_ptr<Expression> group_expression = values[start].type == ValueType::OPEN_BRACKET
		? ValuesToExpression(std::vector<Value>(std::begin(values) + start + 1, std::begin(values) + index))
		: new_expression(values[start], nullptr, nullptr);

	// evaluate subscript operator
	while (index < values.size() && values[index].type == ValueType::OPERATOR && values[index].data == "[]")
		group_expression = new_expression(values[index++], nullptr, group_expression);

	// evaluate unary operator
	return isFrontUnary
		? new_expression(values[start - 1], nullptr, group_expression)
		: group_expression;
}

int Parser::GetOperatorPrecedence(const ValueType& type, const std::string& value)
{
	static const std::vector<std::unordered_set<std::string> > operators{
		{ "[]", "." },
		{ "!" },
		{ "*", "/", "%" },
		{ "+", "-" },
		{ ">", "<", ">=", "<=" },
		{ "==", "!=" },
		{ "||", "&&" }
	};

	assert(type == ValueType::OPERATOR && "value must be operator to have operator precedence smh");

	for (std::size_t a = 0; a < operators.size(); ++a)
	{
		if (operators[a].find(value) != std::end(operators[a]))
			return a;
	}

	assert(false && "operator missing");
	return 0;
}

std::shared_ptr<Expression> Parser::ValuesToExpression(const std::vector<Value>& values)
{
	assert(!values.empty() && "values shouldn't be empty");

	std::size_t a = 0;

	std::shared_ptr<Expression> root = GetNextGroup(values, a);
	const Expression* protect = root.get();

	// parse expression

	while (a < values.size() - 1)
	{
		if (values[a].type != ValueType::OPERATOR)
			throw BackError(file, line, "missing operator between values in expression");

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

		const std::size_t opIndex = a++;

		const std::shared_ptr<Expression> nextValue = GetNextGroup(values, a);
		const std::shared_ptr<Expression> opNode = new_expression(values[opIndex], curr, nextValue);

		protect = nextValue.get();

		if (curr == root)
			root = opNode;
		else
			prev->right = opNode;
	}

	return root;
}

std::vector<VariableType> Parser::TypeCheckExpression(Scope& current_scope, const std::shared_ptr<Expression>& node,
	const std::string& op_name, const std::vector<VariableType>& required_types, bool is_for_loop_range)
{
	assert(node != nullptr && "node cannot be NULL");

	// if node->left and node->right is NULL, then node must be a value
	if (node->left == nullptr && node->right == nullptr)
	{
		if (node->type == ValueType::ARRAY)
		{
			if (!is_for_loop_range)
				return std::vector<VariableType>{ VariableType::ARRAY };

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
			CheckVariable* check_variable = night::get_variable(current_scope, node->data);
			if (check_variable == nullptr)
			{
				const CheckClass* check_object = night::get_container(check_classes, node->data);
				if (check_object == nullptr)
					throw BackError(file, line, "variable '" + node->data + "' is undefined");
				
				return { { VariableType::CLASS, check_object->name } };
			}

			if (is_for_loop_range)
			{
				if (!night::find_type(check_variable->types, VariableType::STR) &&
					!night::find_type(check_variable->types, VariableType::ARRAY))
					throw BackError(file, line, "range must be of type array");

				// refactor:
				//
				// should return each individual element types, but that can't be
				// deduced from check_variable, so returning all_types for now
				//
				return all_types;
			}

			if (check_variable->types.empty())
				check_variable->types = required_types;

			return check_variable->types;
		}
		if (node->type == ValueType::CALL)
		{
			CheckFunction* check_function = night::get_container(check_functions, node->data);
			if (check_function == nullptr)
				throw BackError(file, line, "function '" + node->data + "' is not defined");

			if (check_function->is_void)
				throw BackError(file, line, "function '" + node->data + "' doesn't return a value; functions used in expressions must return a value");
			if (node->extras.size() != check_function->parameters.size())
				throw BackError(file, line, "function '" + node->data + "' was called with '" + std::to_string(node->extras.size()) + "' arguments, but was defined with '" + std::to_string(check_function->parameters.size()) + "' parameters");

			// type checking function parameters
			for (std::size_t a = 0; a < check_function->parameters.size(); ++a)
			{
				const std::vector<VariableType> argumentTypes = TypeCheckExpression(current_scope, node->extras[a], "", {});

				if (check_function->parameters[a].empty())
				{
					check_function->parameters[a] = argumentTypes;
					continue;
				}

				for (const VariableType& argumentType : argumentTypes)
				{
					if (!night::find_type(check_function->parameters[a], argumentType))
						throw BackError(file, line, "argument number '" + std::to_string(a + 1) + "' for function call '" + check_function->name + "' cannot be used because of a type mismatch");
				}
			}

			// type checking return values
			if (check_function->return_types.empty())
			{
				check_function->return_types = required_types;
			}
			else
			{
				bool found_type = false;
				for (std::size_t a = 0; a < required_types.size() && !found_type; ++a)
				{
					if (night::find_type(check_function->return_types, required_types[a]))
						found_type = true;
				}

				if (!found_type)
					throw BackError(file, line, "function '" + node->data + "' can not be used with operator '" + op_name + "'");
			}

			return check_function->return_types;
		}

		switch (node->type)
		{
		case ValueType::BOOL:
			return { VariableType::BOOL };
		case ValueType::NUM:
			return { VariableType::NUM };
		case ValueType::STR:
			return { VariableType::STR };
		default:
			throw BackError(file, line, "unexpected token '" + node->data + "' in expression");
		}
	}

	assert(node->type == ValueType::OPERATOR);

	if (node->data == "+")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(current_scope, node->left,  node->data, { VariableType::NUM, VariableType::STR });
		const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, { VariableType::NUM, VariableType::STR });

		std::vector<VariableType> add_types;
		if (night::find_type(left, VariableType::NUM) && night::find_type(right, VariableType::NUM))
			add_types.push_back(VariableType::NUM);
		if (night::find_type(left, VariableType::STR) || night::find_type(right, VariableType::STR))
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
			const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, { VariableType::NUM });

			if (!night::find_type(right, VariableType::NUM))
				throw BackError(file, line, "unary operator '" + node->data + "' requires to be used on a type of 'num'");

			return { VariableType::NUM };
		}

		// binary operator (minus)

		const std::vector<VariableType> left  = TypeCheckExpression(current_scope, node->left,  node->data, { VariableType::NUM });
		const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, { VariableType::NUM });

		if (!night::find_type(left, VariableType::NUM) || !night::find_type(right, VariableType::NUM))
			throw BackError(file, line, "binary operator '" + node->data + "' can only be used on two numbers");
		
		return { VariableType::NUM };
	}
	if (node->data == "*")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(current_scope, node->left,  node->data, { VariableType::NUM });
		const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, { VariableType::NUM });

		if (!night::find_type(left, VariableType::NUM) || !night::find_type(right, VariableType::NUM))
			throw BackError(file, line, "binary operator '" + node->data + "' can only be used on two numbers");
		
		return { VariableType::NUM };
	}
	if (node->data == "/")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(current_scope, node->left,  node->data, { VariableType::NUM });
		const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, { VariableType::NUM });

		if (!night::find_type(left, VariableType::NUM) || !night::find_type(right, VariableType::NUM))
			throw BackError(file, line, "binary operator '" + node->data + "' can only be used on two numbers");
		
		return { VariableType::NUM };
	}
	if (node->data == "%")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(current_scope, node->left,  node->data, { VariableType::NUM });
		const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, { VariableType::NUM });

		if (!night::find_type(left, VariableType::NUM) || !night::find_type(right, VariableType::NUM))
			throw BackError(file, line, "binary operator '" + node->data + "' can only be used on two numbers");

		return { VariableType::NUM };
	}
	if (node->data == ">")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(current_scope, node->left, node->data, { VariableType::NUM });
		const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, { VariableType::NUM });

		if (!night::find_type(left, VariableType::NUM) || !night::find_type(right, VariableType::NUM))
			throw BackError(file, line, "binary operator '" + node->data + "' can only be used on two values of type 'num'");

		return { VariableType::BOOL };
	}
	if (node->data == "<")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(current_scope, node->left,  node->data, { VariableType::NUM });
		const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, { VariableType::NUM });

		if (!night::find_type(left, VariableType::NUM) || !night::find_type(right, VariableType::NUM))
			throw BackError(file, line, "binary operator '" + node->data + "' can only be used on two values of type 'num'");

		return { VariableType::BOOL };
	}
	if (node->data == ">=")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(current_scope, node->left,  node->data, { VariableType::NUM });
		const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, { VariableType::NUM });

		if (!night::find_type(left, VariableType::NUM) || !night::find_type(right, VariableType::NUM))
			throw BackError(file, line, "binary operator '" + node->data + "' can only be used on two values of type 'bool'");

		return { VariableType::BOOL };
	}
	if (node->data == "<=")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(current_scope, node->left,  node->data, { VariableType::NUM });
		const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, { VariableType::NUM });

		if (!night::find_type(left, VariableType::NUM) || !night::find_type(right, VariableType::NUM))
			throw BackError(file, line, "binary operator '" + node->data + "' can only be used on two values of type 'bool'");

		return { VariableType::BOOL };
	}
	if (node->data == "!")
	{
		const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, { VariableType::BOOL });

		if (!night::find_type(right, VariableType::BOOL))
			throw BackError(file, line, "unary operator '" + node->data + "' can only be used on a value of type 'bool'");

		return { VariableType::BOOL };
	}
	if (node->data == "||")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(current_scope, node->left,  node->data, { VariableType::BOOL });
		const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, { VariableType::BOOL });

		if (!night::find_type(left, VariableType::BOOL) || !night::find_type(right, VariableType::BOOL))
			throw BackError(file, line, "binary operator '" + node->data + "' can only be used on two values of type 'bool'");

		return { VariableType::BOOL };
	}
	if (node->data == "&&")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(current_scope, node->left,  node->data, { VariableType::BOOL });
		const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, { VariableType::BOOL });

		if (!night::find_type(left, VariableType::BOOL) || !night::find_type(right, VariableType::BOOL))
			throw BackError(file, line, "binary operator '" + node->data + "' can only be used on two values of type 'bool'");

		return { VariableType::BOOL };
	}
	if (node->data == "==")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(current_scope, node->left,  node->data, all_types);
		const std::vector<VariableType> right = TypeCheckExpression(current_scope, node->right, node->data, all_types);

		for (const VariableType& type : left)
		{
			if (!night::find_type(right, type))
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
			if (!night::find_type(right, type))
				throw BackError(file, line, "binary operator '" + node->data + "' requires two values of the same type");
		}

		return { VariableType::BOOL };
	}
	if (node->data == "[]")
	{
		// get individual array elements
		bool giae = false;
		const std::vector<VariableType> array_types = TypeCheckExpression(current_scope, node->right,     node->data, { VariableType::ARRAY }, &giae);
		const std::vector<VariableType> index_types = TypeCheckExpression(current_scope, node->extras[0], node->data, { VariableType::NUM });

		if (!giae)
			throw BackError(file, line, "operator '" + node->data + "' requires to be used on a string or array");
		if (!night::find_type(index_types, VariableType::NUM))
			throw BackError(file, line, "operator '" + node->data + "' requires subscript to be type 'num'");
 
		return array_types;
	}
	if (node->data == ".")
	{
		assert(node->right->left == nullptr && node->right->right == nullptr && "method can't be part of an expression");

		// the required_types argument is faulty; check to see what methods are present, then send them through to the function
		// so if object is a string, then only send VariableType::STR through
		const std::vector<VariableType> object_types = TypeCheckExpression(
			current_scope, node->left, node->data,
			{ VariableType::STR, VariableType::ARRAY, VariableType::CLASS }
		);

		if (!night::find_type(object_types, VariableType::ARRAY) && !night::find_type(object_types, VariableType::STR) &&
			!night::find_type(object_types, VariableType::CLASS))
			throw BackError(file, line, "variable '" + node->left->data + "' is used with methods; only objects can be used with methods");

		// find return type of method, and convert arr and str to classes
		std::vector<CheckClass*> check_objects;
		for (const VariableType& object_type : object_types)
		{
			if (object_type == VariableType::ARRAY)
				check_objects.push_back(&check_classes[0]);
			else if (object_type == VariableType::STR)
				check_objects.push_back(&check_classes[1]);
			else if (object_type == VariableType::CLASS)
				check_objects.push_back(night::get_container(check_classes, object_type.class_name));
		}

		// test:
		//
		// is this good logic?
		// i too confusion
		// confuzzled squid
		//
		//
		std::vector<VariableType> rtn_types;
		for (const CheckClass* check_object : check_objects)
		{
			for (const CheckFunction& method : check_object->methods)
			{
				if (method.name == node->right->data)
				{
					rtn_types.insert(rtn_types.end(), method.return_types.begin(), method.return_types.end());
					break;
				}
			}
		}

		return rtn_types;
	}

	assert(false && "operator missing from list");
	return {};
}

std::shared_ptr<Expression> Parser::ParseTokenExpression(Scope& current_scope, const std::size_t start,
	const std::size_t end, std::vector<VariableType>& types)
{
	const std::vector<Value> values = TokensToValues(
		std::vector<Token>(tokens.begin() + start, tokens.begin() + end)
	);

	const std::shared_ptr<Expression> expression = ValuesToExpression(values);
	const std::vector<VariableType> expr_types = TypeCheckExpression(current_scope, expression);

	types.insert(types.end(), expr_types.begin(), expr_types.end());

	return expression;
}

std::shared_ptr<Expression> Parser::ExtractCondition(Scope& current_scope, std::size_t& close_bracket_index, const std::string& stmt_name)
{
	const std::size_t start = close_bracket_index;

	AdvanceToCloseBracket(tokens, TokenType::OPEN_BRACKET, TokenType::CLOSE_BRACKET, close_bracket_index);

	if (close_bracket_index >= tokens.size())
		throw BackError(file, line, "missing closing bracket for " + stmt_name);

	std::vector<VariableType> types;
	const std::shared_ptr<Expression> condition = ParseTokenExpression(current_scope, start, close_bracket_index, types);

	if (!night::find_type(types, VariableType::BOOL))
		throw BackError(file, line, stmt_name + " condition must evaluate to a boolean value");

	return condition;
}

Scope Parser::ExtractBody(Scope& upper_scope, const std::size_t closeBracketIndex, const std::string& stmt)
{
	const std::vector<std::vector<Token> > splitTokens = tokens[closeBracketIndex + 1].type == TokenType::OPEN_CURLY
		? SplitCode(std::vector<Token>(tokens.begin() + closeBracketIndex + 2, tokens.end() - 1))
		: SplitCode(std::vector<Token>(tokens.begin() + closeBracketIndex + 1, tokens.end()));

	Scope body{ &upper_scope };
	for (const std::vector<Token>& toks : splitTokens)
	{
		Parser parse(body, toks);

		if (body.statements.back().type == StatementType::FUNCTION_DEF)
			throw BackError(toks[0].file, toks[0].line, "function definition found in " + stmt + "; " + stmt + "s cannot contain function definitions");
	}

	return body;
}

/* private variables */

bool Parser::in_function = false;
std::vector<VariableType> Parser::return_types;

const std::vector<VariableType> Parser::all_types{
	VariableType::BOOL, VariableType::NUM,
	VariableType::STR, VariableType::ARRAY
};

std::vector<CheckFunction> Parser::check_functions{
	CheckFunction{ "print", { { all_types } }, std::vector<VariableType>()                    },
	CheckFunction{ "input", {},                std::vector<VariableType>{ VariableType::STR } }
};
std::vector<CheckClass>    Parser::check_classes{
	CheckClass{
		"array",
		std::vector<CheckVariable>(),
		std::vector<CheckFunction>{
			{ "len",  {},                                                 std::vector<VariableType>{ VariableType::NUM } },
			{ "push", { { VariableType::ARRAY } },                        std::vector<VariableType>()			         },
			{ "push", { { VariableType::NUM }, { VariableType::ARRAY } }, std::vector<VariableType>()					 },
			{ "pop",  {},                                                 std::vector<VariableType>()					 },
			{ "pop",  { { VariableType::NUM } },                          std::vector<VariableType>()					 }
		}
	},
	CheckClass{
		"string",
		std::vector<CheckVariable>(),
		std::vector<CheckFunction>{
			{ "len",  {}, std::vector<VariableType>{ VariableType::NUM } }
		}
	}
};