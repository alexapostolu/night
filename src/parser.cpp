#include "../include/parser.h"
#include "../include/utils.h"
#include "../include/token.h"
#include "../include/error.h"

#include <string>
#include <vector>
#include <unordered_map>

Expression* new_expression(const std::string& file, int line, const Value& value, Expression* left, Expression* right,
	std::vector<Variable>& variables, std::vector<FunctionDef>& functions)
{
	Expression* expression = new Expression{
		value.type,
		value.data,
		std::vector<Expression>(),
		left,
		right
	};

	for (const std::vector<Value>& values : value.extras)
		expression->extras.push_back(*ParseValues(file, line, values, variables, functions));

	return expression;
}

void CheckFunctionDefinition(const std::vector<Statement>& statements, const std::string& file, int line, const std::string& errorMsg)
{
	for (const Statement& statement : statements)
	{
		if (statement.type == StatementType::FUNCTION_DEF)
			throw Error(file, line, errorMsg);
	}
}

std::vector<Value> TokensToValues(const std::vector<Token>& tokens, std::vector<Variable>& variables, std::vector<FunctionDef>& functions)
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
				Value functionCall{ ValueType::CALL };

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

					// type check array elements
					/*
					if (array.extras.size() == 1) {
						array.type = CheckValuesType(array.extras[0]);
					}
					else if (array.type != CheckValuesType(array.extras.back())) {
						throw Error(file, line, "elements in an array must be of the same type");
					}
					*/

					start = a + 1;

					if (tokens[a].type == TokenType::CLOSE_SQUARE)
						break;

					continue;
				}
			}

			if (a >= tokens.size())
				throw Error(file, line, "missing closing square for array");

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
			case TokenType::OPERATOR:
				values.push_back(Value{ ValueType::OPERATOR, tokens[a].value });
				break;
			case TokenType::OPEN_BRACKET:
				values.push_back(Value{ ValueType::OPEN_BRACKET, tokens[a].value });
				break;
			case TokenType::CLOSE_BRACKET:
				values.push_back(Value{ ValueType::CLOSE_BRACKET, tokens[a].value });
				break;
			default:
				throw Error(file, line, "unexpected token '" + tokens[a].value + "' in expression");
			}
		}
	}

	return values;
}

