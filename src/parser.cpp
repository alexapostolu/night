#include "../include/parser.hpp"
#include "../include/utils.hpp"
#include "../include/token.hpp"
#include "../include/error.hpp"

#include <memory>
#include <algorithm>
#include <string>
#include <vector>
#include <unordered_set>
#include <optional>

/* methods */

Parser::Parser(std::vector<Statement>& statements, const std::vector<Token>& _tokens)
	: file(_tokens[0].file), line(_tokens[0].line), tokens(_tokens)
{
	if (tokens.size() >= 1 && tokens[0].type == TokenType::SET)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::VAR)
			throw Error(file, line, "expected variable name after keyword 'set'");
		if (tokens.size() == 2 || tokens[2].type != TokenType::ASSIGN)
			throw Error(file, line, "expected assignment symbol '=' after variable name");
		if (tokens.size() == 3)
			throw Error(file, line, "expected expression after assignment symbol '='");

		if (night::find_container(check_variables, tokens[1].data))
			throw Error(file, line, "variable '" + tokens[1].data + "' is already defined");
		if (night::find_container(check_functions, tokens[1].data))
			throw Error(file, line, "variable '" + tokens[1].data + "' has the same name as a function; variable and function names must be unique");
		if (night::find_container(check_classes, tokens[1].data))
			throw Error(file, line, "variable '" + tokens[1].data + "' has the same name as a class; variable and class names must be unique");

		std::vector<VariableType> expression_types;
		const std::shared_ptr<Expression> expression = ParseTokenExpression(
			3, tokens.size(), expression_types
		);

		check_variables.push_back(CheckVariable{ tokens[1].data, expression_types });

		statements.push_back(Statement{
			StatementType::VARIABLE,
			Variable{ tokens[1].data, expression }
		});
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VAR && tokens[1].type == TokenType::ASSIGN)
	{
		if (tokens.size() == 2)
			throw Error(file, line, "expected expression after assignment operator");

		CheckVariable* variable = night::get_container(check_variables, tokens[0].data);
		if (variable == nullptr)
			throw Error(file, line, "variable '" + tokens[0].data + "' is not defined");

		if (!variable->is_param())
		{
			if (tokens[1].data == "+=" && !night::find_type(variable->types, VariableType::NUM) &&
				!night::find_type(variable->types, VariableType::STR))
				throw Error(file, line, "variable '" + variable->name + "' does not contain type 'num' or 'str'; assignment '+=' requires variables to contain type 'num' or 'str'");
			if (tokens[1].data != "=" && tokens[1].data != "+=" && !night::find_type(variable->types, VariableType::NUM))
				throw Error(file, line, "variable '" + variable->name + "' does not contain type 'num'; assignment '" + tokens[1].data + "' requires variables to contain type 'num'");
		}

		std::vector<VariableType> expression_types;
		const std::shared_ptr<Expression> expression = ParseTokenExpression(
			2, tokens.size(), expression_types
		);

		if (tokens[1].data != "=" && tokens[1].data != "+=" && !night::find_type(expression_types, VariableType::NUM) &&
			!night::find_type(expression_types, VariableType::STR))
			throw Error(file, line, "expression does not contain type 'num' or 'str'; assignment '+=' requires expressions to contain type 'num' or 'str'");
		if (tokens[1].data != "=" && tokens[1].data == "+=" && !night::find_type(expression_types, VariableType::NUM))
			throw Error(file, line, "expression does not contain type 'num'; assignment '" + tokens[1].data + "' requires expressions to contain type 'num'");

		variable->types.insert(std::end(variable->types), std::begin(expression_types), std::end(expression_types));

		statements.push_back(Statement{
			StatementType::ASSIGNMENT,
			Assignment{ tokens[1].data[0], tokens[0].data, expression }
		});
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::IF)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::OPEN_BRACKET)
			throw Error(file, line, "expected open bracket after 'if' keyword");
		if (tokens.size() == 2)
			throw Error(file, line, "expected close bracket in if condition");
		if (tokens.size() == 3 && tokens[2].type == TokenType::CLOSE_BRACKET)
			throw Error(file, line, "expected expression in between brackets");

		std::size_t closeBracketIndex = 2;

		const std::shared_ptr<Expression> condition = ExtractCondition(closeBracketIndex, "if condition");
		const std::vector<Statement> body = ExtractBody(closeBracketIndex, "if statement");

		statements.push_back(Statement{
			StatementType::IF_STATEMENT,
			IfStatement{ { Conditional{ condition, body } } }
	});
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::ELSE && tokens[1].type == TokenType::IF)
	{
		if (tokens.size() == 2 || tokens[2].type != TokenType::OPEN_BRACKET)
			throw Error(file, line, "expected open bracket after 'if' keyword");
		if (tokens.size() == 3)
			throw Error(file, line, "expected close bracket in if condition");
		if (tokens.size() == 4 && tokens[3].type == TokenType::CLOSE_BRACKET)
			throw Error(file, line, "expected expression in between brackets");
		if (statements.size() == 0 || statements.back().type != StatementType::IF_STATEMENT ||
			std::get<Conditional>(statements.back().stmt).is_else())
			throw Error(file, line, "else if statements are required to come after an if or else if statement");

		std::size_t closeBracketIndex = 3;

		const std::shared_ptr<Expression> conditionExpr = ExtractCondition(closeBracketIndex, "else if condition");
		const std::vector<Statement> body = ExtractBody(closeBracketIndex, "else if statement");

		std::get<Conditional>(statements.back().stmt).chains.push_back(
			Conditional{ conditionExpr, body }
		);
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::ELSE)
	{
		if (statements.size() == 0 || statements.back().type != StatementType::IF_STATEMENT ||
			std::get<Conditional>(statements.back().stmt).is_else())
			throw Error(file, line, "else statement is required to come after an if or else if statement");

		std::get<Conditional>(statements.back().stmt).chains.push_back(
			Conditional{ nullptr, ExtractBody(0, "else statement") }
		);
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::DEF)
	{
		if (in_function)
			throw Error(file, line, "function definition found within function definition; functions cannot contain functions");

		if (tokens.size() == 1 || tokens[1].type != TokenType::VAR)
			throw Error(file, line, "expected function name after 'def' keyword");
		if (tokens.size() == 2 || tokens[2].type != TokenType::OPEN_BRACKET)
			throw Error(file, line, "expected open bracket after function name");
		if (tokens.size() == 3)
			throw Error(file, line, "expected open bracket after function name");
		if (tokens.size() == 4 && tokens[3].type != TokenType::CLOSE_BRACKET)
			throw Error(file, line, "expected closing bracket for function parameters");
		if (tokens.size() == 4 || (tokens.size() == 5 && tokens[4].type != TokenType::OPEN_CURLY))
			throw Error(file, line, "expected open curly bracket for function body");

		if (night::find_container(check_variables, tokens[1].data))
			throw Error("function '" + tokens[1].data + "' has the same name as a variable; variable and function names are required to be unique");
		if (night::find_container(check_functions, tokens[1].data))
			throw Error("function '" + tokens[1].data + "' is already defined");
		if (night::find_container(check_classes, tokens[1].data))
			throw Error("function '" + tokens[1].data + "' has the same name as a class; function and class names are required to be unique");

		// function parameters

		std::vector<std::string> parameter_names;
		const std::size_t variable_size = check_variables.size();

		std::size_t closeBracketIndex = 3;
		for (; closeBracketIndex < tokens.size() && tokens[closeBracketIndex].type != TokenType::CLOSE_BRACKET; closeBracketIndex += 2)
		{
			if (tokens[closeBracketIndex].type != TokenType::VAR)
				throw Error(file, line, "expected variable names as function parameters");

			if (night::find_container(check_variables, tokens[closeBracketIndex].data))
				throw Error(file, line, "function parameter cannot have the same name as another variable");
			if (night::find_container(check_functions, tokens[closeBracketIndex].data))
				throw Error(file, line, "function parameter cannot have the same name as a function");
			if (night::find_container(check_classes, tokens[closeBracketIndex].data))
				throw Error(file, line, "function parameter cannot have the same name as a class");

			parameter_names.push_back(tokens[closeBracketIndex].data);
			check_variables.push_back({ tokens[closeBracketIndex].data });

			if (tokens[closeBracketIndex + 1].type == TokenType::CLOSE_BRACKET)
			{
				closeBracketIndex++;
				break;
			}

			if (tokens[closeBracketIndex + 1].type != TokenType::COMMA)
				throw Error(file, line, "expected comma or closing bracket after function parameter");
		}

		if (tokens[closeBracketIndex].type != TokenType::CLOSE_BRACKET)
			throw Error(file, line, "expected closing bracket after function parameters");
		if (tokens[closeBracketIndex + 1].type != TokenType::OPEN_CURLY)
			throw Error(file, line, "expected opening curly after function parameters");

		// function statement

		// define function before extracting body to ensure function is defined for recursion
		check_functions.push_back({ tokens[1].data });

		CheckFunction* check_function = &check_functions.back();
		//check_function->return_values = std::vector<std::pair<std::string, VariableType> >();
		check_function->parameters.resize(parameter_names.size());

		in_function = true;

		FunctionDef function{
			tokens[1].data,
			parameter_names,
			ExtractBody(closeBracketIndex, "function definitions"),
			return_types
		};

		in_function = false;
		return_types.clear();

		// check function

		for (std::size_t a = variable_size; a < check_variables.size(); ++a)
		{
			// what is check_variables[a].types is empty because parameter encountered no expressions?
			// if (check_variables[a].types.empty()), give it all the types
			check_function->parameters[a - variable_size] = check_variables[a].types;
		}

		check_variables.erase(std::begin(check_variables) + variable_size, std::end(check_variables));

		check_function->return_types = function.return_types;

		// pushing statement

		statements.push_back(Statement{ StatementType::FUNCTION_DEF, function });
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VAR && tokens[1].type == TokenType::OPEN_BRACKET)
	{
		if (tokens.size() == 2 || (tokens.size() == 3 && tokens[2].type != TokenType::CLOSE_BRACKET))
			throw Error(file, line, "missing closing bracket");

		const CheckFunction* checkFunction = night::get_container(check_functions, tokens[0].data);
		if (checkFunction == nullptr)
			throw Error(file, line, "function " + tokens[0].data + "' is not defined");

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
						throw Error(file, line, "unexpected tokens after function call; each statement must be on it's own line");

					std::vector<VariableType> get_argument_types;

					argument_expressions.push_back(
						ParseTokenExpression(start, a, get_argument_types)
					);

					argument_types.push_back(get_argument_types);

					start = a + 1;
					continue;
				}
			}
		}

		// error checking arguments with function parameters

		if (argument_types.size() != checkFunction->parameters.size())
			throw Error(file, line, "function '" + tokens[0].data + "' is called with '" + std::to_string(argument_types.size()) + "' arguments but is defined with '" + std::to_string(checkFunction->parameters.size()) + "' parameters");

		for (std::size_t a = 0; a < checkFunction->parameters.size(); ++a)
		{
			for (const VariableType& parameterType : argument_types[a])
			{
				if (!night::find_type(checkFunction->parameters[a], parameterType))
					throw Error(file, line, "argument number '" + std::to_string(a + 1) + "' for function call '" + tokens[0].data + "' cannot be used because of a type mismatch");
			}
		}

		// pushing statement

		statements.push_back(Statement{
			StatementType::FUNCTION_CALL,
			FunctionCall{ tokens[0].data, argument_expressions }
		});
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::RETURN)
	{
		if (!in_function)
			throw Error(file, line, "return statement is outside of a function definition; return statements can only be inside functions");
		if (tokens.size() == 1)
			throw Error(file, line, "expected expression after 'return' keyword");

		statements.push_back(Statement{
			StatementType::RETURN,
			Return{ ParseTokenExpression(1, tokens.size(), return_types) }
		});
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::WHILE)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::OPEN_BRACKET)
			throw Error(file, line, "expected open bracket after 'while' keyword");
		if (tokens.size() == 2)
			throw Error(file, line, "expected condition in while loop");
		if (tokens.size() == 3 && tokens[2].type != TokenType::CLOSE_BRACKET)
			throw Error(file, line, "expected closing bracket in condition");
		if (tokens.size() == 3 && tokens[2].type == TokenType::CLOSE_BRACKET)
			throw Error(file, line, "expected condition in between brackets");

		std::size_t closeBracketIndex = 2;

		const std::shared_ptr<Expression> condition = ExtractCondition(closeBracketIndex, "while loop");
		const std::vector<Statement> body = ExtractBody(closeBracketIndex, "while loop");

		statements.push_back(Statement{
			StatementType::WHILE_LOOP,
			WhileLoop{ condition, body }
		});
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::FOR)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::OPEN_BRACKET)
			throw Error(file, line, "expected open bracket after 'for' keyword");
		if (tokens.size() == 2 || tokens[2].type != TokenType::VAR)
			throw Error(file, line, "expected iterator name after open bracket");
		if (tokens.size() == 3 || tokens[3].type != TokenType::COLON)
			throw Error(file, line, "expected colon after iterator name");
		if (tokens.size() == 4)
			throw Error(file, line, "expected array after colon");

		if (night::find_container(check_variables, tokens[2].data))
			throw Error(file, line, "variable '" + tokens[2].data + "' is already defined");

		// evaluating iterator and range

		std::size_t closeBracketIndex = 4;
		AdvanceToCloseBracket(tokens, TokenType::OPEN_BRACKET, TokenType::CLOSE_BRACKET, closeBracketIndex);

		if (closeBracketIndex >= tokens.size())
			throw Error(file, line, "missing closing bracket in for loop conditions");

		const std::vector<Value> range_values = TokensToValues(std::vector<Token>(std::begin(tokens) + 4, std::begin(tokens) + closeBracketIndex));
		const std::shared_ptr<Expression> range_expr = ValuesToExpression(range_values);

		bool return_element_types = false;
		const std::vector<VariableType> range_types = TypeCheckExpression(range_expr, "", {}, &return_element_types);

		if (!return_element_types)
			throw Error(file, line, "range must be of type array");

		check_variables.push_back(CheckVariable{ tokens[2].data, range_types });

		// extracting scope and constructing statement

		const std::vector<Statement> body = ExtractBody(closeBracketIndex, "for loop");

		check_variables.pop_back(); // remove the iterator variable

		statements.push_back(Statement{
			StatementType::FOR_LOOP,
			ForLoop{ tokens[2].data, range_expr, body }
		});
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VAR && tokens[1].type == TokenType::OPEN_SQUARE)
	{
		if (tokens.size() == 2)
			throw Error(file, line, "expected index after open square");

		CheckVariable* check_variable = night::get_container(check_variables, tokens[0].data);
		if (check_variable == nullptr)
			throw Error(file, line, "variable '" + tokens[1].data + "' is not defined");

		// check if variable is an array or a string

		const bool is_array = night::find_type(check_variable->types, VariableType::ARRAY);

		if (!is_array && night::find_type(check_variable->types, VariableType::STR))
			throw Error(file, line, "variable '" + check_variable->name + "' doesn't contain a string or an array; to be access with an index, a variable must contain a string or an array");

		// extract and evaluate element

		std::size_t assignmentIndex = 2;
		for (; assignmentIndex < tokens.size() && tokens[assignmentIndex].type != TokenType::ASSIGN; ++assignmentIndex);

		if (tokens[assignmentIndex - 1].type != TokenType::CLOSE_SQUARE)
			throw Error(file, line, "missing closing square");
		if (assignmentIndex == tokens.size())
			throw Error(file, line, "missing assignment operator");

		std::vector<VariableType> elemType;
		const std::shared_ptr<Expression> elemExpr = ParseTokenExpression(
			2, assignmentIndex - 1, elemType
		);

		if (!night::find_type(elemType, VariableType::NUM))
			throw Error(file, line, "index for array '" + tokens[0].data + "' is required to be of type 'int'");

		// extract expression

		std::vector<VariableType> tips;
		const std::shared_ptr<Expression> assignExpr = ParseTokenExpression(
			assignmentIndex + 1, tokens.size(), tips
		);

		check_variable->types.insert(std::end(check_variable->types), std::begin(tips), std::end(tips));

		if (!is_array && (assignExpr->type != ValueType::STR || assignExpr->data.length() >= 1))
			throw Error(file, line, "a string element can only be assigned to a string of length 1");

		// pushing statement

		statements.push_back(Statement{
			StatementType::ELEMENT,
			Element{ tokens[0].data, elemExpr, assignExpr }
		});
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VAR && tokens[1].type == TokenType::OPERATOR)
	{
		if (tokens[1].data != ".")
			throw Error(file, line, "invalid syntax");
		if (tokens.size() == 2)
			throw Error(file, line, "expected method name after dot operator");

		std::vector<VariableType> temp;
		const std::shared_ptr<Expression> object = ParseTokenExpression(0, tokens.size(), temp);

		statements.push_back(Statement{
			StatementType::METHOD_CALL,
			MethodCall{ tokens[0].data, object }
		});
	}
	else
	{
		throw Error(file, line, "invalid syntax");
	}
}

