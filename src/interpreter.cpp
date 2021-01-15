#include "../include/interpreter.hpp"
#include "../include/night.hpp"
#include "../include/utils.hpp"
#include "../include/error.hpp"
#include "../include/token.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

NightData EvaluateExpression(const std::shared_ptr<Expression>& node, std::vector<NightVariable>& variables,
	const std::vector<NightFunction>& functions, bool check)
{
	assert(node != nullptr && "node should not be NULL");

	// if left and right node are NULL, then node must be a value
	if (node->left == nullptr && node->right == nullptr)
	{
		if (node->type == ValueType::ARRAY)
		{
			// compile time checks?

			NightData data;
			for (const std::shared_ptr<Expression>& element : node->extras)
				data.extras.push_back(EvaluateExpression(element, variables, functions));

			if (data.extras.empty())
			{
				data.type = VariableType::EMPTY_ARR;
				return data;
			}

			switch (data.extras.back().type)
			{
			case VariableType::BOOL:
				data.type = VariableType::BOOL_ARR;
				break;
			case VariableType::NUM:
				data.type = VariableType::NUM_ARR;
				break;
			case VariableType::STR:
				data.type = VariableType::STR_ARR;
				break;
			case VariableType::BOOL_ARR:
				data.type = VariableType::MULT_BOOL_ARR;
				break;
			case VariableType::NUM_ARR:
				data.type = VariableType::MULT_NUM_ARR;
				break;
			case VariableType::STR_ARR:
				data.type = VariableType::MULT_STR_ARR;
				break;
			case VariableType::EMPTY_ARR:
				data.type = VariableType::MULT_EMPTY_ARR;
				break;
			default:
				data.type = data.extras.back().type;
			}

			return data;
		}
		if (node->type == ValueType::VARIABLE)
		{
			NightVariable* variable = night::get_container(variables, node->data);
			std::cout << node->data << '\n';
			assert(variable != nullptr && "variable is not defined");

			return variable->value;
		}
		if (node->type == ValueType::CALL)
		{
			if (node->data == "input")
			{
				std::string uinput;
				getline(std::cin, uinput);

				return NightData{ VariableType::STR, uinput };
			}

		    const NightFunction* function = night::get_container(functions, node->data);
			assert(function != nullptr && "function is not defined");

			std::vector<NightVariable> varables(variables);
			// create variables from parameters
			assert(function->parameters.size() == node->extras.size() && "function parameters and function call don't match");
			for (std::size_t a = 0; a < function->parameters.size(); ++a)
			{
				/*
				variables.push_back(NightVariable{
					function->parameters[a],
					EvaluateExpression(node->extras[a], variables, functions)
				});
				*/
				for (NightVariable& variable : variables)
				{
					if (variable.name == function->parameters[a])
					{
						variable.value = EvaluateExpression(node->extras[a], variables, functions);
					}
				}
			}

			// interpret function body and extract return value
			NightData returnValue;
			returnValue.data = "___NO___";
			Interpreter(function->body, &returnValue);
			variables = varables;

			// remove variables since they are now out of scope
			/*
			variables.erase(
				variables.begin() + variables.size() - function->parameters.size(),
				variables.end()
			);
			*/

			return returnValue;
		}

		switch (node->type)
		{
		case ValueType::BOOL:
			return NightData{ VariableType::BOOL, node->data };
		case ValueType::NUM:
			return NightData{ VariableType::NUM, node->data };
		case ValueType::STR:
			return NightData{ VariableType::STR, node->data };
		}
	}

	assert(node->type == ValueType::OPERATOR && "node should be operator");

	if (node->data == "+")
	{
		const NightData expr1 = EvaluateExpression(node->left,  variables, functions);
		const NightData expr2 = EvaluateExpression(node->right, variables, functions);

		if (expr1.type != VariableType::BOOL && expr1.type != VariableType::NUM && expr1.type != VariableType::STR)
			throw Error("operator '+' can only be used on numbers or strings");
		if (expr2.type != VariableType::BOOL && expr2.type != VariableType::NUM && expr2.type != VariableType::STR)
			throw Error("operator '+' can only be used on numbers or strings");

		if (expr1.type == VariableType::STR || expr2.type == VariableType::STR)
		{
			return NightData{ VariableType::STR, expr1.data + expr2.data };
		}
		else
		{
			if (expr1.type != VariableType::NUM || expr2.type != VariableType::NUM)
				throw Error("operator '+' can only be used on numbers or strings");

			return NightData{
				expr1.type,
				std::to_string(std::stof(expr1.data) + std::stof(expr2.data))
			};
		}
	}
	if (node->data == "-")
	{
		if (node->left == nullptr)
		{
			NightData expression = EvaluateExpression(node->right, variables, functions);
			expression.data = std::to_string(-std::stof(expression.data));

			return expression;
		}

		EVAL_EXPR(std::to_string(std::stof(expr1.data) - std::stof(expr2.data)), VariableType::NUM);
	}
	if (node->data == "*")
	{
		EVAL_EXPR(std::to_string(std::stof(expr1.data) * std::stof(expr2.data)), VariableType::NUM);
	}
	if (node->data == "/")
	{
		EVAL_EXPR(std::to_string(std::stof(expr1.data) / std::stof(expr2.data)), VariableType::NUM);
	}
	if (node->data == "%")
	{
		EVAL_EXPR(std::to_string(std::stoi(expr1.data) % std::stoi(expr2.data)), VariableType::NUM);
	}
	if (node->data == ">")
	{
		EVAL_EXPR(std::stof(expr1.data) > std::stof(expr2.data) ? "true" : "false", VariableType::BOOL);
	}
	if (node->data == "<")
	{
		EVAL_EXPR(std::stof(expr1.data) < std::stof(expr2.data) ? "true" : "false", VariableType::BOOL);
	}
	if (node->data == ">=")
	{
		EVAL_EXPR(std::stof(expr1.data) >= std::stof(expr2.data) ? "true" : "false", VariableType::BOOL);
	}
	if (node->data == "<=")
	{
		EVAL_EXPR(std::stof(expr1.data) <= std::stof(expr2.data) ? "true" : "false", VariableType::BOOL);
	}
	if (node->data == "!")
	{
		NightData expression = EvaluateExpression(node->right, variables, functions);
		expression.data = expression.data == "true" ? "false" : "true";

		return expression;
	}
	if (node->data == "||")
	{
		EVAL_EXPR(expr1.data == "true" || expr2.data == "true" ? "true" : "false", VariableType::BOOL);
	}
	if (node->data == "&&")
	{
		EVAL_EXPR(expr1.data == "true" && expr2.data == "true" ? "true" : "false", VariableType::BOOL);
	}
	if (node->data == "==")
	{
		EVAL_EXPR(expr1.data == expr2.data ? "true" : "false", VariableType::BOOL);
	}
	if (node->data == "!=")
	{
		EVAL_EXPR(expr1.data != expr2.data ? "true" : "false", VariableType::BOOL);
	}
	if (node->data == "[]")
	{
		const NightData     array = EvaluateExpression(node->right, variables, functions);
		const unsigned long index = std::stoul(EvaluateExpression(node->extras[0], variables, functions).data);

		if (array.type == VariableType::STR)
		{
			if (index < 0 || index >= array.data.length())
				throw Error("string subscript out of range");

			return NightData{ VariableType::STR, std::string(1, array.data[index]) };
		}
		else
		{
			if (index < 0 || index >= array.extras.size())
				throw Error("array subscript is out of range");

			return array.extras[index];
		}
	}
	if (node->data == ".")
	{
		assert(node->left != nullptr && node->right != nullptr && "binary node can't have NULL left or right");

		NightData object        = EvaluateExpression(node->left,  variables, functions);
		const Expression method = *node->right;

		const bool is_array = object.type != VariableType::BOOL && object.type != VariableType::NUM && object.type != VariableType::STR;

		if (is_array && method.data == "len")
		{
			return NightData{ VariableType::NUM, std::to_string(object.extras.size()) };
		}
		if (is_array && method.data == "push")
		{
			const NightData value = EvaluateExpression(method.extras[0], variables, functions);
			object.extras.push_back(value);

			return object;
		}
		if (is_array && method.data == "pop" && method.extras.empty())
		{
			object.extras.pop_back();
			return object;
		}
		if (is_array && method.data == "pop" && !method.extras.empty())
		{
			const NightData index = EvaluateExpression(method.extras[0], variables, functions);
			object.extras.erase(object.extras.begin() + std::stoi(index.data));

			return object;
		}
		if (object.type == VariableType::STR && method.data == "len")
		{
			return NightData{ VariableType::NUM, std::to_string(object.data.length()) };
		}
	}

	assert(false && "operator missing");
	return NightData();
}

