#include "../include/parser.h"
#include "../include/utils.h"
#include "../include/token.h"
#include "../include/error.h"

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

std::shared_ptr<Expression> new_expression(const std::string& file, const int line, const Value& value, const std::shared_ptr<Expression>& left,
	const std::shared_ptr<Expression>& right, const std::vector<Variable>& variables, const std::vector<FunctionDef>& functions)
{
	std::shared_ptr<Expression> expression = std::make_shared<Expression>(Expression{
		value.type,
		value.data,
		std::vector<std::shared_ptr<Expression> >(),
		left,
		right,
		value.arrayDepth
	});

	for (const std::vector<Value>& values : value.extras)
		expression->extras.push_back(ParseValues(file, line, values, variables, functions));

	return expression;
}

std::vector<Value> TokensToValues(const std::vector<Token>& tokens, const std::vector<CheckVariable>& variables,
	const std::vector<CheckFunction>& functions, int* _arrayDepth)
{
	assert(!tokens.empty() && "tokens shouldn't be empty");

	const std::string file = tokens[0].file;
	const int         line = tokens[0].line;

	// convert tokens to values

	std::vector<Value> values;
	for (std::size_t a = 0; a < tokens.size(); ++a)
	{
		if (tokens[a].type == TokenType::VARIABLE)
		{
			if (a < tokens.size() - 1 && tokens[a + 1].type == TokenType::OPEN_BRACKET)
			{
				if ((a == 0 || tokens[a - 1].type != TokenType::OPERATOR || tokens[a - 1].value != ".") &&
					GetContainer(functions, tokens[a].value) == nullptr)
					throw Error(file, line, "function '" + tokens[a].value + "' is not defined");

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
							std::vector<Token>(tokens.begin() + start, tokens.begin() + a),
							variables, functions
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
				values.push_back(Value{ ValueType::VARIABLE, tokens[a].value, std::vector<std::vector<Value> >(), 0 });

				while (a < tokens.size() - 1 && tokens[a + 1].type == TokenType::OPEN_SQUARE)
				{
					a += 2;
					const std::size_t start = a;

					AdvanceCloseBracketIndex(tokens, TokenType::OPEN_SQUARE, TokenType::CLOSE_SQUARE, a);
					if (a >= tokens.size())
						throw Error(file, line, "missing closing square bracket for subscript operator");

					Value subscript{ ValueType::OPERATOR, "[]" };
					subscript.extras.push_back(TokensToValues(
						std::vector<Token>(tokens.begin() + start, tokens.begin() + a),
						variables, functions
					));

					values.push_back(subscript);
				}
			}
			else
			{
				if (GetContainer(variables, tokens[a].value) == nullptr)
					throw Error(file, line, "variable '" + tokens[a].value + "' is not defined");

				values.push_back(Value{ ValueType::VARIABLE, tokens[a].value, std::vector<std::vector<Value> >(), 0 });
			}
		}
		else if (tokens[a].type == TokenType::OPEN_SQUARE)
		{
			if (_arrayDepth != nullptr)
				(*_arrayDepth)++;

			Value array;

			a++;

			if (tokens[a].type == TokenType::CLOSE_SQUARE)
			{
				values.push_back(Value{ ValueType::EMPTY_ARRAY });
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
						std::vector<Token>(tokens.begin() + start, tokens.begin() + a),
						variables, functions,
						&arrayDepth
					));

					if (array.extras.size() == 1)
					{
						array.arrayDepth = arrayDepth;
						array.type = TypeCheckExpression(
							file, line,
							ParseValues(file, line, array.extras.back(), variables, functions),
							variables, functions
						).type;

						switch (array.type)
						{
						case ValueType::BOOL:
							array.type = ValueType::BOOL_ARR;
							break;
						case ValueType::NUM:
							array.type = ValueType::NUM_ARR;
							break;
						case ValueType::STRING:
							array.type = ValueType::STRING_ARR;
							break;
						default:
							break;
						}
					}
					else
					{
						if (arrayDepth != array.arrayDepth)
							throw Error(file, line, std::to_string(arrayDepth + 1) + "D array cannot be in the same array as a " + std::to_string(array.arrayDepth + 1) + "D array");

						ValueType elemType = TypeCheckExpression(
							file, line,
							ParseValues(file, line, array.extras.back(), variables, functions),
							variables, functions
						).type;

						switch (elemType)
						{
						case ValueType::BOOL:
							elemType = ValueType::BOOL_ARR;
							break;
						case ValueType::NUM:
							elemType = ValueType::NUM_ARR;
							break;
						case ValueType::STRING:
							elemType = ValueType::STRING_ARR;
							break;
						default:
							break;
						}

						if (elemType != array.type)
							throw Error(file, line, "elements in array must have the same type");
					}

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
				values.push_back(Value{ ValueType::BOOL, tokens[a].value, std::vector<std::vector<Value> >(), 0 });
				break;
			case TokenType::NUM_VAL:
				values.push_back(Value{ ValueType::NUM, tokens[a].value, std::vector<std::vector<Value> >(), 0 });
				break;
			case TokenType::STRING_VAL:
				values.push_back(Value{ ValueType::STRING, tokens[a].value, std::vector<std::vector<Value> >(), 0 });
				break;
			case TokenType::OPEN_BRACKET:
				values.push_back(Value{ ValueType::OPEN_BRACKET, tokens[a].value, std::vector<std::vector<Value> >(), 0 });
				break;
			case TokenType::CLOSE_BRACKET:
				values.push_back(Value{ ValueType::CLOSE_BRACKET, tokens[a].value, std::vector<std::vector<Value> >(), 0 });
				break;
			case TokenType::OPERATOR:
				values.push_back(Value{ ValueType::OPERATOR, tokens[a].value, std::vector<std::vector<Value> >(), 0 });
				break;
			default:
				throw Error(file, line, "unexpected token '" + tokens[a].value + "' in expression");
			}
		}
	}

	return values;
}