std::vector<Value> Parser::TokensToValues(const std::vector<Token>& tokens)
{
	assert(!tokens.empty() && "tokens shouldn't be empty");

	std::vector<Value> values;
	for (std::size_t a = 0; a < tokens.size(); ++a)
	{
		if (tokens[a].type == TokenType::VAR)
		{
			if (a < tokens.size() - 1 && tokens[a + 1].type == TokenType::OPEN_BRACKET)
			{
				Value value_call{ ValueType::CALL, tokens[a].data };

				if (tokens[a + 2].type == TokenType::CLOSE_BRACKET)
				{
					values.push_back(value_call);
					continue;
				}

				a += 2;

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
							TokensToValues(night::access(tokens, start, a))
						);

						start = a + 1;

						if (tokens[a].type == TokenType::CLOSE_BRACKET)
							break;

						continue;
					}
				}

				if (a >= tokens.size())
					throw Error(file, line, "missing closing bracket for function call");

				values.push_back(value_call);
			}
			else if (a < tokens.size() - 1 && tokens[a + 1].type == TokenType::OPEN_SQUARE)
			{
				values.push_back(Value{ ValueType::VARIABLE, tokens[a].data });

				while (a < tokens.size() - 1 && tokens[a + 1].type == TokenType::OPEN_SQUARE)
				{
					a += 2;
					const std::size_t start = a;

					AdvanceToCloseBracket(tokens, TokenType::OPEN_SQUARE, TokenType::CLOSE_SQUARE, a);
					if (a >= tokens.size())
						throw Error(file, line, "missing closing square bracket for subscript operator");

					values.push_back({ ValueType::OPERATOR, "[]" });
					values.back().extras.push_back(TokensToValues(
						std::vector<Token>(tokens.begin() + start, tokens.begin() + a)
					));
				}
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
					value_array.extras.push_back(
						TokensToValues(night::access(tokens, start, a))
					);

					start = a + 1;

					if (tokens[a].type == TokenType::CLOSE_SQUARE)
						break;

					continue;
				}
			}

			if (a >= tokens.size())
				throw Error(file, line, "missing closing square bracket for array");

			values.push_back(value_array);
		}
		else
		{
			switch (tokens[a].type)
			{
			case TokenType::BOOL:
				values.push_back(Value{ ValueType::BOOL, tokens[a].data });
				break;
			case TokenType::NUM:
				values.push_back(Value{ ValueType::NUM, tokens[a].data });
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
				throw Error(file, line, "unexpected token '" + tokens[a].data + "' in expression");
			}
		}
	}

	return values;
}