void Interpreter(const std::vector<Statement>& statements, NightData* returnValue)
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
			NightVariable* variable = night::get_container(variables, std::get<Assignment>(statement.stmt).name);
			assert(variable != nullptr && "definitions should be checked in the parser");

			switch (std::get<Assignment>(statement.stmt).type)
			{
			case '=':
				variable->value = EvaluateExpression(std::get<Assignment>(statement.stmt).value, variables, functions);
				break;
			case '+': {
				const NightData assign = EvaluateExpression(std::get<Assignment>(statement.stmt).value, variables, functions);

				if (variable->value.type == VariableType::STR)
				{
					variable->value.data += assign.data;
				}
				else if (variable->value.type == VariableType::NUM)
				{
					if (assign.type != VariableType::NUM)
						throw Error("assignment '+=' can only be used on numbers and strings");

					variable->value.data = std::to_string(
						std::stof(variable->value.data) + std::stof(assign.data)
					);
				}
				else
				{
					throw Error("assignment '+=' can only be used on numbers or strings");
				}
				break;
			}
			default:
				break;
			}

			break;
		}
		case StatementType::IF_STATEMENT: {
			if (EvaluateExpression(std::get<Conditional>(statement.stmt).condition, variables, functions).data == "true")
			{
				Interpreter(std::get<Conditional>(statement.stmt).body, returnValue);
				break;
			}

			for (const Conditional& conditional : std::get<Conditional>(statement.stmt).chains)
			{
				if (conditional.condition == nullptr || EvaluateExpression(conditional.condition, variables, functions).data == "true")
				{
					Interpreter(conditional.body, returnValue);
					break;
				}
			}

			break;
		}
		case StatementType::FUNCTION_DEF: {
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
				for (const std::shared_ptr<Expression>& parameter : std::get<FunctionCall>(statement.stmt).arguments)
				{
					const NightData data = EvaluateExpression(parameter, variables, functions);
					NightPrint(data);
				}

				break;
			}

			const NightFunction* function = night::get_container(functions, std::get<FunctionCall>(statement.stmt).name);
			assert(function != nullptr && "definitions should be checked in the parser");

			for (std::size_t a = 0; a < function->parameters.size(); ++a)
			{
				const NightData eval = EvaluateExpression(std::get<FunctionCall>(statement.stmt).arguments[a], variables, functions);
				variables.push_back(NightVariable{ function->parameters[a], eval });
			}

			Interpreter(function->body);

			break;
		}
		case StatementType::RETURN: {
			assert(returnValue != nullptr && "returnValue should not be NULL");
			*returnValue = EvaluateExpression(std::get<Return>(statement.stmt).expression, variables, functions);

			variables.erase(std::begin(variables) + variablesSize, std::end(variables));
			return;
		}
		case StatementType::WHILE_LOOP: {
			while (EvaluateExpression(std::get<WhileLoop>(statement.stmt).condition, variables, functions).data == "true")
				Interpreter(std::get<WhileLoop>(statement.stmt).body);

			break;
		}
		case StatementType::FOR_LOOP: {
			const NightData range = EvaluateExpression(std::get<ForLoop>(statement.stmt).range, variables, functions);

			variables.push_back(NightVariable{ std::get<ForLoop>(statement.stmt).iterator, NightData() });
			NightVariable* iterator = &variables.back();

			if (range.type == VariableType::STR)
			{
				for (char rangeValue : range.data)
				{
					iterator->value = NightData{ VariableType::STR, std::string(1, rangeValue) };
					Interpreter(std::get<ForLoop>(statement.stmt).body);
				}
			}
			else
			{
				for (const NightData& rangeValue : range.extras)
				{
					iterator->value = rangeValue;
					Interpreter(std::get<ForLoop>(statement.stmt).body);
				}
			}

			variables.pop_back();

			break;
		}
		case StatementType::ELEMENT: {
			NightVariable* variable = night::get_container(variables, std::get<Element>(statement.stmt).name);
			assert(variable != nullptr && "definitions should be checked in the parser");

			const std::size_t index = std::stoi(EvaluateExpression(std::get<Element>(statement.stmt).index, variables, functions).data);

			if (variable->value.type == VariableType::STR)
				variable->value.data[index] = EvaluateExpression(std::get<Element>(statement.stmt).assign, variables, functions).data[0];
			else
				variable->value.extras[index] = EvaluateExpression(std::get<Element>(statement.stmt).assign, variables, functions);

			break;
		}
		case StatementType::METHOD_CALL: {
			NightVariable* variable = night::get_container(variables, std::get<MethodCall>(statement.stmt).name);
			variable->value = EvaluateExpression(std::get<MethodCall>(statement.stmt).method_call, variables, functions);

			break;
		}
		default: {
			assert(false && "statement missing");
		}
		}

		if (returnValue != nullptr && returnValue->data != "___NO___")
			break;
	}

	variables.erase(std::begin(variables) + variablesSize, std::end(variables));
}