std::vector<VariableType> TypeCheckExpression(const std::string& file, const int line, const std::shared_ptr<Expression>& node,
	const std::vector<CheckVariable>& variables, const std::vector<CheckFunction>& functions, std::vector<CheckVariable>& parameters)
{
	// if node->left and node->right is NULL, then node must be a value
	if (node->left == nullptr && node->right == nullptr)
	{
		if (node->type == ValueType::VARIABLE)
		{
			const CheckVariable* checkVariable = GetContainer(variables, node->data);
			if (checkVariable == nullptr)
			{
				checkVariable = GetContainer(parameters, node->data);
				assert(checkVariable != nullptr && "variable should already be defined; check definitions in 'TokensToValue()'");
			}

			return checkVariable->types;
		}
		if (node->type == ValueType::CALL)
		{
			if (node->data == "input")
				return { VariableType::STR };

			const CheckFunction* function = GetContainer(functions, node->data);
			assert(function != nullptr && "function should already be defined; check definitions in 'TokensToValue()'");

			if (node->extras.size() != function->parameters.size())
				throw Error(file, line, "function '" + node->data + "' was called with '" + std::to_string(node->extras.size()) + "' parameters, but was defined with '" + std::to_string(function->parameters.size()) + "' parameters");

			// type check function parameters in the function body here

			if (function->isVoid)
				throw Error(file, line, "function '" + node->data + "' doesn't return a value; functions used in expressions must return a value");

			return function->returnValues;
		}

		// if node isn't a variable or a function call,
		// then node must be a literal value

		switch (node->type)
		{
		case ValueType::BOOL:
			return { VariableType::BOOL };
		case ValueType::BOOL_ARR:
			return { VariableType::BOOL_ARR };
		case ValueType::NUM:
			return { VariableType::NUM };
		case ValueType::NUM_ARR:
			return { VariableType::NUM_ARR };
		case ValueType::STRING:
			return { VariableType::STR };
		case ValueType::STRING_ARR:
			return { VariableType::STR_ARR };
		case ValueType::EMPTY_ARRAY:
			return { VariableType::EMPTY_ARR };
		default:
			throw Error(file, line, "unexpected token '" + node->data + "' in expression");
		}
	}

	assert(node->type == ValueType::OPERATOR && "node must be an operator");

	if (node->data == "+")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM, VariableType::STR };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find(parameter->types, VariableType::NUM) && !night::find(parameter->types, VariableType::STR))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with '+'");
			}
		}
		if (node->right->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->right->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM, VariableType::STR };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find(parameter->types, VariableType::NUM) && !night::find(parameter->types, VariableType::STR))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with '+'");
			}
		}

		const std::vector<VariableType> left  = TypeCheckExpression(file, line, node->left,  variables, functions, parameters);
		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, parameters);

		std::vector<VariableType> returnTypes;

		if (night::find(left, VariableType::NUM) && night::find(right, VariableType::NUM))
			returnTypes.push_back(VariableType::NUM);
		if (night::find(left, VariableType::STR) || night::find(right, VariableType::STR))
			returnTypes.push_back(VariableType::STR);

		if (!returnTypes.empty())
			throw Error(file, line, "binary operator '+' can only be used on two numbers or in string concatenation");

		return returnTypes;
	}
	if (node->data == "-")
	{
		if (node->right->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->right->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '-'");
			}
		}

		if (node->left == nullptr)
		{
			const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, parameters);

			if (!night::find(right, VariableType::NUM))
				throw Error(file, line, "unary operator '-' can only be used on numbers");

			return { VariableType::NUM };
		}

		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '-'");
			}
		}

		const std::vector<VariableType> left  = TypeCheckExpression(file, line, node->left,  variables, functions, parameters);
		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, parameters);

		if (!night::find(left, VariableType::NUM) || !night::find(right, VariableType::NUM))
			throw Error(file, line, "binary operator '-' can only be used on two numbers");
		
		return { VariableType::NUM };
	}
	if (node->data == "*")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '*'");
			}
		}
		if (node->right->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->right->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '*'");
			}
		}

		const std::vector<VariableType> left  = TypeCheckExpression(file, line, node->left,  variables, functions, parameters);
		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, parameters);

		if (!night::find(left, VariableType::NUM) || !night::find(right, VariableType::NUM))
			throw Error(file, line, "binary operator '*' can only be used on two numbers");
		
		return { VariableType::NUM };
	}
	if (node->data == "/")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '/'");
			}
		}
		if (node->right->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->right->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '/'");
			}
		}

		const std::vector<VariableType> left  = TypeCheckExpression(file, line, node->left,  variables, functions, parameters);
		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, parameters);

		if (!night::find(left, VariableType::NUM) || !night::find(right, VariableType::NUM))
			throw Error(file, line, "binary operator '/' can only be used on two numbers");
		
		return { VariableType::NUM };
	}
	if (node->data == "%")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '%'");
			}
		}
		if (node->right->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->right->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '%'");
			}
		}

		const std::vector<VariableType> left  = TypeCheckExpression(file, line, node->left,  variables, functions, parameters);
		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, parameters);

		if (!night::find(left, VariableType::NUM) || !night::find(right, VariableType::NUM))
			throw Error(file, line, "binary operator '%' can only be used on two numbers");

		return { VariableType::NUM };
	}
	if (node->data == ">")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '>'");
			}
		}
		if (node->right->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->right->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '>'");
			}
		}

		const std::vector<VariableType> left  = TypeCheckExpression(file, line, node->left,  variables, functions, parameters);
		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, parameters);

		if (!night::find(left, VariableType::NUM) || !night::find(right, VariableType::NUM))
			throw Error(file, line, "binary operator '>' can only be used on two numbers");
		
		return { VariableType::BOOL };
	}
	if (node->data == "<")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '<'");
			}
		}
		if (node->right->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->right->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '<'");
			}
		}

		const std::vector<VariableType> left  = TypeCheckExpression(file, line, node->left,  variables, functions, parameters);
		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, parameters);

		if (!night::find(left, VariableType::NUM) || !night::find(right, VariableType::NUM))
			throw Error(file, line, "binary operator '<' can only be used on two numbers");
		
		return { VariableType::BOOL };
	}
	if (node->data == ">=")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '>='");
			}
		}
		if (node->right->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->right->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '>='");
			}
		}

		const std::vector<VariableType> left  = TypeCheckExpression(file, line, node->left,  variables, functions, parameters);
		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, parameters);

		if (!night::find(left, VariableType::NUM) || !night::find(right, VariableType::NUM))
			throw Error(file, line, "binary operator '>=' can only be used on two numbers");
		
		return { VariableType::BOOL };
	}
	if (node->data == "<=")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '<='");
			}
		}
		if (node->right->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->right->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::NUM };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find(parameter->types, VariableType::NUM))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '<='");
			}
		}

		const std::vector<VariableType> left  = TypeCheckExpression(file, line, node->left,  variables, functions, parameters);
		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, parameters);

		if (!night::find(left, VariableType::NUM) || !night::find(right, VariableType::NUM))
			throw Error(file, line, "binary operator '<=' can only be used on two numbers");
		
		return { VariableType::BOOL };
	}
	if (node->data == "!")
	{
		if (node->right->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->right->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::BOOL };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find(parameter->types, VariableType::BOOL))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '!'");
			}
		}

		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, parameters);

		if (!night::find(right, VariableType::BOOL))
			throw Error(file, line, "unary operator '!' can only be used on booleans");

		return { VariableType::BOOL };
	}
	if (node->data == "||")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::BOOL };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find(parameter->types, VariableType::BOOL))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '||'");
			}
		}
		if (node->right->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->right->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::BOOL };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find(parameter->types, VariableType::BOOL))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '||'");
			}
		}

		const std::vector<VariableType> left  = TypeCheckExpression(file, line, node->left,  variables, functions, parameters);
		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, parameters);

		if (!night::find(left, VariableType::BOOL) || !night::find(right, VariableType::BOOL))
			throw Error(file, line, "binary operator '||' can only be used on two numbers");

		return { VariableType::BOOL };
	}
	if (node->data == "&&")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::BOOL };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find(parameter->types, VariableType::BOOL))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '&&'");
			}
		}
		if (node->right->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->right->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = { VariableType::BOOL };
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find(parameter->types, VariableType::BOOL))
					throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with operator '&&'");
			}
		}

		const std::vector<VariableType> left  = TypeCheckExpression(file, line, node->left,  variables, functions, parameters);
		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, parameters);

		if (!night::find(left, VariableType::BOOL) || !night::find(right, VariableType::BOOL))
			throw Error(file, line, "binary operator '&&' can only be used on two numbers");

		return { VariableType::BOOL };
	}
	if (node->data == "==")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->left->data);
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
			CheckVariable* parameter = GetContainer(parameters, node->right->data);
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

		const std::vector<VariableType> left  = TypeCheckExpression(file, line, node->left,  variables, functions, parameters);
		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, parameters);

		for (const VariableType& type : left)
		{
			if (night::find(right, type))
				return { VariableType::BOOL };
		}

		throw Error(file, line, "binary operator '==' can only compare values of the same type");
	}
	if (node->data == "!=")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->left->data);
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
			CheckVariable* parameter = GetContainer(parameters, node->right->data);
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

		const std::vector<VariableType> left  = TypeCheckExpression(file, line, node->left,  variables, functions, parameters);
		const std::vector<VariableType> right = TypeCheckExpression(file, line, node->right, variables, functions, parameters);

		for (const VariableType& type : left)
		{
			if (night::find(right, type))
				return { VariableType::BOOL };
		}

		throw Error(file, line, "binary operator '!=' can only compare values of the same type");
	}
	if (node->data == "[]")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				parameter->types = {
					VariableType::BOOL_ARR,
					VariableType::NUM_ARR,
					VariableType::STR_ARR,
					VariableType::EMPTY_ARR
				};
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (!night::find(parameter->types, VariableType::BOOL_ARR) && /*!night::find(array, VariableType::MULT_BOOL_ARR) &&*/
					!night::find(parameter->types, VariableType::NUM_ARR) && /*!night::find(array, VariableType::MULT_NUM_ARR) &&*/
					!night::find(parameter->types, VariableType::STR_ARR) && /*!night::find(array, VariableType::MULT_STR_ARR) &&*/
					!night::find(parameter->types, VariableType::EMPTY_ARR) && /*!night::find(array, VariableType::MULT_EMPTY_ARR) &&*/
					!night::find(parameter->types, VariableType::STR))
					throw Error(file, line, "parameter " + parameter->name + " cannot be used with operator '[]'");
			}
		}

		const std::vector<VariableType> array = TypeCheckExpression(file, line, node->right,     variables, functions, parameters);
		const std::vector<VariableType> index = TypeCheckExpression(file, line, node->extras[0], variables, functions, parameters);

		if (!night::find(array, VariableType::BOOL_ARR) && /*!night::find(array, VariableType::MULT_BOOL_ARR) &&*/
			!night::find(array, VariableType::NUM_ARR) && /*!night::find(array, VariableType::MULT_NUM_ARR) &&*/
			!night::find(array, VariableType::STR_ARR) && /*!night::find(array, VariableType::MULT_STR_ARR) &&*/
			!night::find(array, VariableType::EMPTY_ARR) && /*!night::find(array, VariableType::MULT_EMPTY_ARR) &&*/
			!night::find(array, VariableType::STR))
			throw Error(file, line, "subscript operator can only be used on strings or arrays");
		if (!night::find(index, VariableType::NUM))
			throw Error(file, line, "subscript operator has to be a number");

		return array;
	}
	if (node->data == ".")
	{
		if (node->left->type == ValueType::VARIABLE)
		{
			CheckVariable* parameter = GetContainer(parameters, node->left->data);
			if (parameter != nullptr && parameter->types.empty())
			{
				if (node->right->data == "len")
				{
					parameter->types = {
						VariableType::BOOL_ARR,
						VariableType::NUM_ARR,
						VariableType::STR_ARR,
						VariableType::EMPTY_ARR,
						VariableType::STR
					};
				}
				else if (node->right->data == "push" || node->right->data == "pop")
				{
					parameter->types = {
						VariableType::BOOL_ARR,
						VariableType::NUM_ARR,
						VariableType::STR_ARR,
						VariableType::EMPTY_ARR
					};
				}
			}
			else if (parameter != nullptr && !parameter->types.empty())
			{
				if (node->right->data == "len" || node->right->data == "push" || node->right->data == "pop")
				{
					if (!night::find(parameter->types, VariableType::BOOL_ARR) && /*!night::find(array, VariableType::MULT_BOOL_ARR) &&*/
						!night::find(parameter->types, VariableType::NUM_ARR) && /*!night::find(array, VariableType::MULT_NUM_ARR) &&*/
						!night::find(parameter->types, VariableType::STR_ARR) && /*!night::find(array, VariableType::MULT_STR_ARR) &&*/
						!night::find(parameter->types, VariableType::EMPTY_ARR) && /*!night::find(array, VariableType::MULT_EMPTY_ARR) &&*/
						!night::find(parameter->types, VariableType::STR))
						throw Error(file, line, "parameter '" + parameter->name + "' cannot be used with method '" + node->right->data + "()'");
				}
			}
		}

		const std::vector<VariableType> object = TypeCheckExpression(file, line, node->left,  variables, functions, parameters);

		if (node->right->data == "len" &&
			(night::find(object, VariableType::BOOL_ARR) || /*!night::find(array, VariableType::MULT_BOOL_ARR) &&*/
			night::find(object, VariableType::NUM_ARR) || /*!night::find(array, VariableType::MULT_NUM_ARR) &&*/
			night::find(object, VariableType::STR_ARR) || /*!night::find(array, VariableType::MULT_STR_ARR) &&*/
			night::find(object, VariableType::EMPTY_ARR) ||
			night::find(object, VariableType::STR)))
			return { VariableType::NUM };
		if (node->right->data == "pop")
			return object;
		if (node->right->data == "push")
		{
			std::vector<VariableType> returnTypes;
			const std::vector<VariableType> insertElemType = TypeCheckExpression(file, line, node->right->extras[0], variables, functions, parameters);
			if (night::find(object, VariableType::BOOL_ARR) && night::find(insertElemType, VariableType::BOOL))
				returnTypes.push_back(VariableType::BOOL_ARR);
			if (night::find(object, VariableType::NUM_ARR) && night::find(insertElemType, VariableType::NUM))
				returnTypes.push_back(VariableType::NUM_ARR);
			if (night::find(object, VariableType::STR_ARR) && night::find(insertElemType, VariableType::STR))
				returnTypes.push_back(VariableType::STR_ARR);

			return returnTypes;
		}
	}

	assert_rtn(false && ("operator '" + node->data + "' missing from list").c_str(), { (VariableType)0 });
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