VariableType TypeCheckExpression(const std::string& file, int line, const Expression* node,
	std::vector<Variable>& variables, std::vector<FunctionDef>& functions)
{
	if (node->left == nullptr && node->right == nullptr)
	{
		if (node->type == ValueType::VARIABLE)
		{
			const Variable* variable = GetContainer(variables, node->data);
			assert(variable != nullptr && "variable should already be defined; check definitions in 'TokensToValue()'");

			return TypeCheckExpression(file, line, variable->value, variables, functions);
		}
		if (node->type == ValueType::CALL)
		{
			if (node->data == "input")
				return VariableType::STRING;

			const FunctionDef* function = GetContainer(functions, node->data);
			assert(function != nullptr && "variable should already be defined; check definitions in 'TokensToValue()'");

			if (node->extras.size() != function->parameters.size())
				throw Error(file, line, "function '" + node->data + "' was called with '" + std::to_string(node->extras.size()) + "' parameters, but was defined with '" + std::to_string(function->parameters.size()) + "' parameters");

			// type check function parameters here

			for (const Statement& statement : function->body)
			{
				if (statement.type == StatementType::RETURN)
					return TypeCheckExpression(file, line, std::get<Return>(statement.stmt).expression, variables, functions);
			}

			throw Error(file, line, "function '" + node->data + "' doesn't return a value; functions used in expressions must return a value");
		}

		if (node->type != ValueType::BOOL && node->type != ValueType::BOOL_ARR && node->type != ValueType::NUM &&
			node->type != ValueType::NUM_ARR && node->type != ValueType::STRING && node->type != ValueType::STRING_ARR)
			throw Error(file, line, "invalid expression");
	}

	if (node->data == "+")
	{
		const VariableType type1 = TypeCheckExpression(file, line, node->left, variables, functions);
		const VariableType type2 = TypeCheckExpression(file, line, node->right, variables, functions);

		if ((type1 != VariableType::STRING && type1 != VariableType::NUM) ||
			(type2 != VariableType::STRING && type2 != VariableType::NUM))
			throw Error(file, line, "operator '+' can only be used to add two numbers, or in string concatenation");

	}
	if (node->data == "-")
	{
		if (node->left == nullptr)
		{
			const VariableType type = TypeCheckExpression(file, line, node->right, variables, functions);
			if (type != VariableType::NUM)
				throw Error(file, line, "operator '-' is used on a " + VarTypeToStr(type) + "; operator '-' can only be used on numbers");
		}

		CHECK_EXPR(VariableType::NUM);
	}
	if (node->data == "*")
	{
		CHECK_EXPR(VariableType::NUM);
	}
	if (node->data == "/")
	{
		CHECK_EXPR(VariableType::NUM);
	}
	if (node->data == "%")
	{
		CHECK_EXPR(VariableType::NUM);
	}
	if (node->data == ">")
	{
		CHECK_EXPR(VariableType::NUM);
	}
	if (node->data == "<")
	{
		CHECK_EXPR(VariableType::NUM);
	}
	if (node->data == ">=")
	{
		CHECK_EXPR(VariableType::NUM);
	}
	if (node->data == "<=")
	{
		CHECK_EXPR(VariableType::NUM);
	}
	if (node->data == "!")
	{
		const VariableType type = TypeCheckExpression(file, line, node->right, variables, functions);
		if (type != VariableType::BOOL)
			throw Error(file, line, "operator '!' is used on a '" + VarTypeToStr(type) + "'; operator '!' can only be used on booleans");
	}
	if (node->data == "||")
	{
		CHECK_EXPR(VariableType::BOOL);
	}
	if (node->data == "&&")
	{
		CHECK_EXPR(VariableType::BOOL);
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
		CHECK_EXPR(VariableType::NUM);
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

	assert_rtn(false && "operator missing", VariableType::BOOL);
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

std::vector<Value> GetBracketExpression(const std::string& file, int line, const std::vector<Value>& values, std::size_t& index)
{
	std::size_t start = index;
	AdvanceCloseBracketIndex(file, line, values, ValueType::OPEN_BRACKET, ValueType::CLOSE_BRACKET, index);

	if (index >= values.size())
		throw Error(file, line, "missing closing bracket in expression");

	return std::vector<Value>(values.begin() + start, values.begin() + index);
}

Expression* GetNextGroup(const std::string& file, int line, const std::vector<Value>& values, std::size_t& index,
	std::vector<Variable>& variables, std::vector<FunctionDef>& functions)
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
			Expression* groupExpression = nullptr;
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

Expression* ParseValues(const std::string& file, int line, const std::vector<Value>& values,
	std::vector<Variable>& variables, std::vector<FunctionDef>& functions)
{
	assert(!values.empty() && "values shouldn't be empty");

	std::size_t a = 0;

	Expression* root = GetNextGroup(file, line, values, a, variables, functions);
	Expression* protect = root;

	// parse expression

	while (a < values.size() - 1)
	{
		if (values[a].type != ValueType::OPERATOR)
			throw Error(file, line, "missing operator between values in expression");

		// travel tree to find a nice spot to settle down

		Expression* curr = root;
		Expression* prev = nullptr;
		while ((curr->left != nullptr || curr->right != nullptr) && curr != protect &&
			GetOperatorPrecedence(curr->type, curr->data) > GetOperatorPrecedence(values[a].type, values[a].data))
		{
			prev = curr;
			curr = curr->right;
		}

		// create nodes

		const std::size_t opIndex = a;

		Expression* nextValue = GetNextGroup(file, line, values, ++a, variables, functions);
		Expression* opNode = new_expression(file, line, values[opIndex], curr, nextValue, variables, functions);

		protect = nextValue;

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

	assert(!tokens.empty() && "tokens array should not be empty");

	if (tokens.size() >= 1 && tokens[0].type == TokenType::SET)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::VARIABLE)
			throw Error(file, line, "expected variable name after 'set' keyword");
		if (tokens.size() == 2 || tokens[2].type != TokenType::ASSIGNMENT)
			throw Error(file, line, "expected assignment operator after variable name");
		if (tokens.size() == 3)
			throw Error(file, line, "expected expression after assignment operator");

		if (GetContainer(variables, tokens[1].value) != nullptr)
			throw Error("variable '" + tokens[1].value + "' is already defined");
		if (GetContainer(functions, tokens[1].value) != nullptr)
			throw Error("variable '" + tokens[1].value + "' has the same name as a function; variable and function names must be unique");

		const std::vector<Value> values = TokensToValues(std::vector<Token>(tokens.begin() + 3, tokens.end()), variables, functions);
		Expression* expression = ParseValues(file, line, values, variables, functions);

		TypeCheckExpression(file, line, expression, variables, functions);
		
		const Variable variable{ tokens[1].value, expression };
		const Statement statement{ StatementType::VARIABLE, variable };
		
		variables.push_back(variable);
		statements.push_back(statement);
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VARIABLE && tokens[1].type == TokenType::ASSIGNMENT)
	{
		if (tokens.size() == 2)
			throw Error(file, line, "expected expression after assignment operator");

		Variable* variable = GetContainer(variables, tokens[0].value);
		if (variable == nullptr)
			throw Error("variable '" + tokens[0].value + "' is not defined");

		const std::vector<Value> values = TokensToValues(std::vector<Token>(tokens.begin() + 2, tokens.end()), variables, functions);
		Expression* expression = ParseValues(file, line, values, variables, functions);

		TypeCheckExpression(file, line, expression, variables, functions);

		variable->value = expression;

		const Assignment assignment{ tokens[0].value, expression };
		const Statement statement{ StatementType::ASSIGNMENT, assignment };

		statements.push_back(statement);
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::IF)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::OPEN_BRACKET)
			throw Error(file, line, "expected open bracket after 'if' keyword");
		if (tokens.size() == 2)
			throw Error(file, line, "expected close bracket in if condition");
		if (tokens.size() == 3 && tokens[2].type == TokenType::CLOSE_BRACKET)
			throw Error(file, line, "expected expression in between brackets");

		// if statement condition

		std::size_t closeBracketIndex = 2;
		AdvanceCloseBracketIndex(file, line, tokens, TokenType::OPEN_BRACKET, TokenType::CLOSE_BRACKET, closeBracketIndex);

		if (closeBracketIndex >= tokens.size())
			throw Error(file, line, "missing closing bracket for if statement");

		const std::vector<Value> conditionValues = TokensToValues(
			std::vector<Token>(tokens.begin() + 2, tokens.begin() + closeBracketIndex),
			variables, functions
		);

		const Expression* conditionExpr = ParseValues(file, line, conditionValues, variables, functions);
		const VariableType conditionType = TypeCheckExpression(file, line, conditionExpr, variables, functions);

		if (conditionType != VariableType::BOOL)
			throw Error(file, line, "if statement condition evaluates to a '" + VarTypeToStr(conditionType) + "'; conditions must evaluate to a boolean");

		// if statement body

		std::vector<Statement> scope;
		const std::vector<std::vector<Token> > code = SplitCode(std::vector<Token>(tokens.begin() + closeBracketIndex + 2, tokens.end() - 1));
		for (const std::vector<Token>& tokens : code)
		{
			Parser(scope, tokens);

			if (scope.back().type == StatementType::FUNCTION_DEF)
				throw Error(file, line, "function definition found in if statement; statements cannot contain function definitions");
		}

		const Conditional condition{ conditionExpr, scope };
		const Statement statement{ StatementType::CONDITIONAL, condition };

		statements.push_back(statement);
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

		// else if statement condition

		std::size_t closeBracketIndex = 3;
		AdvanceCloseBracketIndex(file, line, tokens, TokenType::OPEN_BRACKET, TokenType::CLOSE_BRACKET, closeBracketIndex);

		if (closeBracketIndex >= tokens.size())
			throw Error(file, line, "missing closing bracket for else if statement");

		const std::vector<Value> conditionValues = TokensToValues(
			std::vector<Token>(tokens.begin() + 3, tokens.begin() + closeBracketIndex),
			variables, functions
		);

		const Expression* conditionExpr = ParseValues(file, line, conditionValues, variables, functions);
		const VariableType conditionType = TypeCheckExpression(file, line, conditionExpr, variables, functions);

		if (conditionType != VariableType::BOOL)
			throw Error(file, line, "else if statement condition evaluates to a '" + VarTypeToStr(conditionType) + "'; conditions must evaluate to a boolean");

		// else if statement body

		std::vector<Statement> scope;
		const std::vector<std::vector<Token> > code = SplitCode(std::vector<Token>(tokens.begin() + closeBracketIndex + 2, tokens.end() - 1));
		for (const std::vector<Token>& tokens : code)
		{
			Parser(scope, tokens);

			if (scope.back().type == StatementType::FUNCTION_DEF)
				throw Error(file, line, "function definition found in else if statement; statements cannot contain function definitions");
		}

		std::get<Conditional>(statements.back().stmt).chains.push_back(
			Conditional{ conditionExpr, scope }
		);
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::ELSE)
	{
		if (tokens.size() == 2 || tokens[2].type != TokenType::OPEN_CURLY)
			throw Error(file, line, "expected open brace after 'else' keyword");
		if (statements.size() == 0 || statements.back().type != StatementType::CONDITIONAL ||
			std::get<Conditional>(statements.back().stmt).condition == nullptr)
			throw Error(file, line, "expected if or else if statement before else statement");

		// else statement body

		std::vector<Statement> scope;
		std::vector<std::vector<Token> > code = SplitCode(std::vector<Token>(tokens.begin() + 2, tokens.end() - 1));
		for (const std::vector<Token>& tokens : code)
		{
			Parser(scope, tokens);

			if (scope.back().type == StatementType::FUNCTION_DEF)
				throw Error(file, line, "function definition found in else statement; statements cannot contain function definitions");
		}

		std::get<Conditional>(statements.back().stmt).chains.push_back(Conditional{ nullptr, scope });
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
		if (tokens.size() == 4 || (tokens.size() == 5 || tokens[4].type != TokenType::OPEN_CURLY))
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

		// function body

		std::vector<Statement> body;

		Parser(body, std::vector<Token>(tokens.begin() + closeBracketIndex + 2, tokens.end() - 1), true);

		CheckFunctionDefinition(body, file, line, "function '" + tokens[1].value + "' contains a function definition; function definitions must exist outside of any conditional, loop, and function");

		const FunctionDef function{ tokens[1].value, parameters, body };
		const Statement statement{ StatementType::FUNCTION_DEF, function };

		functions.push_back(function);
		statements.push_back(statement);
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VARIABLE && tokens[1].type == TokenType::OPEN_BRACKET)
	{
		if (tokens.size() == 2 || (tokens.size() == 3 && tokens[2].type != TokenType::CLOSE_BRACKET))
			throw Error(file, line, "missing closing bracket");

		// function parameters

		std::vector<Expression*> parameters;
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

				const std::vector<Value> paramExpr = TokensToValues(std::vector<Token>(
					tokens.begin() + start, tokens.begin() + a),
					variables, functions
				);

				parameters.push_back(ParseValues(file, line, paramExpr, variables, functions));

				start = a + 1;
				continue;
			}
		}

		const FunctionCall functionCall{ tokens[0].value, parameters };
		const Statement statement{ StatementType::FUNCTION_CALL, functionCall };

		statements.push_back(statement);
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::RETURN)
	{
		if (!inFunction)
			throw Error(file, line, "return statement is outside of a function definition; return statements can only be inside functions");
		if (tokens.size() == 1)
			throw Error(file, line, "expected expression after 'return' keyword");

		const std::vector<Value> values = TokensToValues(
			std::vector<Token>(tokens.begin() + 1, tokens.end()), 
			variables, functions
		);

		const Return returnStmt{ ParseValues(file, line, values, variables, functions) };
		const Statement statement{ StatementType::RETURN, returnStmt };

		statements.push_back(statement);
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
		AdvanceCloseBracketIndex(file, line, tokens, TokenType::OPEN_BRACKET, TokenType::CLOSE_BRACKET, closeBracketIndex);

		if (closeBracketIndex >= tokens.size())
			throw Error(file, line, "missing closing bracket for while loop condition");
		if (closeBracketIndex == tokens.size() - 1 || tokens[closeBracketIndex].type != TokenType::OPEN_CURLY)
			throw Error(file, line, "expected open curly bracket for while loop body");

		const std::vector<Value> conditionValues = TokensToValues(
			std::vector<Token>(tokens.begin() + 2, tokens.begin() + closeBracketIndex),
			variables, functions
		);

		const Expression* conditionExpr = ParseValues(file, line, conditionValues, variables, functions);
		const VariableType conditionType = TypeCheckExpression(file, line, conditionExpr, variables, functions);

		if (conditionType != VariableType::BOOL)
			throw Error(file, line, "while loop condition evaluated to a '" + VarTypeToStr(conditionType) + "'; conditions must evaluate to a boolean");

		// while loop body

		std::vector<Statement> body;

		Parser(body, std::vector<Token>(tokens.begin() + closeBracketIndex + 1, tokens.end() - 1));
		CheckFunctionDefinition(body, file, line, "while loops cannot contain functions");

		const WhileLoop whileLoop{ conditionExpr, body };
		const Statement statement{ StatementType::WHILE_LOOP, whileLoop };

		statements.push_back(statement);
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

		// range

		std::size_t closeBracketIndex = 0;
		AdvanceCloseBracketIndex(file, line, tokens, TokenType::OPEN_BRACKET, TokenType::CLOSE_BRACKET, closeBracketIndex);

		if (closeBracketIndex >= tokens.size())
			throw Error(file, line, "missing closing bracket in for loop conditions");

		const std::vector<Value> rangeValue = TokensToValues(
			std::vector<Token>(tokens.begin() + 4, tokens.begin() + closeBracketIndex),
			variables, functions
		);

		const Expression* rangeExpr = ParseValues(file, line, rangeValue, variables, functions);
		const VariableType rangeType = TypeCheckExpression(file, line, rangeExpr, variables, functions);

		if (rangeType != VariableType::BOOL_ARR && rangeType != VariableType::NUM_ARR &&
			rangeType != VariableType::STRING_ARR && rangeType != VariableType::STRING)
			throw Error(file, line, "for loop range evaluated to a '" + VarTypeToStr(rangeType) + "'; ranges must evaluate to an array or a string");

		// scope

		std::vector<Statement> statements;
		Parser(statements, std::vector<Token>(tokens.begin() + closeBracketIndex + 1, tokens.end() - 1));

		const ForLoop forLoop{ tokens[1].value, rangeExpr, statements };
		const Statement statement{ StatementType::FOR_LOOP, forLoop };

		statements.push_back(statement);
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VARIABLE && tokens[1].type == TokenType::OPEN_SQUARE)
	{
		if (tokens.size() == 2)
			throw Error(file, line, "expected index after open square");

		Variable* variable = GetContainer(variables, tokens[0].value);
		if (variable == nullptr)
			throw Error(file, line, "variable '" + tokens[1].value + "' is not defined");
		if (variable->value->type != ValueType::STRING && variable->value->extras.empty())
			throw Error(file, line, "variable '" + variable->name + "' doesn't contain a string or an array; to be access with an index, a variable must contain a string or an array");

		// element

		std::size_t assignmentIndex = 2;
		for (; assignmentIndex < tokens.size() && tokens[assignmentIndex].type != TokenType::ASSIGNMENT; ++assignmentIndex);

		if (tokens[assignmentIndex - 1].type != TokenType::CLOSE_SQUARE)
			throw Error(file, line, "missing closing square");
		if (assignmentIndex == tokens.size())
			throw Error(file, line, "missing assignment operator");

		const std::vector<Value> indexValues = TokensToValues(
			std::vector<Token>(tokens.begin() + 2, tokens.begin() + assignmentIndex - 1),
			variables, functions
		);

		const Expression* indexExpr = ParseValues(file, line, indexValues, variables, functions);
		const VariableType indexType = TypeCheckExpression(file, line, indexExpr, variables, functions);

		if (indexType != VariableType::NUM)
			throw Error(file, line, "index evaluates to a '" + VarTypeToStr(indexType) + "'; indices must evaluate to a number");

		if (variable->value->extras.empty() && std::stoul(indexExpr->data) >= variable->value->data.length())
			throw Error("index '" + indexExpr->data + "' is out of bounds for string of size '" + std::to_string(variable->value->extras.size()) + "'");
		else if (!variable->value->extras.empty() && std::stoul(indexExpr->data) >= variable->value->data.length())
			throw Error("index '" + indexExpr->data + "' is out of bounds for array of size '" + std::to_string(variable->value->extras.size()) + "'");

		// assign expression

		const std::vector<Value> assignValues = TokensToValues(
			std::vector<Token>(tokens.begin() + assignmentIndex + 1, tokens.end()),
			variables, functions
		);

		const Expression* assignExpr = ParseValues(file, line, assignValues, variables, functions);
		const VariableType assignType = TypeCheckExpression(file, line, assignExpr, variables, functions);

		const Element element{ tokens[0].value, indexExpr, assignExpr };
		const Statement statement{ StatementType::ELEMENT, element };
		
		if (variable->value->extras.empty())
		{
			if (assignType != VariableType::STRING || assignExpr->type != ValueType::STRING || assignExpr->data.length() >= 1)
				throw Error(file, line, "an element of a string can only be assigned to a string of length 1");

			variable->value->data[std::stoi(indexExpr->data)] = assignExpr->data[0];
		}
		else
		{
			variable->value->extras[std::stoi(indexExpr->data)] = *assignExpr;
		}

		statements.push_back(statement);
	}
	else
	{
		throw Error(file, line, "invalid syntax");
	}
}