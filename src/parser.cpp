#include "../include/parser.hpp"
#include "../include/utils.hpp"
#include "../include/token.hpp"
#include "../include/error.hpp"

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

std::vector<Value> TokensToValues(const std::vector<Token>& tokens)
{
	assert(!tokens.empty() && "tokens shouldn't be empty");

	const std::string file = tokens[0].file;
	const int         line = tokens[0].line;

	std::vector<Value> values;
	for (std::size_t a = 0; a < tokens.size(); ++a)
	{
		if (tokens[a].type == TokenType::VARIABLE)
		{
			if (a < tokens.size() - 1 && tokens[a + 1].type == TokenType::OPEN_BRACKET)
			{
				Value functionCall{ ValueType::CALL, tokens[a].value };

				if (tokens[a + 2].type == TokenType::CLOSE_BRACKET)
				{
					values.push_back(functionCall);
					continue;
				}

				a += 2;

				for (int start = a, openBracketCount = 0, openSquareCount = 0; a < tokens.size(); ++a)
				{
					if (tokens[a].type == TokenType::OPEN_BRACKET)
						openBracketCount++;
					else if (tokens[a].type == TokenType::CLOSE_BRACKET)
						openBracketCount--;
					else if (tokens[a].type == TokenType::OPEN_SQUARE)
						openSquareCount++;
					else if (tokens[a].type == TokenType::CLOSE_SQUARE)
						openSquareCount--;

					if (openSquareCount == 0 && ((tokens[a].type == TokenType::COMMA && openBracketCount == 0) ||
						(tokens[a].type == TokenType::CLOSE_BRACKET && openBracketCount == -1)))
					{
						functionCall.extras.push_back(TokensToValues(
							std::vector<Token>(tokens.begin() + start, tokens.begin() + a)
						));

						start = a + 1;

						if (tokens[a].type == TokenType::CLOSE_BRACKET)
							break;

						continue;
					}
				}

				if (a >= tokens.size())
					throw Error(file, line, "missing closing bracket for function call");

				values.push_back(functionCall);
			}
			else if (a < tokens.size() - 1 && tokens[a + 1].type == TokenType::OPEN_SQUARE)
			{
				values.push_back(Value{ ValueType::VARIABLE, tokens[a].value });

				while (a < tokens.size() - 1 && tokens[a + 1].type == TokenType::OPEN_SQUARE)
				{
					a += 2;
					const std::size_t start = a;

					AdvanceToCloseBracket(tokens, TokenType::OPEN_SQUARE, TokenType::CLOSE_SQUARE, a);
					if (a >= tokens.size())
						throw Error(file, line, "missing closing square bracket for subscript operator");

					Value subscript{ ValueType::OPERATOR, "[]" };
					subscript.extras.push_back(TokensToValues(
						std::vector<Token>(tokens.begin() + start, tokens.begin() + a)
					));

					values.push_back(subscript);
				}
			}
			else
			{
				values.push_back(Value{ ValueType::VARIABLE, tokens[a].value });
			}
		}
		else if (tokens[a].type == TokenType::OPEN_SQUARE)
		{
			Value array;

			a++;

			if (tokens[a].type == TokenType::CLOSE_SQUARE)
			{
				values.push_back(Value{ ValueType::ARRAY });
				continue;
			}

			// parse array elements
			for (int start = a, openBracketCount = 0, openSquareCount = 0; a < tokens.size(); ++a)
			{
				if (tokens[a].type == TokenType::OPEN_BRACKET)
					openBracketCount++;
				else if (tokens[a].type == TokenType::CLOSE_BRACKET)
					openBracketCount--;
				else if (tokens[a].type == TokenType::OPEN_SQUARE)
					openSquareCount++;
				else if (tokens[a].type == TokenType::CLOSE_SQUARE)
					openSquareCount--;

				// end of element reached; add it to array
				if (openBracketCount == 0 && ((tokens[a].type == TokenType::COMMA && openSquareCount == 0) ||
					(tokens[a].type == TokenType::CLOSE_SQUARE && openSquareCount == -1)))
				{
					int arrayDepth = 0;
					array.extras.push_back(TokensToValues(
						std::vector<Token>(tokens.begin() + start, tokens.begin() + a)
					));

					start = a + 1;

					if (tokens[a].type == TokenType::CLOSE_SQUARE)
						break;

					continue;
				}
			}

			if (a >= tokens.size())
				throw Error(file, line, "missing closing square bracket for array");

			values.push_back(array);
		}
		else
		{
			switch (tokens[a].type)
			{
			case TokenType::BOOL_VAL:
				values.push_back(Value{ ValueType::BOOL, tokens[a].value });
				break;
			case TokenType::NUM_VAL:
				values.push_back(Value{ ValueType::NUM, tokens[a].value });
				break;
			case TokenType::STR_VAL:
				values.push_back(Value{ ValueType::STR, tokens[a].value });
				break;
			case TokenType::OPEN_BRACKET:
				values.push_back(Value{ ValueType::OPEN_BRACKET, tokens[a].value });
				break;
			case TokenType::CLOSE_BRACKET:
				values.push_back(Value{ ValueType::CLOSE_BRACKET, tokens[a].value });
				break;
			case TokenType::OPERATOR:
				values.push_back(Value{ ValueType::OPERATOR, tokens[a].value });
				break;
			default:
				throw Error(file, line, "unexpected token '" + tokens[a].value + "' in expression");
			}
		}
	}

	return values;
}