std::shared_ptr<Expression> GetNextGroup(const std::string& file, const int line, const std::vector<Value>& values, std::size_t& index,
	const std::vector<Variable>& variables, const std::vector<FunctionDef>& functions)
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
				groupExpression = ParseValues(
					file, line,
					std::vector<Value>(values.begin() + start + 1, values.begin() + index - (index == values.size() - 1 ? 0 : 1)),
					variables, functions
				);
			}
			else
			{
				groupExpression = new_expression(file, line, values[start], nullptr, nullptr, variables, functions);
			}

			while (index < values.size() && values[index].type == ValueType::OPERATOR && values[index].data == "[]")
				groupExpression = new_expression(file, line, values[index++], nullptr, groupExpression, variables, functions);

			return isFrontUnary
				? new_expression(file, line, values[start - 1], nullptr, groupExpression, variables, functions)
				: groupExpression;
		}
	}

	throw Error(file, line, "missing closing bracket in expression");
}

std::shared_ptr<Expression> ParseValues(const std::string& file, const int line, const std::vector<Value>& values,
	const std::vector<Variable>& variables, const std::vector<FunctionDef>& functions)
{
	assert(!values.empty() && "values shouldn't be empty");

	std::size_t a = 0;

	std::shared_ptr<Expression> root = GetNextGroup(file, line, values, a, variables, functions);
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

		const std::shared_ptr<Expression> nextValue = GetNextGroup(file, line, values, ++a, variables, functions);
		const std::shared_ptr<Expression> opNode = new_expression(file, line, values[opIndex], curr, nextValue, variables, functions);

		protect = nextValue.get();

		if (curr == root)
			root = opNode;
		else
			prev->right = opNode;
	}

	return root;
}

