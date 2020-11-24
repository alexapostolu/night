#include "../include/parser.h"
#include "../include/utils.h"
#include "../include/token.h"
#include "../include/error.h"

#include <memory>
#include <string>
#include <vector>

std::shared_ptr<Expression> new_expression(const std::string& file, const int line, const Value& value, const std::shared_ptr<Expression>& left,
	const std::shared_ptr<Expression>& right, const std::vector<Variable>& variables, const std::vector<FunctionDef>& functions)
{
	std::shared_ptr<Expression> expression = std::make_shared<Expression>(Expression{
		value.type,
		value.data,
		std::vector<std::shared_ptr<Expression> >(),
		left,
		right
	});

	for (const std::vector<Value>& values : value.extras)
		expression->extras.push_back(ParseValues(file, line, values, variables, functions));

	return expression;
}

std::vector<Value> TokensToValues(const std::vector<Token>& tokens, const std::vector<Variable>& variables,
	const std::vector<FunctionDef>& functions)
{
	assert(!tokens.empty() && "tokens shouldn't be empty");

	const std::string file = tokens[0].file;
	const int line = tokens[0].line;

	// convert tokens to values

	std::vector<Value> values;
	for (std::size_t a = 0; a < tokens.size(); ++a)
	{
		if (tokens[a].type == TokenType::VARIABLE)
		{
			if (a < tokens.size() - 1 && tokens[a + 1].type == TokenType::OPEN_BRACKET)
			{
				if (GetContainer(functions, tokens[a].value) == nullptr)
					throw Error(file, line, "function '" + tokens[a].value + "' is not defined");

				Value functionCall{ ValueType::CALL, tokens[a].value };

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
				values.push_back(Value{ ValueType::VARIABLE, tokens[a].value });

				a += 2;
				std::size_t start = a;

				AdvanceCloseBracketIndex(file, line, tokens, TokenType::OPEN_SQUARE, TokenType::CLOSE_SQUARE, a);

				if (a >= tokens.size())
					throw Error(file, line, "missing closing square bracket for subscript operator");

				Value subscript{ ValueType::OPERATOR, "[]" };
				subscript.extras.push_back(TokensToValues(
					std::vector<Token>(tokens.begin() + start, tokens.begin() + a),
					variables, functions
				));

				values.push_back(subscript);
			}
			else
			{
				if (GetContainer(variables, tokens[a].value) == nullptr)
					throw Error(file, line, "variable '" + tokens[a].value + "' is not defined");

				values.push_back(Value{ ValueType::VARIABLE, tokens[a].value });
			}
		}
		else if (tokens[a].type == TokenType::OPEN_SQUARE || (a < tokens.size() - 1 &&
			tokens[a].type == TokenType::NUM_VAL && tokens[a + 1].type == TokenType::OPEN_SQUARE))
		{
			Value array;

			if (tokens[a].type == TokenType::OPEN_SQUARE)
				a++;
			else
				a += 2;

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

				if (openBracketCount == 0 && ((tokens[a].type == TokenType::COMMA && openBracketCount == 0) ||
					(tokens[a].type == TokenType::CLOSE_SQUARE && openSquareCount == -1)))
				{
					array.extras.push_back(TokensToValues(
						std::vector<Token>(tokens.begin() + start, tokens.begin() + a),
						variables, functions
					));

					const VariableType arrayElemType = VarTypeToArrType(TypeCheckExpression(
						file, line,
						ParseValues(file, line, array.extras.back(), variables, functions),
						variables, functions
					));

					if (arrayElemType == VariableType::NUM_ARR)
					{
						for (const Value& value : array.extras.back())
						{
							if (value.type == ValueType::OPERATOR && value.data == "..")
								return TokensToValues(std::vector<Token>(tokens.begin() + 1, tokens.end() - 1), variables, functions);
						}
					}

					// array ranges

					if (array.extras.size() == 1)
						array.type = VarTypeToValueType(arrayElemType);
					else if (array.type != VarTypeToValueType(arrayElemType))
						throw Error(file, line, "elements in an array must be of the same type");

					start = a + 1;

					if (tokens[a].type == TokenType::CLOSE_SQUARE)
						break;

					continue;
				}
			}

			if (a >= tokens.size())
				throw Error(file, line, "missing closing square for array");

			// fill array with default values if size initialized
			if (tokens[a].type == TokenType::NUM_VAL)
			{
				std::size_t arraySize = std::stol(tokens[a].value);
				if (array.extras.size() > arraySize)
					throw Error(file, line, "array of size '" + std::to_string(arraySize) + "' cannot hold '" + std::to_string(array.extras.size()) + "' elements");

				for (std::size_t b = array.extras.size(); b <= arraySize; ++b)
				{
					// default values for 2D arrays?
					array.extras.push_back(std::vector<Value>{
						Value{ array.type, DefaultValue(array.type) }
					});
				}
			}

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
			case TokenType::STRING_VAL:
				values.push_back(Value{ ValueType::STRING, tokens[a].value });
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

VariableType TypeCheckExpression(const std::string& file, const int line, const std::shared_ptr<Expression>& node,
	const std::vector<Variable>& variables, const std::vector<FunctionDef>& functions)
{
	// if node->left and node->right is NULL, then node must be a value
	if (node->left == nullptr && node->right == nullptr)
	{
		if (node->type == ValueType::VARIABLE)
		{
			const Variable* variable = GetContainer(variables, node->data);
			assert(variable != nullptr && "variable should already be defined; check definitions in 'TokensToValue()'");

			return variable->type;
		}
		if (node->type == ValueType::CALL)
		{
			if (node->data == "input")
				return VariableType::STRING;

			const FunctionDef* function = GetContainer(functions, node->data);
			assert(function != nullptr && "function should already be defined; check definitions in 'TokensToValue()'");

			if (node->extras.size() != function->parameters.size())
				throw Error(file, line, "function '" + node->data + "' was called with '" + std::to_string(node->extras.size()) + "' parameters, but was defined with '" + std::to_string(function->parameters.size()) + "' parameters");

			// type check function parameters in the function body here

			for (const Statement& statement : function->body)
			{
				if (statement.type == StatementType::RETURN)
					return TypeCheckExpression(file, line, std::get<Return>(statement.stmt).expression, variables, functions);
			}

			throw Error(file, line, "function '" + node->data + "' doesn't return a value; functions used in expressions must return a value");
		}

		// if node isn't a variable or a function call,
		// then node must be a literal value

		if (node->type != ValueType::BOOL && node->type != ValueType::BOOL_ARR && node->type != ValueType::NUM &&
			node->type != ValueType::NUM_ARR && node->type != ValueType::STRING && node->type != ValueType::STRING_ARR)
			throw Error(file, line, "unexpected '" + node->data + "' in expression");

		return ValueTypeToVarType(node->type);
	}

	assert(node->type == ValueType::OPERATOR && "node must be an operator");

	if (node->data == "+")
	{
		const VariableType type1 = TypeCheckExpression(file, line, node->left, variables, functions);
		const VariableType type2 = TypeCheckExpression(file, line, node->right, variables, functions);

		if (type1 != VariableType::NUM || type2 != VariableType::NUM &&
			(type1 != VariableType::STRING && type2 != VariableType::STRING))
			throw Error(file, line, "operator '+' can only be used to add two numbers, or in string concatenation");

		if (type1 == VariableType::STRING || type2 == VariableType::STRING)
			return VariableType::STRING;
		else
			return VariableType::NUM;
	}
	if (node->data == "-")
	{
		if (node->left == nullptr)
		{
			const VariableType type = TypeCheckExpression(file, line, node->right, variables, functions);
			if (type != VariableType::NUM)
				throw Error(file, line, "operator '-' is used on a " + VarTypeToStr(type) + "; operator '-' can only be used on numbers");
		}

		CHECK_EXPR(VariableType::NUM, VariableType::NUM);
	}
	if (node->data == "*")
	{
		CHECK_EXPR(VariableType::NUM, VariableType::NUM);
	}
	if (node->data == "/")
	{
		CHECK_EXPR(VariableType::NUM, VariableType::NUM);
	}
	if (node->data == "%")
	{
		CHECK_EXPR(VariableType::NUM, VariableType::NUM);
	}
	if (node->data == ">")
	{
		CHECK_EXPR(VariableType::NUM, VariableType::BOOL);
	}
	if (node->data == "<")
	{
		CHECK_EXPR(VariableType::NUM, VariableType::BOOL);
	}
	if (node->data == ">=")
	{
		CHECK_EXPR(VariableType::NUM, VariableType::BOOL);
	}
	if (node->data == "<=")
	{
		CHECK_EXPR(VariableType::NUM, VariableType::BOOL);
	}
	if (node->data == "!")
	{
		const VariableType type = TypeCheckExpression(file, line, node->right, variables, functions);
		if (type != VariableType::BOOL)
			throw Error(file, line, "operator '!' is used on a '" + VarTypeToStr(type) + "'; operator '!' can only be used on booleans");

		return VariableType::BOOL;
	}
	if (node->data == "||")
	{
		CHECK_EXPR(VariableType::BOOL, VariableType::BOOL);
	}
	if (node->data == "&&")
	{
		CHECK_EXPR(VariableType::BOOL, VariableType::BOOL);
	}
	if (node->data == "==")
	{
		if (TypeCheckExpression(file, line, node->left, variables, functions) !=
			TypeCheckExpression(file, line, node->right, variables, functions))
			throw Error(file, line, "operator '==' can only be used on two values of the same type");
		
		return VariableType::BOOL;
	}
	if (node->data == "!=")
	{
		if (TypeCheckExpression(file, line, node->left, variables, functions) !=
			TypeCheckExpression(file, line, node->right, variables, functions))
			throw Error(file, line, "operator '!=' can only be used on two values of the same type");

		return VariableType::BOOL;
	}
	if (node->data == "[]")
	{
		const VariableType type = TypeCheckExpression(file, line, node->right, variables, functions);
		if (type != VariableType::BOOL_ARR && type != VariableType::NUM_ARR &&
			type != VariableType::STRING_ARR && type != VariableType::STRING)
			throw Error(file, line, "operator '[]' is used on a '" + VarTypeToStr(type) + "'; operator '[]' can only be used on strings or arrays");

		return type;
	}
	if (node->data == "..")
	{
		CHECK_EXPR(VariableType::NUM, VariableType::NUM_ARR);
	}
	if (node->data == ":")
	{
		const VariableType index = TypeCheckExpression(file, line, node->left, variables, functions);
		TypeCheckExpression(file, line, node->right, variables, functions);

		if (index != VariableType::NUM)
			throw Error(file, line, "left hand value of operator ':' is a " + VarTypeToStr(index) + "; left hand value must be a number");

		return VariableType::COORD;
	}
	if (node->data == "<-")
	{
		const VariableType type1 = TypeCheckExpression(file, line, node->left, variables, functions);
		const VariableType type2 = TypeCheckExpression(file, line, node->left, variables, functions);

		if (type1 != VariableType::BOOL_ARR && type1 != VariableType::NUM_ARR &&
			type1 != VariableType::STRING_ARR && type1 != VariableType::STRING)
			throw Error(file, line, "left hand value of operator '<-' is a " + VarTypeToStr(type1) + "; left hand value must be an array");

		// type check insertion of a value

		return type1;
	}
	if (node->data == "->")
	{
		const VariableType type1 = TypeCheckExpression(file, line, node->left, variables, functions);
		const VariableType type2 = TypeCheckExpression(file, line, node->right, variables, functions);

		if (type1 != VariableType::BOOL_ARR && type1 != VariableType::NUM_ARR &&
			type1 != VariableType::STRING_ARR && type1 != VariableType::STRING)
			throw Error(file, line, "left hand value of operator '<-' is a " + VarTypeToStr(type1) + "; left hand value must be an array");

		if (type2 != VariableType::NUM)
			throw Error(file, line, "right hand value of operator '->' is a " + VarTypeToStr(type2) + "; right hand value must be a number");

		return type1;
	}

	assert_rtn(false && "operator missing", VariableType());
}

int GetOperatorPrecedence(const ValueType& type, const std::string& value)
{
	static const std::vector<std::vector<std::string> > operators{
		{ "[]" },
		{ "!" },
		{ "*", "/", "%" },
		{ "+", "-" },
		{ ".." },
		{ ">", "<", ">=", "<=" },
		{ "==", "!=" },
		{ "||", "&&" },
		{ ":" },
		{ "<-", "->" }
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

std::vector<Value> GetBracketExpression(const std::string& file, const int line, const std::vector<Value>& values, std::size_t& index)
{
	std::size_t start = index;
	AdvanceCloseBracketIndex(file, line, values, ValueType::OPEN_BRACKET, ValueType::CLOSE_BRACKET, index);

	if (index >= values.size())
		throw Error(file, line, "missing closing bracket in expression");

	return std::vector<Value>(values.begin() + start, values.begin() + index);
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

			if (values[index].data == "[]")
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
	static std::vector<Variable> variables;
	static std::vector<FunctionDef> functions;

	const std::string file = tokens[0].file;
	const int line = tokens[0].line;

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

		VariableType type;
		const std::shared_ptr<Expression> expression(ExtractExpression(tokens, 3, tokens.size(), variables, functions, &type));

		variables.push_back(Variable{ type, tokens[1].value, expression });

		statements.push_back(Statement{
			StatementType::VARIABLE,
			Variable{ type, tokens[1].value, expression }
		});
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VARIABLE && tokens[1].type == TokenType::ASSIGNMENT)
	{
		if (tokens.size() == 2)
			throw Error(file, line, "expected expression after assignment operator");

		Variable* variable = GetContainer(variables, tokens[0].value);
		if (variable == nullptr)
			throw Error(file, line, "variable '" + tokens[0].value + "' is not defined");

		const std::shared_ptr<Expression> expression(ExtractExpression(tokens, 2, tokens.size(), variables, functions));

		variable->value = expression;

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

		const std::shared_ptr<Expression> condition = ExtractCondition(tokens, closeBracketIndex, variables, functions, "if condition");
		const std::vector<Statement> body = ExtractBody(tokens, closeBracketIndex, variables, "if statement");

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

		const std::shared_ptr<Expression> conditionExpr = ExtractCondition(tokens, closeBracketIndex, variables, functions, "else if condition");
		const std::vector<Statement> body = ExtractBody(tokens, closeBracketIndex, variables, "else if statement");

		std::get<Conditional>(statements.back().stmt).chains.push_back(
			Conditional{ conditionExpr, body }
		);
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::ELSE)
	{
		if (statements.size() == 0 || statements.back().type != StatementType::CONDITIONAL ||
			std::get<Conditional>(statements.back().stmt).condition == nullptr)
			throw Error(file, line, "expected if or else if statement before else statement");

		const std::vector<Statement> body = ExtractBody(tokens, 0, variables, "function definition");

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
			variables.push_back(Variable{ VariableType::COORD, tokens[closeBracketIndex].value });

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

		// constructing statement

		const FunctionDef function{
			tokens[1].value,
			parameters,
			ExtractBody(tokens, closeBracketIndex, variables, "function definitions", true)
		};

		functions.push_back(function);

		statements.push_back(Statement{ StatementType::FUNCTION_DEF, function });
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VARIABLE && tokens[1].type == TokenType::OPEN_BRACKET)
	{
		if (tokens.size() == 2 || (tokens.size() == 3 && tokens[2].type != TokenType::CLOSE_BRACKET))
			throw Error(file, line, "missing closing bracket");

		// function parameters

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

				parameters.push_back(ExtractExpression(tokens, start, a, variables, functions));

				start = a + 1;
				continue;
			}
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

		// while loop condition

		std::size_t closeBracketIndex = 2;
		std::shared_ptr<Expression> condition = ExtractCondition(tokens, closeBracketIndex, variables, functions, "while loop");

		// while loop body

		std::vector<Statement> body = ExtractBody(tokens, closeBracketIndex, variables, "while loop");

		// constructing statement

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
		if (tokens.size() == 3 || (tokens[3].type != TokenType::OPERATOR || tokens[3].value != ":"))
			throw Error(file, line, "expected colon after iterator name");
		if (tokens.size() == 4)
			throw Error(file, line, "expected array after colon");

		if (GetContainer(variables, tokens[2].value) != nullptr)
			throw Error(file, line, "variable '" + tokens[2].value + "' is already defined");

		// evaluating iterator and range

		std::size_t closeBracketIndex = 4; // starts at range
		AdvanceCloseBracketIndex(file, line, tokens, TokenType::OPEN_BRACKET, TokenType::CLOSE_BRACKET, closeBracketIndex);

		if (closeBracketIndex >= tokens.size())
			throw Error(file, line, "missing closing bracket in for loop conditions");

		VariableType rangeType;
		const std::shared_ptr<Expression> rangeExpr(ExtractExpression(tokens, 4, closeBracketIndex, variables, functions, &rangeType));

		if (rangeType != VariableType::BOOL_ARR && rangeType != VariableType::NUM_ARR &&
			rangeType != VariableType::STRING_ARR && rangeType != VariableType::STRING)
			throw Error(file, line, "for loop range evaluated to a " + VarTypeToStr(rangeType) + "; ranges must evaluate to a string or an array");

		const std::size_t variablesSize = variables.size();
		variables.push_back(Variable{ rangeType, tokens[2].value, rangeExpr });

		// extracting scope

		const std::vector<Statement> body = ExtractBody(tokens, closeBracketIndex, variables, "for loop");

		variables.erase(variables.begin() + variablesSize, variables.end());

		// constructing statement

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
		if (variable->value->type != ValueType::STRING && variable->value->extras.empty())
			throw Error(file, line, "variable '" + variable->name + "' doesn't contain a string or an array; to be access with an index, a variable must contain a string or an array");

		// extract and evaluate element

		std::size_t assignmentIndex = 2;
		for (; assignmentIndex < tokens.size() && tokens[assignmentIndex].type != TokenType::ASSIGNMENT; ++assignmentIndex);

		if (tokens[assignmentIndex - 1].type != TokenType::CLOSE_SQUARE)
			throw Error(file, line, "missing closing square");
		if (assignmentIndex == tokens.size())
			throw Error(file, line, "missing assignment operator");

		VariableType elemType;
		const std::shared_ptr<Expression> elemExpr = ExtractExpression(tokens, 2, assignmentIndex - 1, variables, functions, &elemType);

		if (elemType != VariableType::NUM)
			throw Error(file, line, "index evaluates to a '" + VarTypeToStr(elemType) + "'; indices must evaluate to a number");

		if (variable->value->extras.empty() && std::stoul(elemExpr->data) >= variable->value->data.length())
			throw Error(file, line, "index '" + elemExpr->data + "' is out of bounds for string of size '" + std::to_string(variable->value->extras.size()) + "'");
		else if (!variable->value->extras.empty() && std::stoul(elemExpr->data) >= variable->value->extras.size())
			throw Error(file, line, "index '" + elemExpr->data + "' is out of bounds for array of size '" + std::to_string(variable->value->extras.size()) + "'");

		// extract and evaluate expression

		const std::shared_ptr<Expression> assignExpr = ExtractExpression(tokens, assignmentIndex + 1, tokens.size(), variables, functions);

		if (variable->value->extras.empty())
		{
			if (assignExpr->type != ValueType::STRING || assignExpr->data.length() >= 1)
				throw Error(file, line, "a string element can only be assigned to a string of length 1");
		}

		// construct statement
		
		statements.push_back(Statement{
			StatementType::ELEMENT,
			Element{ tokens[0].value, elemExpr, assignExpr }
		});
	}
	else
	{
		throw Error(file, line, "invalid syntax");
	}
}