std::shared_ptr<Expression> new_expression(const std::string& file, const int line, const Value& value, const std::shared_ptr<Expression>& left,
	const std::shared_ptr<Expression>& right)
{
	std::shared_ptr<Expression> expression = std::make_shared<Expression>(Expression{
		value.type,
		value.data,
		std::vector<std::shared_ptr<Expression> >(),
		left,
		right
	});

	for (const std::vector<Value>& values : value.extras)
		expression->extras.push_back(ValuesToExpression(file, line, values));

	return expression;
}

std::shared_ptr<Expression> GetNextGroup(const std::string& file, const int line, const std::vector<Value>& values, std::size_t& index)
{
	const bool isFrontUnary = values[index].type == ValueType::OPERATOR && (values[index].data == "!" || values[index].data == "-");
	if (isFrontUnary)
		index++;

	int openBracketCount = 0;
	for (std::size_t start = index; index < values.size(); ++index)
	{
		if (values[index].type == ValueType::OPEN_BRACKET)
			openBracketCount++;
		else if (values[index].type == ValueType::CLOSE_BRACKET)
			openBracketCount--;

		// move this outside of the loop; break the loop if openBrakcetCount == 0 && index == end or operator
		if ((values[index].type == ValueType::OPERATOR || index == values.size() - 1) && openBracketCount == 0)
		{
			std::shared_ptr<Expression> groupExpression;
			if (values[start].type == ValueType::OPEN_BRACKET)
			{
				groupExpression = ValuesToExpression(
					file, line,
					std::vector<Value>(values.begin() + start + 1, values.begin() + index - (index == values.size() - 1 ? 0 : 1))
				);
			}
			else
			{
				groupExpression = new_expression(file, line, values[start], nullptr, nullptr);
			}

			while (index < values.size() && values[index].type == ValueType::OPERATOR && values[index].data == "[]")
				groupExpression = new_expression(file, line, values[index++], nullptr, groupExpression);

			return isFrontUnary
				? new_expression(file, line, values[start - 1], nullptr, groupExpression)
				: groupExpression;
		}
	}

	throw Error(file, line, "missing closing bracket in expression");
}

int GetOperatorPrecedence(const ValueType& type, const std::string& value)
{
	static const std::vector<std::vector<std::string> > operators{
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
		for (const std::string& op : operators[a])
		{
			if (value == op)
				return a;
		}
	}

	assert_rtn(false && "operator missing", 0);
}

std::shared_ptr<Expression> ValuesToExpression(const std::string& file, const int line, const std::vector<Value>& values)
{
	assert(!values.empty() && "values shouldn't be empty");

	std::size_t a = 0;

	std::shared_ptr<Expression> root = GetNextGroup(file, line, values, a);
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

		++a;
		const std::shared_ptr<Expression> nextValue = GetNextGroup(file, line, values, a);
		const std::shared_ptr<Expression> opNode = new_expression(file, line, values[opIndex], curr, nextValue);

		protect = nextValue.get();

		if (curr == root)
			root = opNode;
		else
			prev->right = opNode;
	}

	return root;
}