void Parser(std::vector<Statement>& statements, const std::vector<Token>& tokens, bool inFunction)
{
	static std::vector<CheckVariable> variables;
	static std::vector<CheckFunction> functions;

	static bool defineOnce = true;
	if (defineOnce)
	{
		functions.push_back(CheckFunction{ "print", {}, true });
		functions.push_back(CheckFunction{ "input", {}, false, { VariableType::STR } });

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

		if (GetContainer(variables, tokens[1].value) != nullptr)
			throw Error(file, line, "variable '" + tokens[1].value + "' is already defined");
		if (GetContainer(functions, tokens[1].value) != nullptr)
			throw Error(file, line, "variable '" + tokens[1].value + "' has the same name as a function; variable and function names must be unique");

		const std::vector<Value> values = TokensToValues(
			std::vector<Token>(tokens.begin() + 3, tokens.end()),
			variables, functions
		);

		const std::shared_ptr<Expression> expression = ParseValues(tokens[0].file, tokens[0].line, values, variables, functions);
		const VariableType type = TypeCheckExpression(tokens[0].file, tokens[0].line, expression, variables, functions, {});

		variables.push_back(CheckVariable{ tokens[1].value, { type } });

		statements.push_back(Statement{
			StatementType::VARIABLE,
			Variable{ type, tokens[1].value, expression }
		});
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VARIABLE && tokens[1].type == TokenType::ASSIGNMENT)
	{
		if (tokens.size() == 2)
			throw Error(file, line, "expected expression after assignment operator");

		CheckVariable* variable = GetContainer(variables, tokens[0].value);
		if (variable == nullptr)
			throw Error(file, line, "variable '" + tokens[0].value + "' is not defined");

		VariableType type;
		const std::shared_ptr<Expression> expression(
			ExtractExpression(tokens, 2, tokens.size(), variables, functions, &type)
		);

		variable->types.push_back(type);

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

		const std::shared_ptr<Expression> condition(
			ExtractCondition(tokens, closeBracketIndex, variables, functions, "if condition", {})
		);

		const std::vector<Statement> body(
			ExtractBody(tokens, closeBracketIndex, variables, "if statement")
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

		const std::shared_ptr<Expression> conditionExpr(
			ExtractCondition(tokens, closeBracketIndex, variables, functions, "else if condition")
		);

		const std::vector<Statement> body(
			ExtractBody(tokens, closeBracketIndex, variables, "else if statement")
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

		const std::vector<Statement> body(
			ExtractBody(tokens, 0, variables, "function definition")
		);

		std::get<Conditional>(statements.back().stmt).chains.push_back(
			Conditional{ nullptr, body }
		);
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::DEF)
	{
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

		if (GetContainer(variables, tokens[1].value) != nullptr)
			throw Error("function '" + tokens[1].value + "' has the same name as a variable; variable and function names must be unique");
		if (GetContainer(functions, tokens[1].value) != nullptr)
			throw Error("function '" + tokens[1].value + "' is already defined");

		std::vector<std::string> parameters;

		// function parameters

		std::size_t closeBracketIndex = 3;
		for (; closeBracketIndex < tokens.size(); closeBracketIndex += 2)
		{
			if (tokens[closeBracketIndex].type != TokenType::VARIABLE)
				throw Error(file, line, "expected variable names as function parameters");

			parameters.push_back(tokens[closeBracketIndex].value);

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

		// type checking paramaters

		CheckFunction functionCheck{ tokens[1].value };

		// constructing statement

		const FunctionDef function{
			tokens[1].value,
			parameters,
			ExtractBody(tokens, closeBracketIndex, variables, "function definitions", true)
		};

		statements.push_back(Statement{ StatementType::FUNCTION_DEF, function });
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VARIABLE && tokens[1].type == TokenType::OPEN_BRACKET)
	{
		if (tokens.size() == 2 || (tokens.size() == 3 && tokens[2].type != TokenType::CLOSE_BRACKET))
			throw Error(file, line, "missing closing bracket");

		const CheckFunction* checkFunction = GetContainer(functions, tokens[0].value);
		if (checkFunction == nullptr)
			throw Error(file, line, "function " + tokens[0].value + "' is not defined");

		// function parameters

		std::vector<VariableType> parameterTypes;
		std::vector<std::shared_ptr<Expression> > parameters;
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

				VariableType paramterType;
				parameters.push_back(ExtractExpression(tokens, start, a, variables, functions, &paramterType));
				parameterTypes.push_back(paramterType);

				start = a + 1;
				continue;
			}
		}

		//
		//
		// all that's left is find parameter types for function definitions,
		// and then we should be done!!!!!!
		//

		if (parameterTypes.size() != checkFunction->parameters.size())
			throw Error(file, line, "function '" + tokens[0].value + "' is called with '" + std::to_string(parameterTypes.size()) + "' arguments but is defined with '" + std::to_string(checkFunction->parameters.size()) + "' parameters");

		for (std::size_t a = 0; a < parameterTypes.size(); ++a)
		{
			if (!night::find(checkFunction->parameters[a].types, parameterTypes[a]))
				throw Error(file, line, "argument number '" + std::to_string(a + 1) + "' for function call '" + tokens[0].value + "' cannot be used because of a type mismatch");
		}

		statements.push_back(Statement{
			StatementType::FUNCTION_CALL,
			FunctionCall{ tokens[0].value, parameters }
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
			Return{ ExtractExpression(tokens, 1, tokens.size(), variables, functions) }
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
			ExtractCondition(tokens, closeBracketIndex, variables, functions, "while loop")
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

		if (GetContainer(variables, tokens[2].value) != nullptr)
			throw Error(file, line, "variable '" + tokens[2].value + "' is already defined");

		// evaluating iterator and range

		std::size_t closeBracketIndex = 4; // starts at range
		AdvanceCloseBracketIndex(tokens, TokenType::OPEN_BRACKET, TokenType::CLOSE_BRACKET, closeBracketIndex);
		
		if (closeBracketIndex >= tokens.size())
			throw Error(file, line, "missing closing bracket in for loop conditions");

		VariableType rangeType;
		const std::shared_ptr<Expression> rangeExpr(
			ExtractExpression(tokens, 4, closeBracketIndex, variables, functions, &rangeType)
		);

		if (rangeType != VariableType::BOOL_ARR   &&
			rangeType != VariableType::NUM_ARR    &&
			rangeType != VariableType::STR_ARR &&
			rangeType != VariableType::STR)
			throw Error(file, line, "for loop range evaluated to a " + night::ttos(rangeType) + "; ranges must evaluate to a string or an array");

		switch (rangeType)
		{
		case VariableType::BOOL_ARR:
			rangeType = VariableType::BOOL;
			break;
		case VariableType::NUM_ARR:
			rangeType = VariableType::NUM;
			break;
		case VariableType::STR_ARR:
			rangeType = VariableType::STR;
			break;
		default:
			break;
		}

		variables.push_back(Variable{ rangeType, tokens[2].value, rangeExpr });

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

		const Variable* variable = GetContainer(variables, tokens[0].value);
		if (variable == nullptr)
			throw Error(file, line, "variable '" + tokens[1].value + "' is not defined");

		// check if variable is an array or a string
		const bool isArray = variable->type == VariableType::BOOL_ARR || variable->type == VariableType::NUM_ARR ||
							 variable->type == VariableType::STR_ARR;

		if (!isArray && variable->value->type != ValueType::STRING)
			throw Error(file, line, "variable '" + variable->name + "' doesn't contain a string or an array; to be access with an index, a variable must contain a string or an array");

		// extract and evaluate element

		std::size_t assignmentIndex = 2;
		for (; assignmentIndex < tokens.size() && tokens[assignmentIndex].type != TokenType::ASSIGNMENT; ++assignmentIndex);

		if (tokens[assignmentIndex - 1].type != TokenType::CLOSE_SQUARE)
			throw Error(file, line, "missing closing square");
		if (assignmentIndex == tokens.size())
			throw Error(file, line, "missing assignment operator");

		VariableType elemType;
		const std::shared_ptr<Expression> elemExpr(
			ExtractExpression(tokens, 2, assignmentIndex - 1, variables, functions, &elemType)
		);

		if (elemType != VariableType::NUM)
			throw Error(file, line, "index evaluates to a '" + night::ttos(elemType) + "'; indices must evaluate to a number");

		// check if index is out of bounds for string or array length

		if (!isArray && std::stoul(elemExpr->data) >= variable->value->data.length())
			throw Error(file, line, "index '" + elemExpr->data + "' is out of bounds for string of size '" + std::to_string(variable->value->extras.size()) + "'");
		if (isArray && std::stoul(elemExpr->data) >= variable->value->extras.size())
			throw Error(file, line, "index '" + elemExpr->data + "' is out of bounds for array of size '" + std::to_string(variable->value->extras.size()) + "'");

		// extract expression

		const std::shared_ptr<Expression> assignExpr(
			ExtractExpression(tokens, assignmentIndex + 1, tokens.size(), variables, functions)
		);

		if (!isArray && (assignExpr->type != ValueType::STRING || assignExpr->data.length() >= 1))
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

		const std::shared_ptr<Expression> object(
			ExtractExpression(tokens, 0, tokens.size(), variables, functions)
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