std::shared_ptr<Expression> Parser::new_expression(const Value& value, const std::shared_ptr<Expression>& left,
	const std::shared_ptr<Expression>& right)
{
	std::shared_ptr<Expression> expression = std::make_shared<Expression>(Expression{
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
		throw Error(file, line, "unary operators cannot be adjacent to one another");

	/*
	int openBracketCount = 0;
	for (std::size_t start = index; index < values.size(); ++index)
	{
		if (values[index].type == ValueType::OPEN_BRACKET)
			openBracketCount++;
		else if (values[index].type == ValueType::CLOSE_BRACKET)
			openBracketCount--;

		if ((values[index].type == ValueType::OPERATOR || index == values.size() - 1) && openBracketCount == 0)
		{
			std::shared_ptr<Expression> groupExpression;
			if (values[start].type == ValueType::OPEN_BRACKET)
			{
				groupExpression = ValuesToExpression(
					night::access(values, start + 1, index - (index == values.size() - 1 ? 0 : 1))
				);
			}
			else
			{
				groupExpression = new_expression(values[start], nullptr, nullptr);
			}

			while (index < values.size() && values[index].type == ValueType::OPERATOR && values[index].data == "[]")
				groupExpression = new_expression(values[index++], nullptr, groupExpression);

			return isFrontUnary
				? new_expression(values[start - 1], nullptr, groupExpression)
				: groupExpression;
		}
	}
	*/

	// increments index to next operator
	std::size_t start = index;
	for (int open_bracket_count = 0; index < values.size(); ++index)
	{
		if (values[index].type == ValueType::OPEN_BRACKET)
			open_bracket_count++;
		else if (values[index].type == ValueType::CLOSE_BRACKET)
			open_bracket_count--;
		
		if (open_bracket_count == 0 && (values[index].type == ValueType::OPERATOR || index == values.size() - 1))
		{
			if (index == values.size() - 1)
			{
				if (values[index].type == ValueType::OPERATOR)
					throw Error(file, line, "expected value after operator '" + values[index].data + "'");

				index++;
			}

			break;
		}
		else if (index == values.size() - 1 && open_bracket_count > 0)
		{
			throw Error(file, line, "missing closing bracket in expression");
		}
		else if (index == values.size() - 1 && open_bracket_count < 0)
		{
			throw Error(file, line, "missing opening bracket in expression");
		}
	}

	// change to regular pointer?
	std::shared_ptr<Expression> group_expression = values[start].type == ValueType::OPEN_BRACKET
		? ValuesToExpression(night::access(values, start + 1, index - 1))
		: new_expression(values[start], nullptr, nullptr);

	while (index < values.size() && values[index].type == ValueType::OPERATOR && values[index].data == "[]")
		group_expression = new_expression(values[index++], nullptr, group_expression);

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
			throw Error(file, line, "missing operator between values in expression");

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

		const std::size_t opIndex = a;

		a++;

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

std::vector<VariableType> Parser::TypeCheckExpression(const std::shared_ptr<Expression>& node,
	const std::string& op_name, const std::vector<VariableType>& required_types, bool* turn_into_array)
{
	// this variable is used to store the class in an expression, since
	// returning VariableType::CLASS isn't enough - the name of the class
	// is also needed
	//
	// however, there can be multiple classes that are returned, so we
	// need a vector to store them
	//
	// but that's going to be a big change, so let's just focus on one
	// class return type first :)
	//
	//std::vector<CheckClass*> check_object
	static CheckClass* check_object;

	assert(node != nullptr && "node cannot be NULL");

	// if node->left and node->right is NULL, then node must be a value
	if (node->left == nullptr && node->right == nullptr)
	{
		if (node->type == ValueType::ARRAY)
		{
			if (turn_into_array == nullptr)
				return { VariableType::ARRAY };

			std::vector<VariableType> element_types;
			for (const std::shared_ptr<Expression>& element : node->extras)
			{
				const std::vector<VariableType> element_type = TypeCheckExpression(element);
				element_types.insert(std::end(element_types), std::begin(element_type), std::end(element_type));
			}

			*turn_into_array = true;
			return element_types;
		}
		if (node->type == ValueType::VARIABLE)
		{
			CheckVariable* check_variable = night::get_container(check_variables, node->data);
			if (check_variable == nullptr)
			{
				// bottom part of an AST
				// you can't return just CLASS, you also need to know what class it is
				// so that's why we store that info in this static variable
				check_object = night::get_container(check_classes, node->data);
				if (check_object == nullptr)
					throw Error(file, line, "variable '" + node->data + "' is undefined");
				
				return { VariableType::CLASS };
			}

			if (check_variable->is_param())
			{
				// if required_types is empty, then it must be a for loop range
				check_variable->types = required_types.empty()
					? std::vector<VariableType>{ VariableType::ARRAY }
					: required_types;
			}

			return check_variable->types;
		}
		if (node->type == ValueType::CALL)
		{
			CheckFunction* check_function = night::get_container(check_functions, node->data);
			if (check_function == nullptr)
				throw Error(file, line, "function '" + node->data + "' is not defined");

			if (check_function->is_void)
				throw Error(file, line, "function '" + node->data + "' doesn't return a value; functions used in expressions must return a value");
			if (node->extras.size() != check_function->parameters.size())
				throw Error(file, line, "function '" + node->data + "' was called with '" + std::to_string(node->extras.size()) + "' arguments, but was defined with '" + std::to_string(check_function->parameters.size()) + "' parameters");

			// type checking function parameters
			for (std::size_t a = 0; a < check_function->parameters.size(); ++a)
			{
				const std::vector<VariableType> argumentTypes = TypeCheckExpression(node->extras[a], "", {});

				if (check_function->parameters[a].empty())
				{
					check_function->parameters[a] = argumentTypes;
					continue;
				}

				for (const VariableType& argumentType : argumentTypes)
				{
					if (!night::find_type(check_function->parameters[a], argumentType))
						throw Error(file, line, "argument number '" + std::to_string(a + 1) + "' for function call '" + check_function->name + "' cannot be used because of a type mismatch");
				}
			}

			// type checking return values
			if (check_function->return_types.empty())
			{
				check_function->return_types = required_types;
			}
			else if (!check_function->return_types.empty() && !required_types.empty())
			{
				for (const VariableType& required_type : required_types)
				{
					if (!night::find_type(check_function->return_types, required_type))
						throw Error(file, line, "function '" + node->data + "' can not be used with operator '" + op_name + "'");
				}
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
			throw Error(file, line, "unexpected token '" + node->data + "' in expression");
		}
	}

	assert(node->type == ValueType::OPERATOR && "node must be an operator");

	if (node->data == "+")
	{
		const std::vector<VariableType> plus_types{ VariableType::NUM, VariableType::STR };

		const std::vector<VariableType> left  = TypeCheckExpression(node->left,  node->data, plus_types);
		const std::vector<VariableType> right = TypeCheckExpression(node->right, node->data, plus_types);

		if (night::find_type(left, VariableType::NUM) && night::find_type(right, VariableType::NUM))
			return { VariableType::NUM };
		if (night::find_type(left, VariableType::STR) || night::find_type(right, VariableType::STR))
			return { VariableType::STR };

		throw Error(file, line, "binary operator '" + node->data + "' requires two types of 'num', or at least one type of 'str'");
	}
	if (node->data == "-")
	{
		// unary operator (negative)

		if (node->left == nullptr)
		{
			const std::vector<VariableType> right = TypeCheckExpression(node->right, node->data, { VariableType::NUM });

			if (!night::find_type(right, VariableType::NUM))
				throw Error(file, line, "unary operator '" + node->data + "' requires to be used on a type of 'num'");

			return { VariableType::NUM };
		}

		// binary operator (minus)

		const std::vector<VariableType> left  = TypeCheckExpression(node->left,  node->data, { VariableType::NUM });
		const std::vector<VariableType> right = TypeCheckExpression(node->right, node->data, { VariableType::NUM });

		if (!night::find_type(left, VariableType::NUM) || !night::find_type(right, VariableType::NUM))
			throw Error(file, line, "binary operator '" + node->data + "' can only be used on two numbers");
		
		return { VariableType::NUM };
	}
	if (node->data == "*")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(node->left,  node->data, { VariableType::NUM });
		const std::vector<VariableType> right = TypeCheckExpression(node->right, node->data, { VariableType::NUM });

		if (!night::find_type(left, VariableType::NUM) || !night::find_type(right, VariableType::NUM))
			throw Error(file, line, "binary operator '" + node->data + "' can only be used on two numbers");
		
		return { VariableType::NUM };
	}
	if (node->data == "/")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(node->left,  node->data, { VariableType::NUM });
		const std::vector<VariableType> right = TypeCheckExpression(node->right, node->data, { VariableType::NUM });

		if (!night::find_type(left, VariableType::NUM) || !night::find_type(right, VariableType::NUM))
			throw Error(file, line, "binary operator '" + node->data + "' can only be used on two numbers");
		
		return { VariableType::NUM };
	}
	if (node->data == "%")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(node->left,  node->data, { VariableType::NUM });
		const std::vector<VariableType> right = TypeCheckExpression(node->right, node->data, { VariableType::NUM });

		if (!night::find_type(left, VariableType::NUM) || !night::find_type(right, VariableType::NUM))
			throw Error(file, line, "binary operator '" + node->data + "' can only be used on two numbers");

		return { VariableType::NUM };
	}
	if (node->data == ">")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(node->left, node->data, { VariableType::NUM });
		const std::vector<VariableType> right = TypeCheckExpression(node->right, node->data, { VariableType::NUM });

		if (!night::find_type(left, VariableType::NUM) || !night::find_type(right, VariableType::NUM))
			throw Error(file, line, "binary operator '" + node->data + "' can only be used on two values of type 'num'");

		return { VariableType::BOOL };
	}
	if (node->data == "<")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(node->left,  node->data, { VariableType::NUM });
		const std::vector<VariableType> right = TypeCheckExpression(node->right, node->data, { VariableType::NUM });

		if (!night::find_type(left, VariableType::NUM) || !night::find_type(right, VariableType::NUM))
			throw Error(file, line, "binary operator '" + node->data + "' can only be used on two values of type 'num'");

		return { VariableType::BOOL };
	}
	if (node->data == ">=")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(node->left,  node->data, { VariableType::NUM });
		const std::vector<VariableType> right = TypeCheckExpression(node->right, node->data, { VariableType::NUM });

		if (!night::find_type(left, VariableType::NUM) || !night::find_type(right, VariableType::NUM))
			throw Error(file, line, "binary operator '" + node->data + "' can only be used on two values of type 'bool'");

		return { VariableType::BOOL };
	}
	if (node->data == "<=")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(node->left,  node->data, { VariableType::NUM });
		const std::vector<VariableType> right = TypeCheckExpression(node->right, node->data, { VariableType::NUM });

		if (!night::find_type(left, VariableType::NUM) || !night::find_type(right, VariableType::NUM))
			throw Error(file, line, "binary operator '" + node->data + "' can only be used on two values of type 'bool'");

		return { VariableType::BOOL };
	}
	if (node->data == "!")
	{
		const std::vector<VariableType> right = TypeCheckExpression(node->right, node->data, { VariableType::BOOL });

		if (!night::find_type(right, VariableType::BOOL))
			throw Error(file, line, "unary operator '" + node->data + "' can only be used on a value of type 'bool'");

		return { VariableType::BOOL };
	}
	if (node->data == "||")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(node->left,  node->data, { VariableType::BOOL });
		const std::vector<VariableType> right = TypeCheckExpression(node->right, node->data, { VariableType::BOOL });

		if (!night::find_type(left, VariableType::BOOL) || !night::find_type(right, VariableType::BOOL))
			throw Error(file, line, "binary operator '" + node->data + "' can only be used on two values of type 'bool'");

		return { VariableType::BOOL };
	}
	if (node->data == "&&")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(node->left,  node->data, { VariableType::BOOL });
		const std::vector<VariableType> right = TypeCheckExpression(node->right, node->data, { VariableType::BOOL });

		if (!night::find_type(left, VariableType::BOOL) || !night::find_type(right, VariableType::BOOL))
			throw Error(file, line, "binary operator '" + node->data + "' can only be used on two values of type 'bool'");

		return { VariableType::BOOL };
	}
	if (node->data == "==")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(node->left,  node->data,  all_types);
		const std::vector<VariableType> right = TypeCheckExpression(node->right, node->data, all_types);

		for (const VariableType& type : left)
		{
			if (!night::find_type(right, type))
				throw Error(file, line, "binary operator '" + node->data + "' requires two values of the same type");
		}

		return { VariableType::BOOL };
	}
	if (node->data == "!=")
	{
		const std::vector<VariableType> left  = TypeCheckExpression(node->left,  node->data,  all_types);
		const std::vector<VariableType> right = TypeCheckExpression(node->right, node->data, all_types);

		for (const VariableType& type : left)
		{
			if (!night::find_type(right, type))
				throw Error(file, line, "binary operator '" + node->data + "' requires two values of the same type");
		}

		return { VariableType::BOOL };
	}
	if (node->data == "[]")
	{
		bool tia = false;
		const std::vector<VariableType> array_types = TypeCheckExpression(node->right,     node->data, { VariableType::ARRAY }, &tia);
		const std::vector<VariableType> index_types = TypeCheckExpression(node->extras[0], node->data, { VariableType::NUM });

		if (!tia)
			throw Error(file, line, "operator '" + node->data + "' requires to be used on a string or array");
		if (!night::find_type(index_types, VariableType::NUM))
			throw Error(file, line, "operator '" + node->data + "' requires subscript to be type 'num'");

		// once we know it's an array, then get the individual 
		return array_types;

		/*
		std::vector<VariableType> return_types;
		if (night::find_type(array, VariableType::BOOL_ARR))
			return_types.push_back(VariableType::BOOL);
		if (night::find_type(array, VariableType::MULT_BOOL_ARR))
			return_types.push_back(VariableType::BOOL_ARR);
		if (night::find_type(array, VariableType::NUM_ARR))
			return_types.push_back(VariableType::NUM);
		if (night::find_type(array, VariableType::MULT_NUM_ARR))
			return_types.push_back(VariableType::NUM_ARR);
		if (night::find_type(array, VariableType::STR_ARR))
			return_types.push_back(VariableType::STR);
		if (night::find_type(array, VariableType::MULT_STR_ARR))
			return_types.push_back(VariableType::STR_ARR);
		if (night::find_type(array, VariableType::MULT_EMPTY_ARR))
			return_types.push_back(VariableType::EMPTY_ARR);
		*/

		//return return_types;
	}
	if (node->data == ".")
	{
		assert(node->right->left == nullptr && node->right->right == nullptr && "method can't be part of an expression");

		// the required_types argument is faulty; check to see what methods are present, then send them through to the function
		// so if object is a string, then only send VariableType::STR through
		const std::vector<VariableType> object = TypeCheckExpression(
			node->left, node->data, { VariableType::STR, VariableType::ARRAY, VariableType::CLASS }
		);

		if (check_object == nullptr && node->left->type != ValueType::ARRAY && node->left->type != ValueType::STR)
			throw Error(file, line, "variable '" + node->left->data + "' is used with methods; only objects can be used with methods");

		// find return type of method
		const CheckFunction* method = night::get_container(check_object->methods, node->right->data);
		if (method == nullptr)
			throw Error(file, line, "object '" + check_object->name + "' does not have method '" + node->right->data + "()'");
		
		if (method->is_void)
			throw Error(file, line, "method '" + method->name + "' has a NULL return type; methods in expressions are required to have non-NULL return types");
				
		check_object = nullptr;

		// if return type is a class, set check_object to it
		for (const VariableType& return_type : method->return_types)
		{
			if (return_type.type == VariableType::CLASS)
			{
				check_object = night::get_container(check_classes, return_type.class_name);
				assert(check_object != nullptr && "class name should be defined when the method is defined; check the method definition section of the parser to catch this error");
			}
		}

		return method->return_types;
	}

	assert(false && "operator missing from list");
	return {};
}

