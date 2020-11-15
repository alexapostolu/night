#include "../include/interpreter.h"
#include "../include/night.h"
#include "../include/utils.h"
#include "../include/error.h"
#include "../include/token.h"

#include <string>
#include <vector>

Expression EvaluateExpression(const Expression* node, std::vector<NightVariable>& variables, std::vector<NightFunction>& functions)
{
	assert(node != nullptr && "node should not be NULL");

	// if left and right node are NULL, then node must be a value
	if (node->left == nullptr && node->right == nullptr)
	{
		if (node->type == ValueType::VARIABLE)
		{
			const NightVariable* variableValue = GetContainer(variables, node->data);
			assert(variableValue != nullptr && "variableValue is not defined");

			return variableValue->data;
		}
		if (node->type == ValueType::CALL)
		{
			if (node->data == "input")
			{
				std::string uinput;
				getline(std::cin, uinput);

				// type check this!!!
				return Expression{ ValueType::STRING, uinput };
			}

			const NightFunction* function = GetContainer(functions, node->data);
			assert(function != nullptr && "function is not defined");

			// create variables from parameters
			assert(function->parameters.size() == node->extras.size() && "function parameters and function call don't match");
			for (std::size_t a = 0; a < function->parameters.size(); ++a)
			{
				variables.push_back(NightVariable{
					function->parameters[a],
					EvaluateExpression(&node->extras[a], variables, functions)
				});
			}

			// interpret function body and extract return value
			Expression* returnValue = nullptr;
			Interpreter(function->body, returnValue);

			assert(returnValue != nullptr && "function used in expression doesn't have a return statement");

			// remove variables since they are now out of scope
			variables.erase(
				variables.begin() + variables.size() - function->parameters.size(),
				variables.end()
			);

			return *returnValue;
		}

		return *node;
	}

	if (node->data == "+")
	{
		const Expression expr1 = EvaluateExpression(node->left, variables, functions);
		const Expression expr2 = EvaluateExpression(node->right, variables, functions);

		if (expr1.type == ValueType::STRING || expr2.type == ValueType::STRING)
		{
			return Expression{
				ValueType::STRING,
				expr1.data + expr2.data
			};
		}
		else
		{
			return Expression{
				expr1.type,
				std::to_string(std::stof(expr1.data) + std::stof(expr2.data))
			};
		}
	}
	if (node->data == "-")
	{
		if (node->left == nullptr)
		{
			Expression expression = EvaluateExpression(node->right, variables, functions);
			expression.data = std::to_string(-std::stof(expression.data));

			return expression;
		}

		EVAL_EXPR(std::to_string(std::stof(expr1.data) - std::stof(expr2.data)));
	}
	if (node->data == "*")
	{
		EVAL_EXPR(std::to_string(std::stof(expr1.data) * std::stof(expr2.data)));
	}
	if (node->data == "/")
	{
		EVAL_EXPR(std::to_string(std::stof(expr1.data) / std::stof(expr2.data)));
	}
	if (node->data == "%")
	{
		EVAL_EXPR(std::to_string(std::stoi(expr1.data) % std::stoi(expr2.data)));
	}
	if (node->data == ">")
	{
		EVAL_EXPR(std::to_string(std::stof(expr1.data) > std::stof(expr2.data)));
	}
	if (node->data == "<")
	{
		EVAL_EXPR(std::to_string(std::stof(expr1.data) < std::stof(expr2.data)));
	}
	if (node->data == ">=")
	{
		EVAL_EXPR(std::to_string(std::stof(expr1.data) >= std::stof(expr2.data)));
	}
	if (node->data == "<=")
	{
		EVAL_EXPR(std::to_string(std::stof(expr1.data) <= std::stof(expr2.data)));
	}
	if (node->data == "!")
	{
		Expression expression = EvaluateExpression(node->right, variables, functions);
		expression.data = expression.data == "true" ? "false" : "true";

		return expression;
	}
	if (node->data == "||")
	{
		EVAL_EXPR(expr1.data == "true" || expr2.data == "true" ? "true" : "false");
	}
	if (node->data == "&&")
	{
		EVAL_EXPR(expr1.data == "true" && expr2.data == "true" ? "true" : "false");
	}
	if (node->data == "==")
	{
		EVAL_EXPR(expr1.data == expr2.data ? "true" : "false");
	}
	if (node->data == "!=")
	{
		EVAL_EXPR(expr1.data != expr2.data ? "true" : "false");
	}
	if (node->data == "[]")
	{
		const Expression array = EvaluateExpression(node->right, variables, functions);
		return array.extras[std::stoi(EvaluateExpression(&node->extras[0], variables, functions).data)];
	}
	if (node->data == "..")
	{
		const Expression start = EvaluateExpression(node->right, variables, functions);
		const Expression end = EvaluateExpression(node->left, variables, functions);

		Expression array{ ValueType::NUM_ARR };
		for (int a = std::stoi(start.data); a <= std::stoi(end.data); ++a)
		{
			array.extras.push_back(Expression{
				ValueType::NUM,
				std::to_string(a),
				std::vector<Expression>(),
				nullptr,
				nullptr
			});
		}

		return array;
	}
	if (node->data == ":")
	{
		const Expression index = EvaluateExpression(node->left, variables, functions);
		const Expression value = EvaluateExpression(node->right, variables, functions);

		Expression coord{ ValueType::NUM };
		coord.data = index.data;
		coord.extras.push_back(value);

		return coord;
	}
	if (node->data == "<-")
	{
		const Expression index = EvaluateExpression(node->right, variables, functions);
		Expression array = EvaluateExpression(node->left, variables, functions);

		if (index.extras.empty())
		{
			array.extras.push_back(index);
		}
		else
		{
			array.extras.insert(
				array.extras.begin() + std::stoi(index.data),
				index.extras[0]
			);
		}

		return array;
	}
	if (node->data == "->")
	{
		const Expression index = EvaluateExpression(node->right, variables, functions);
		Expression array = EvaluateExpression(node->left, variables, functions);

		assert(!array.extras.empty() && "array shouldn't be empty");
		array.extras.erase(array.extras.begin() + std::stoi(index.data));

		return array;
	}

	assert_rtn(false && "operator missing", Expression());
}