std::vector<VariableType> TypeCheckExpression(const std::string& file, const int line, const std::shared_ptr<Expression>& node,
	const std::vector<CheckVariable>& variables, const std::vector<CheckFunction>& functions, const std::vector<CheckClass>& classes,
	std::vector<CheckVariable>& parameters)
{
	static int array_depth = 1;

	// if node->left and node->right is NULL, then node must be a value
	if (node->left == nullptr && node->right == nullptr)
	{
		if (node->type == ValueType::ARRAY)
		{
			++array_depth;

			if (node->extras.empty())
				return { VariableType::EMPTY_ARR };

			int set_array_depth = -1;
			std::vector<VariableType> set_array_types;

			for (const std::shared_ptr<Expression>& element : node->extras)
			{
				if (set_array_depth == -1)
				{
					set_array_types = TypeCheckExpression(file, line, element, variables, functions, classes, parameters);
					set_array_depth = array_depth;

					continue;
				}

				const std::vector<VariableType> array_types = TypeCheckExpression(
						file, line, element, variables, functions, classes, parameters
					);

				for (const VariableType& array_type : array_types)
				{
					if (!night::find_type(set_array_types, array_type))
						throw Error(file, line, "elements in array potentially could have different types");
				}

				if (set_array_depth != array_depth)
					throw Error(file, line, "elements in array must contain the same depth");
			}

			array_depth = 1;
			return set_array_types;
		}
		if (node->type == ValueType::VARIABLE)
		{
			const CheckVariable* variable = night::get_container(variables, node->data);
			if (variable == nullptr)
			{
				variable = night::get_container(parameters, node->data);
				if (variable == nullptr)
					throw Error(file, line, "variable '" + node->data + "' is not defined");
			}

			return variable->types;
		}
		if (node->type == ValueType::CALL)
		{
			const CheckFunction* function = night::get_container(functions, node->data);
			if (function == nullptr)
				throw Error(file, line, "function '" + node->data + "' is not defined");

			if (function->returnValues.empty())
				throw Error(file, line, "function '" + node->data + "' doesn't return a value; functions used in expressions must return a value");
			if (node->extras.size() != function->parameters.size())
				throw Error(file, line, "function '" + node->data + "' was called with '" + std::to_string(node->extras.size()) + "' arguments, but was defined with '" + std::to_string(function->parameters.size()) + "' parameters");

			// type checking function parameters
			for (std::size_t a = 0; a < function->parameters.size(); ++a)
			{
				const std::vector<VariableType> argumentTypes = TypeCheckExpression(
					file, line, node->extras[a], variables, functions, classes, parameters
				);

				for (const VariableType& argumentType : argumentTypes)
				{
					if (!night::find_type(function->parameters[a].types, argumentType))
						throw Error(file, line, "argument number '" + std::to_string(a + 1) + "' for function call '" + function->name + "' cannot be used because of a type mismatch");
				}
			}

			return function->returnValues;
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
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM, VariableType::STR };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find_type(parameter->types, VariableType::NUM) && !night::find_type(parameter->types, VariableType::STR))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with '+'");
			}
		}
		if (node->right->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->right->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM, VariableType::STR };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find_type(parameter->types, VariableType::NUM) && !night::find_type(parameter->types, VariableType::STR))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with '+'");
			}
		}

		const std::vector<VariableType> left  = TypeCheckExpression(file, line, node->left,  variables, functions, classes, parameters);
		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, classes, parameters);

		std::vector<VariableType> returnTypes;

		if (night::find_type(left, VariableType::NUM) && night::find_type(right, VariableType::NUM))
			returnTypes.push_back(VariableType::NUM);
		if (night::find_type(left, VariableType::STR) || night::find_type(right, VariableType::STR))
			returnTypes.push_back(VariableType::STR);

		if (!returnTypes.empty())
			throw Error(file, line, "binary operator '+' can only be used on two numbers or in string concatenation");

		return returnTypes;
	}
	if (node->data == "-")
	{
		if (node->right->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->right->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find_type(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '-'");
			}
		}

		if (node->left == nullptr)
		{
			const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, classes, parameters);

			if (!night::find_type(right, VariableType::NUM))
				throw Error(file, line, "unary operator '-' can only be used on numbers");

			return { VariableType::NUM };
		}

		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find_type(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '-'");
			}
		}

		const std::vector<VariableType> left  = TypeCheckExpression(file, line, node->left,  variables, functions, classes, parameters);
		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, classes, parameters);

		if (!night::find_type(left, VariableType::NUM) || !night::find_type(right, VariableType::NUM))
			throw Error(file, line, "binary operator '-' can only be used on two numbers");
		
		return { VariableType::NUM };
	}
	if (node->data == "*")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find_type(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '*'");
			}
		}
		if (node->right->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->right->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find_type(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '*'");
			}
		}

		const std::vector<VariableType> left  = TypeCheckExpression(file, line, node->left,  variables, functions, classes, parameters);
		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, classes, parameters);

		if (!night::find_type(left, VariableType::NUM) || !night::find_type(right, VariableType::NUM))
			throw Error(file, line, "binary operator '*' can only be used on two numbers");
		
		return { VariableType::NUM };
	}
	if (node->data == "/")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find_type(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '/'");
			}
		}
		if (node->right->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->right->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find_type(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '/'");
			}
		}

		const std::vector<VariableType> left  = TypeCheckExpression(file, line, node->left,  variables, functions, classes, parameters);
		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, classes, parameters);

		if (!night::find_type(left, VariableType::NUM) || !night::find_type(right, VariableType::NUM))
			throw Error(file, line, "binary operator '/' can only be used on two numbers");
		
		return { VariableType::NUM };
	}
	if (node->data == "%")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find_type(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '%'");
			}
		}
		if (node->right->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->right->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find_type(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '%'");
			}
		}

		const std::vector<VariableType> left  = TypeCheckExpression(file, line, node->left,  variables, functions, classes, parameters);
		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, classes, parameters);

		if (!night::find_type(left, VariableType::NUM) || !night::find_type(right, VariableType::NUM))
			throw Error(file, line, "binary operator '%' can only be used on two numbers");

		return { VariableType::NUM };
	}
	if (node->data == ">")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find_type(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '>'");
			}
		}
		if (node->right->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->right->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find_type(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '>'");
			}
		}

		const std::vector<VariableType> left  = TypeCheckExpression(file, line, node->left,  variables, functions, classes, parameters);
		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, classes, parameters);

		if (!night::find_type(left, VariableType::NUM) || !night::find_type(right, VariableType::NUM))
			throw Error(file, line, "binary operator '>' can only be used on two numbers");
		
		return { VariableType::BOOL };
	}
	if (node->data == "<")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find_type(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '<'");
			}
		}
		if (node->right->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->right->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find_type(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '<'");
			}
		}

		const std::vector<VariableType> left  = TypeCheckExpression(file, line, node->left,  variables, functions, classes, parameters);
		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, classes, parameters);

		if (!night::find_type(left, VariableType::NUM) || !night::find_type(right, VariableType::NUM))
			throw Error(file, line, "binary operator '<' can only be used on two numbers");
		
		return { VariableType::BOOL };
	}
	if (node->data == ">=")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find_type(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '>='");
			}
		}
		if (node->right->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->right->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find_type(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '>='");
			}
		}

		const std::vector<VariableType> left  = TypeCheckExpression(file, line, node->left,  variables, functions, classes, parameters);
		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, classes, parameters);

		if (!night::find_type(left, VariableType::NUM) || !night::find_type(right, VariableType::NUM))
			throw Error(file, line, "binary operator '>=' can only be used on two numbers");
		
		return { VariableType::BOOL };
	}
	if (node->data == "<=")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find_type(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '<='");
			}
		}
		if (node->right->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->right->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find_type(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '<='");
			}
		}

		const std::vector<VariableType> left  = TypeCheckExpression(file, line, node->left,  variables, functions, classes, parameters);
		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, classes, parameters);

		if (!night::find_type(left, VariableType::NUM) || !night::find_type(right, VariableType::NUM))
			throw Error(file, line, "binary operator '<=' can only be used on two numbers");
		
		return { VariableType::BOOL };
	}
	if (node->data == "!")
	{
		if (node->right->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->right->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::BOOL };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find_type(parameter->types, VariableType::BOOL))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '!'");
			}
		}

		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, classes, parameters);

		if (!night::find_type(right, VariableType::BOOL))
			throw Error(file, line, "unary operator '!' can only be used on booleans");

		return { VariableType::BOOL };
	}
	if (node->data == "||")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::BOOL };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find_type(parameter->types, VariableType::BOOL))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '||'");
			}
		}
		if (node->right->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->right->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::BOOL };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find_type(parameter->types, VariableType::BOOL))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '||'");
			}
		}

		const std::vector<VariableType> left  = TypeCheckExpression(file, line, node->left,  variables, functions, classes, parameters);
		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, classes, parameters);

		if (!night::find_type(left, VariableType::BOOL) || !night::find_type(right, VariableType::BOOL))
			throw Error(file, line, "binary operator '||' can only be used on two numbers");

		return { VariableType::BOOL };
	}
	if (node->data == "&&")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::BOOL };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find_type(parameter->types, VariableType::BOOL))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '&&'");
			}
		}
		if (node->right->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->right->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::BOOL };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find_type(parameter->types, VariableType::BOOL))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '&&'");
			}
		}

		const std::vector<VariableType> left  = TypeCheckExpression(file, line, node->left,  variables, functions, classes, parameters);
		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, classes, parameters);

		if (!night::find_type(left, VariableType::BOOL) || !night::find_type(right, VariableType::BOOL))
			throw Error(file, line, "binary operator '&&' can only be used on two numbers");

		return { VariableType::BOOL };
	}
	if (node->data == "==")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = {
					VariableType::BOOL, VariableType::BOOL_ARR,
					VariableType::NUM, VariableType::NUM_ARR,
					VariableType::STR, VariableType::STR_ARR,
					VariableType::EMPTY_ARR
				};
			}
		}
		if (node->right->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->right->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = {
					VariableType::BOOL, VariableType::BOOL_ARR,
					VariableType::NUM, VariableType::NUM_ARR,
					VariableType::STR, VariableType::STR_ARR,
					VariableType::EMPTY_ARR
				};
			}
		}

		const std::vector<VariableType> left  = TypeCheckExpression(file, line, node->left,  variables, functions, classes, parameters);
		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, classes, parameters);

		for (const VariableType& type : left)
		{
			if (night::find_type(right, type))
				return { VariableType::BOOL };
		}

		throw Error(file, line, "binary operator '==' can only compare values of the same type");
	}
	if (node->data == "!=")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = {
					VariableType::BOOL, VariableType::BOOL_ARR,
					VariableType::NUM, VariableType::NUM_ARR,
					VariableType::STR, VariableType::STR_ARR,
					VariableType::EMPTY_ARR
				};
			}
		}
		if (node->right->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->right->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = {
					VariableType::BOOL, VariableType::BOOL_ARR,
					VariableType::NUM, VariableType::NUM_ARR,
					VariableType::STR, VariableType::STR_ARR,
					VariableType::EMPTY_ARR
				};
			}
		}

		const std::vector<VariableType> left  = TypeCheckExpression(file, line, node->left,  variables, functions, classes, parameters);
		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, classes, parameters);

		for (const VariableType& type : left)
		{
			if (night::find_type(right, type))
				return { VariableType::BOOL };
		}

		throw Error(file, line, "binary operator '!=' can only compare values of the same type");
	}
	if (node->data == "[]")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = {
					VariableType::BOOL_ARR, VariableType::MULT_BOOL_ARR,
					VariableType::NUM_ARR, VariableType::MULT_NUM_ARR,
					VariableType::STR_ARR, VariableType::MULT_STR_ARR,
					VariableType::EMPTY_ARR, VariableType::MULT_EMPTY_ARR,
				};
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find_type(parameter->types, VariableType::BOOL) || !night::find_type(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter " + parameter->name + " cannot be used with operator '[]'");
			}
		}

		const std::vector<VariableType> array = TypeCheckExpression(file, line, node->right,     variables, functions, classes, parameters);
		const std::vector<VariableType> index = TypeCheckExpression(file, line, node->extras[0], variables, functions, classes, parameters);

		if(!night::find_type(array, VariableType::BOOL) || !night::find_type(array, VariableType::NUM))
			throw Error(file, line, "subscript operator can only be used on strings or arrays");
		if (!night::find_type(index, VariableType::NUM))
			throw Error(file, line, "subscript operator has to be a number");

		return array;
	}
	if (node->data == ".")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = night::get_container(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				if (node->right->data == "len" || node->right->data == "push" || node->right->data == "pop")
				{
					parameter->types = {
						VariableType::BOOL_ARR, VariableType::MULT_BOOL_ARR,
						VariableType::NUM_ARR, VariableType::MULT_NUM_ARR,
						VariableType::STR_ARR, VariableType::MULT_STR_ARR,
						VariableType::EMPTY_ARR, VariableType::MULT_EMPTY_ARR,
						VariableType::STR
					};
				}
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (node->right->data == "len" || node->right->data == "push" || node->right->data == "pop")
				{
					if (!night::find_type(parameter->types, VariableType::BOOL_ARR) &&
						!night::find_type(parameter->types, VariableType::MULT_BOOL_ARR) &&
						!night::find_type(parameter->types, VariableType::NUM_ARR) &&
						!night::find_type(parameter->types, VariableType::MULT_NUM_ARR) &&
						!night::find_type(parameter->types, VariableType::STR_ARR) &&
						!night::find_type(parameter->types, VariableType::MULT_STR_ARR) &&
						!night::find_type(parameter->types, VariableType::EMPTY_ARR) &&
						!night::find_type(parameter->types, VariableType::MULT_EMPTY_ARR) &&
						!night::find_type(parameter->types, VariableType::STR))
						throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with method '" + node->right->data + "()'");
				}
			}
		}

		std::vector<VariableType> object = TypeCheckExpression(
			file, line, node->left, variables, functions, classes, parameters
		);

		if (node->right->data == "len" &&
			(night::find_type(object, VariableType::BOOL_ARR) || night::find_type(object, VariableType::MULT_BOOL_ARR) ||
			night::find_type(object, VariableType::NUM_ARR) || night::find_type(object, VariableType::MULT_NUM_ARR) ||
			night::find_type(object, VariableType::STR_ARR) || night::find_type(object, VariableType::MULT_STR_ARR) ||
			night::find_type(object, VariableType::EMPTY_ARR) ||
			night::find_type(object, VariableType::STR)))
			return { VariableType::NUM };
		if (node->right->data == "pop")
		{
			object.push_back(VariableType::EMPTY_ARR);
			return object;
		}
		if (node->right->data == "push")
		{
			std::vector<VariableType> returnTypes;
			const std::vector<VariableType> push_backElemType = TypeCheckExpression(
				file, line, node->right->extras[0], variables, functions, classes, parameters
			);

			if (night::find_type(object, VariableType::BOOL_ARR) && night::find_type(push_backElemType, VariableType::BOOL))
				returnTypes.push_back(VariableType::BOOL_ARR);
			if (night::find_type(object, VariableType::NUM_ARR) && night::find_type(push_backElemType, VariableType::NUM))
				returnTypes.push_back(VariableType::NUM_ARR);
			if (night::find_type(object, VariableType::STR_ARR) && night::find_type(push_backElemType, VariableType::STR))
				returnTypes.push_back(VariableType::STR_ARR);

			return returnTypes;
		}
	}

	assert_rtn(false && "operator missing from list", { (VariableType)0 });
}