std::shared_ptr<Expression> Parser::ParseTokenExpression(const std::size_t start,
	const std::size_t end, std::vector<VariableType>& types)
{
	const std::vector<Value> values = TokensToValues(night::access(tokens, start, end));
	const std::shared_ptr<Expression> expression = ValuesToExpression(values);
	const std::vector<VariableType> expr_types = TypeCheckExpression(expression);

	types.insert(std::end(types), std::begin(expr_types), std::end(expr_types));

	return expression;
}

std::shared_ptr<Expression> Parser::ExtractCondition(std::size_t& close_bracket_index, const std::string& stmt_name)
{
	const std::size_t start = close_bracket_index;

	AdvanceToCloseBracket(tokens, TokenType::OPEN_BRACKET, TokenType::CLOSE_BRACKET, close_bracket_index);

	if (close_bracket_index >= tokens.size())
		throw Error(file, line, "missing closing bracket for " + stmt_name);

	std::vector<VariableType> types;
	const std::shared_ptr<Expression> conditionExpr = ParseTokenExpression(start, close_bracket_index, types);

	if (!night::find_type(types, VariableType::BOOL))
		throw Error(file, line, stmt_name + " condition must evaluate to a boolean value");

	return conditionExpr;
}

std::vector<Statement> Parser::ExtractBody(const std::size_t closeBracketIndex, const std::string& stmt)
{
	const std::vector<std::vector<Token> > splitTokens = tokens[closeBracketIndex + 1].type == TokenType::OPEN_CURLY
		? SplitCode(std::vector<Token>(std::begin(tokens) + closeBracketIndex + 2, std::end(tokens) - 1))
		: SplitCode(std::vector<Token>(std::begin(tokens) + closeBracketIndex + 1, std::end(tokens)));

	const std::size_t variableSize = check_variables.size();

	std::vector<Statement> body;
	for (const std::vector<Token>& toks : splitTokens)
	{
		Parser(body, toks);

		if (body.back().type == StatementType::FUNCTION_DEF)
			throw Error(toks[0].file, toks[0].line, "function definition found in " + stmt + "; " + stmt + "s cannot contain function definitions");
	}

	check_variables.erase(std::begin(check_variables) + variableSize, std::end(check_variables));

	return body;
}