void Interpreter(const std::vector<Statement>& statements, Expression* returnValue)
{
	static std::vector<NightVariable> variables;
	static std::vector<NightFunction> functions;

	const std::size_t variablesSize = variables.size();

	for (const Statement& statement : statements)
	{
		switch (statement.type)
		{
		case StatementType::VARIABLE: {
			variables.push_back(NightVariable{
				std::get<Variable>(statement.stmt).name,
				EvaluateExpression(std::get<Variable>(statement.stmt).value, variables, functions)
			});

			break;
		}
		case StatementType::ASSIGNMENT: {
			NightVariable* variable = GetContainer(variables, std::get<Assignment>(statement.stmt).name);
			assert(variable != nullptr && "definitions should be checked in the parser");

			variable->data = EvaluateExpression(std::get<Assignment>(statement.stmt).value, variables, functions);

			break;
		}
		case StatementType::CONDITIONAL: {
			if (EvaluateExpression(std::get<Conditional>(statement.stmt).condition, variables, functions).data == "true")
			{
				Interpreter(std::get<Conditional>(statement.stmt).body);
				break;
			}

			for (const Conditional& conditional : std::get<Conditional>(statement.stmt).chains)
			{
				if (conditional.condition == nullptr || EvaluateExpression(conditional.condition, variables, functions).data == "true")
				{
					Interpreter(conditional.body);
					break;
				}
			}

			break;
		}
		case StatementType::FUNCTION_DEF: {
			if (GetContainer(variables, std::get<FunctionDef>(statement.stmt).name) != nullptr)
				throw Error("function '" + std::get<FunctionDef>(statement.stmt).name + "' cannot have the same name as a variable");
			if (GetContainer(functions, std::get<FunctionDef>(statement.stmt).name) != nullptr)
				throw Error("function '" + std::get<FunctionDef>(statement.stmt).name + "' is already defined");

			functions.push_back(NightFunction{
				std::get<FunctionDef>(statement.stmt).name,
				std::get<FunctionDef>(statement.stmt).parameters,
				std::get<FunctionDef>(statement.stmt).body
				});

			break;
		}
		case StatementType::FUNCTION_CALL: {
			if (std::get<FunctionCall>(statement.stmt).name == "print")
			{
				for (const Expression* parameter : std::get<FunctionCall>(statement.stmt).parameters)
				{
					Expression expression = EvaluateExpression(parameter, variables, functions);
					NightPrint(expression);
				}

				break;
			}

			const NightFunction* function = GetContainer(functions, std::get<FunctionCall>(statement.stmt).name);
			if (function == nullptr)
				throw Error("function '" + std::get<FunctionCall>(statement.stmt).name + "' is undefined");

			Interpreter(function->body);

			break;
		}
		case StatementType::RETURN: {
			assert(returnValue != nullptr && "returnValue should not be NULL");

			*returnValue = EvaluateExpression(std::get<Return>(statement.stmt).expression, variables, functions);

			variables.erase(variables.begin() + variablesSize, variables.end());
			return;
		}
		case StatementType::WHILE_LOOP: {
			while (EvaluateExpression(std::get<WhileLoop>(statement.stmt).condition, variables, functions).data == "true")
				Interpreter(std::get<WhileLoop>(statement.stmt).body);

			break;
		}
		case StatementType::FOR_LOOP: {
			const Expression range = EvaluateExpression(std::get<ForLoop>(statement.stmt).range, variables, functions);
			assert(!range.extras.empty() && "range should not be empty");

			variables.push_back(NightVariable{ std::get<ForLoop>(statement.stmt).index, Expression() });
			NightVariable* index = &variables.back();

			for (Expression rangeValue : range.extras)
			{
				index->data = rangeValue;
				Interpreter(std::get<ForLoop>(statement.stmt).body);
			}

			break;
		}
		case StatementType::ELEMENT: {
			NightVariable* variable = GetContainer(variables, std::get<Element>(statement.stmt).name);
			if (variable == nullptr)
				throw Error("variable '" + std::get<Element>(statement.stmt).name + "' is not defined");

			if (variable->data.type != ValueType::STRING && variable->data.type != ValueType::BOOL_ARR &&
				variable->data.type != ValueType::NUM_ARR && variable->data.type != ValueType::STRING_ARR)
				throw Error("variable '" + std::get<Element>(statement.stmt).name + "' must contain a string or an array");

			const std::size_t index = std::stoi(EvaluateExpression(std::get<Element>(statement.stmt).index, variables, functions).data);
			if (index >= variable->data.extras.size())
				throw Error("index '" + std::to_string(index) + "' is out of bounds for array of size '" +
					std::to_string(variable->data.extras.size()) + "'");

			variable->data.extras[index] = EvaluateExpression(std::get<Element>(statement.stmt).assign, variables, functions);

			break;
		}
		default: {
			assert(false && "statement missing");
		}
		}
	}

	variables.erase(variables.begin() + variablesSize, variables.end());
}