void Parser(std::vector<Statement>& statements, const std::vector<Token>& tokens)
{
	static std::vector<CheckVariable> variables;
	static std::vector<CheckFunction> functions;
	static std::vector<CheckClass>    classes;

	static std::vector<CheckVariable> parameters;
	static bool inFunction = false;

	static bool defineOnce = true;
	if (defineOnce)
	{
		functions.push_back(CheckFunction{ "print", {}, {} });
		functions.push_back(CheckFunction{ "input", {}, { VariableType::STR } });

		classes.push_back(CheckClass{ "array" });
		classes.back().methods = {
			CheckFunction{ "len",  {},            { VariableType::NUM }            },
			CheckFunction{ "push", { { "" } }                                   },
			CheckFunction{ "push", { { "", { VariableType::NUM } }, { "" } } },
			CheckFunction{ "pop"                                                   },
			CheckFunction{ "pop",  { { "", { VariableType::NUM } } }            }
		};

		defineOnce = false;
	}

	const std::string file = tokens[0].file;
	const int         line = tokens[0].line;

	assert(!tokens.empty() && "tokens should not be empty");

	if (tokens.size() >= 1 && tokens[0].type == TokenType::SET)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::VARIABLE)
			throw Error(file, line, "expected variable name after 'set' keyword");
		if (tokens.size() == 2 || tokens[2].type != TokenType::ASSIGNMENT)
			throw Error(file, line, "expected assignment operator after variable name");
		if (tokens.size() == 3)
			throw Error(file, line, "expected expression after assignment operator");

		if (night::find_container(variables, tokens[1].value))
			throw Error(file, line, "variable '" + tokens[1].value + "' is already defined");
		if (night::find_container(functions, tokens[1].value))
			throw Error(file, line, "variable '" + tokens[1].value + "' has the same name as a function; variable and function names must be unique");
		if (night::find_container(classes, tokens[1].value))
			throw Error(file, line, "variable '" + tokens[1].value + "' has the same name as a class; variable and class names must be unique");
		if (night::find_container(parameters, tokens[1].value))
			throw Error(file, line, "variable '" + tokens[1].value + "' cannot have the same name as a parameter");

		std::vector<VariableType> types;
		const std::shared_ptr<Expression> expression = ParseTokenExpression(
			tokens, 3, tokens.size(), variables, functions, classes, parameters, &types
		);

		variables.push_back(CheckVariable{ tokens[1].value, types });

		statements.push_back(Statement{
			StatementType::VARIABLE,
			Variable{ tokens[1].value, expression }
		});
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VARIABLE && tokens[1].type == TokenType::ASSIGNMENT)
	{
		if (tokens.size() == 2)
			throw Error(file, line, "expected expression after assignment operator");

		CheckVariable* variable = night::get_container(variables, tokens[0].value);
		if (variable == nullptr)
		{
			variable = night::get_container(parameters, tokens[0].value);
			if (variable == nullptr)
				throw Error(file, line, "variable '" + tokens[0].value + "' is not defined");
		}

		std::vector<VariableType> types;
		const std::shared_ptr<Expression> expression = ParseTokenExpression(
			tokens, 3, tokens.size(), variables, functions, classes, parameters, &types
		);

		variable->types.insert(variable->types.end(), types.begin(), types.end());

		statements.push_back(Statement{
			StatementType::ASSIGNMENT,
			Assignment{ tokens[0].value, expression }
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
		const std::shared_ptr<Expression> condition = ExtractCondition(
			tokens, closeBracketIndex, variables, functions, classes, parameters, "if condition"
		);

		const std::vector<Statement> body = ExtractBody(
			tokens, closeBracketIndex, variables, "if statement"
		);

		statements.push_back(Statement{
			StatementType::CONDITIONAL,
			Conditional{ condition, body }
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
		if (statements.size() == 0 || statements.back().type != StatementType::CONDITIONAL ||
			std::get<Conditional>(statements.back().stmt).condition == nullptr)
			throw Error(file, line, "expected if or else if statement before else if statement");

		std::size_t closeBracketIndex = 3;

		const std::shared_ptr<Expression> conditionExpr = ExtractCondition(
			tokens, closeBracketIndex, variables, functions, classes, parameters, "else if condition"
		);

		const std::vector<Statement> body = ExtractBody(
			tokens, closeBracketIndex, variables, "else if statement"
		);

		std::get<Conditional>(statements.back().stmt).chains.push_back(
			Conditional{ conditionExpr, body }
		);
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::ELSE)
	{
		if (statements.size() == 0 || statements.back().type != StatementType::CONDITIONAL ||
			std::get<Conditional>(statements.back().stmt).condition == nullptr)
			throw Error(file, line, "expected if or else if statement before else statement");

		const std::vector<Statement> body = ExtractBody(
			tokens, 0, variables, "function definition"
		);

		std::get<Conditional>(statements.back().stmt).chains.push_back(
			Conditional{ nullptr, body }
		);
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::DEF)
	{
		if (inFunction)
			throw Error(file, line, "function definition found within function definition; functions cannot contain funcitons");

		if (tokens.size() == 1 || tokens[1].type != TokenType::VARIABLE)
			throw Error(file, line, "expected function name after 'def' keyword");
		if (tokens.size() == 2 || tokens[2].type != TokenType::OPEN_BRACKET)
			throw Error(file, line, "expected open bracket after function name");
		if (tokens.size() == 3)
			throw Error(file, line, "expected open bracket after function name");
		if (tokens.size() == 4 && tokens[3].type != TokenType::CLOSE_BRACKET)
			throw Error(file, line, "expected closing bracket for function parameters");
		if (tokens.size() == 4 || (tokens.size() == 5 && tokens[4].type != TokenType::OPEN_CURLY))
			throw Error(file, line, "expected open curly bracket for function body");

		if (night::find_container(variables, tokens[1].value))
			throw Error("function '" + tokens[1].value + "' has the same name as a variable; variable and function names must be unique");
		if (night::find_container(functions, tokens[1].value))
			throw Error("function '" + tokens[1].value + "' is already defined");
		if (night::find_container(classes, tokens[1].value))
			throw Error("function '" + tokens[1].value + "' has the same name as a class; function and class names must be unique");

		// function parameters

		std::vector<std::string> parameterNames;

		std::size_t closeBracketIndex = 3;
		for (; closeBracketIndex < tokens.size(); closeBracketIndex += 2)
		{
			if (tokens[closeBracketIndex].type != TokenType::VARIABLE)
				throw Error(file, line, "expected variable names as function parameters");

			if (night::find_container(variables, tokens[closeBracketIndex].value))
				throw Error(file, line, "function parameter cannot have the same name as another variable");
			if (night::find_container(functions, tokens[closeBracketIndex].value))
				throw Error(file, line, "function parameter cannot have the same name as a function");
			if (night::find_container(classes, tokens[closeBracketIndex].value))
				throw Error(file, line, "function parameter cannot have the same name as a class");
			if (night::find_container(parameters, tokens[closeBracketIndex].value))
				throw Error(file, line, "function parameters cannot have the same names");

			parameterNames.push_back(tokens[closeBracketIndex].value);
			parameters.push_back({ tokens[closeBracketIndex].value });

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

		// type checking parameters

		CheckFunction functionCheck{ tokens[1].value };

		// constructing statement

		inFunction = true;

		const FunctionDef function{
			tokens[1].value,
			parameterNames,
			ExtractBody(tokens, closeBracketIndex, variables, "function definitions")
		};

		parameters.clear();

		statements.push_back(Statement{ StatementType::FUNCTION_DEF, function });
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VARIABLE && tokens[1].type == TokenType::OPEN_BRACKET)
	{
		if (tokens.size() == 2 || (tokens.size() == 3 && tokens[2].type != TokenType::CLOSE_BRACKET))
			throw Error(file, line, "missing closing bracket");

		const CheckFunction* checkFunction = night::get_container(functions, tokens[0].value);
		if (checkFunction == nullptr)
			throw Error(file, line, "function " + tokens[0].value + "' is not defined");

		// function parameters

		std::vector<std::vector<VariableType> > argument_types;
		std::vector<std::shared_ptr<Expression> >      argument_expressions;

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
					ParseTokenExpression(tokens, start, a, variables, functions, classes, parameters, &get_argument_types)
				);
				
				argument_types.push_back(get_argument_types);

				start = a + 1;
				continue;
			}
		}

		if (argument_types.size() != checkFunction->parameters.size())
			throw Error(file, line, "function '" + tokens[0].value + "' is called with '" + std::to_string(argument_types.size()) + "' arguments but is defined with '" + std::to_string(checkFunction->parameters.size()) + "' parameters");

		for (std::size_t a = 0; a < checkFunction->parameters.size(); ++a)
		{
			for (const VariableType& parameterType : argument_types[a])
			{
				if (!night::find_type(checkFunction->parameters[a].types, parameterType))
					throw Error(file, line, "argument number '" + std::to_string(a + 1) + "' for function call '" + tokens[0].value + "' cannot be used because of a type mismatch");
			}
		}

		statements.push_back(Statement{
			StatementType::FUNCTION_CALL,
			FunctionCall{ tokens[0].value, argument_expressions }
		});
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::RETURN)
	{
		if (!inFunction)
			throw Error(file, line, "return statement is outside of a function definition; return statements can only be inside functions");
		if (tokens.size() == 1)
			throw Error(file, line, "expected expression after 'return' keyword");

		statements.push_back(Statement{
			StatementType::RETURN,
			Return{ ParseTokenExpression(tokens, 1, tokens.size(), variables, functions, classes, parameters) }
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

		const std::shared_ptr<Expression> condition(
			ExtractCondition(tokens, closeBracketIndex, variables, functions, classes, parameters, "while loop")
		);

		const std::vector<Statement> body(
			ExtractBody(tokens, closeBracketIndex, variables, "while loop")
		);

		statements.push_back(Statement{
			StatementType::WHILE_LOOP,
			WhileLoop{ condition, body }
		});
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::FOR)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::OPEN_BRACKET)
			throw Error(file, line, "expected open bracket after 'for' keyword");
		if (tokens.size() == 2 || tokens[2].type != TokenType::VARIABLE)
			throw Error(file, line, "expected iterator name after open bracket");
		if (tokens.size() == 3 || tokens[3].type != TokenType::COLON )
			throw Error(file, line, "expected colon after iterator name");
		if (tokens.size() == 4)
			throw Error(file, line, "expected array after colon");

		if (!night::find_container(variables, tokens[2].value))
			throw Error(file, line, "variable '" + tokens[2].value + "' is already defined");

		// evaluating iterator and range

		std::size_t closeBracketIndex = 4; // starts at range
		AdvanceToCloseBracket(tokens, TokenType::OPEN_BRACKET, TokenType::CLOSE_BRACKET, closeBracketIndex);
		
		if (closeBracketIndex >= tokens.size())
			throw Error(file, line, "missing closing bracket in for loop conditions");

		std::vector<VariableType> rangeTypes;
		const std::shared_ptr<Expression> rangeExpr(
			ParseTokenExpression(tokens, 4, closeBracketIndex, variables, functions, classes, parameters, &rangeTypes)
		);

		std::vector<VariableType> iteratorTypes;
		if (night::find_type(rangeTypes, VariableType::BOOL_ARR))
			iteratorTypes.push_back(VariableType::BOOL);
		if (night::find_type(rangeTypes, VariableType::MULT_BOOL_ARR))
			iteratorTypes.push_back(VariableType::BOOL_ARR);
		if (night::find_type(rangeTypes, VariableType::NUM_ARR))
			iteratorTypes.push_back(VariableType::NUM);
		if (night::find_type(rangeTypes, VariableType::MULT_NUM_ARR))
			iteratorTypes.push_back(VariableType::NUM_ARR);
		if (night::find_type(rangeTypes, VariableType::STR_ARR))
			iteratorTypes.push_back(VariableType::STR);
		if (night::find_type(rangeTypes, VariableType::MULT_STR_ARR))
			iteratorTypes.push_back(VariableType::STR_ARR);
		if (night::find_type(rangeTypes, VariableType::MULT_EMPTY_ARR))
			iteratorTypes.push_back(VariableType::EMPTY_ARR);
		if (night::find_type(rangeTypes, VariableType::EMPTY_ARR))
			throw Error(file, line, "for loop range evaluates to an empty array; for loop ranges can only be strings or non-empty arrays");

		variables.push_back(CheckVariable{ tokens[2].value, iteratorTypes });

		// extracting scope and constructing statement

		const std::vector<Statement> body(
			ExtractBody(tokens, closeBracketIndex, variables, "for loop")
		);

		variables.pop_back(); // remove the iterator variable

		statements.push_back(Statement{
			StatementType::FOR_LOOP,
			ForLoop{ tokens[2].value, rangeExpr, body }
		});
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VARIABLE && tokens[1].type == TokenType::OPEN_SQUARE)
	{
		if (tokens.size() == 2)
			throw Error(file, line, "expected index after open square");

		const CheckVariable* variable = night::get_container(variables, tokens[0].value);
		if (variable == nullptr)
			throw Error(file, line, "variable '" + tokens[1].value + "' is not defined");

		// check if variable is an array or a string
		const bool isArray = night::find_type(variable->types, VariableType::BOOL_ARR) ||
							 night::find_type(variable->types, VariableType::NUM_ARR)  ||
							 night::find_type(variable->types, VariableType::STR_ARR);

		if (!isArray && night::find_type(variable->types, VariableType::STR))
			throw Error(file, line, "variable '" + variable->name + "' doesn't contain a string or an array; to be access with an index, a variable must contain a string or an array");

		// extract and evaluate element

		std::size_t assignmentIndex = 2;
		for (; assignmentIndex < tokens.size() && tokens[assignmentIndex].type != TokenType::ASSIGNMENT; ++assignmentIndex);

		if (tokens[assignmentIndex - 1].type != TokenType::CLOSE_SQUARE)
			throw Error(file, line, "missing closing square");
		if (assignmentIndex == tokens.size())
			throw Error(file, line, "missing assignment operator");

		std::vector<VariableType> elemType;
		const std::shared_ptr<Expression> elemExpr = ParseTokenExpression(
			tokens, 2, assignmentIndex - 1, variables, functions, classes, parameters, &elemType
		);

		if (!night::find_type(elemType, VariableType::NUM))
			throw Error(file, line, "index for array '" + tokens[0].value + "' does not contain a type int");

		// extract expression

		const std::shared_ptr<Expression> assignExpr = ParseTokenExpression(
			tokens, assignmentIndex + 1, tokens.size(), variables, functions, classes, parameters
		);

		if (!isArray && (assignExpr->type != ValueType::STR || assignExpr->data.length() >= 1))
			throw Error(file, line, "a string element can only be assigned to a string of length 1");

		// construct statement
		
		statements.push_back(Statement{
			StatementType::ELEMENT,
			Element{ tokens[0].value, elemExpr, assignExpr }
		});
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VARIABLE && tokens[1].type == TokenType::OPERATOR)
	{
		if (tokens[1].value != ".")
			throw Error(file, line, "invalid syntax");
		if (tokens.size() == 2)
			throw Error(file, line, "expected method name after dot operator");

		const std::shared_ptr<Expression> object = ParseTokenExpression(
			tokens, 0, tokens.size(), variables, functions, classes, parameters
		);

		statements.push_back(Statement{
			StatementType::METHOD_CALL,
			MethodCall{ tokens[0].value, object }
		});
	}
	else
	{
		throw Error(file, line, "invalid syntax");
	}
}