/* "Check" struct methods */

bool Parser::CheckVariable::is_param() const
{
	return types.empty();
}

bool Parser::CheckFunction::is_empty() const
{
	return !is_void && return_types.empty();
}

/* private variables */

bool Parser::in_function = false;
std::vector<VariableType> Parser::return_types;

const std::vector<VariableType> Parser::all_types{
	VariableType::BOOL, VariableType::NUM,
	VariableType::STR, VariableType::ARRAY
};

std::vector<Parser::CheckVariable> Parser::check_variables;
std::vector<Parser::CheckFunction> Parser::check_functions{
	CheckFunction{ "print", { { all_types } }, std::vector<VariableType>()                                 },
	CheckFunction{ "input", {},                std::vector<VariableType>{ { nullptr, VariableType::STR } } }
};
std::vector<Parser::CheckClass>    Parser::check_classes{
	CheckClass{
		"array",
		std::vector<CheckVariable>(),
		std::vector<CheckFunction>{
			{ "len",  {},                                                 std::vector<VariableType>{ { nullptr, VariableType::NUM } } },
			{ "push", { { VariableType::ARRAY } },                        std::vector<VariableType>()								 },
			{ "push", { { VariableType::NUM }, { VariableType::ARRAY } }, std::vector<VariableType>()								 },
			{ "pop",  {},                                                 std::vector<VariableType>()								 },
			{ "pop",  { { VariableType::NUM } },                          std::vector<VariableType>()								 }
		}
	},
	CheckClass{
		"string",
		std::vector<CheckVariable>(),
		std::vector<CheckFunction>{
			{ "len",  {}, std::vector<VariableType>{ { nullptr, VariableType::NUM } } }
		}
	}
};