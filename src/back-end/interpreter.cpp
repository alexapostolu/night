#include "../../include/back-end/interpreter.hpp"
#include "../../include/back-end/night.hpp"
#include "../../include/back-end/utils.hpp"
#include "../../include/back-end/token.hpp"
#include "../../include/error.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

Interpreter::Interpreter(NightScope& current_scope, const std::vector<Statement>& statements, NightData* return_value)
{
	for (const Statement& statement : statements)
	{
		if (exit_function)
			break;

		const std::string file = statement.file;
		const int		  line = statement.line;

		switch (statement.type)
		{
		case StatementType::VARIABLE: {
			current_scope.night_variables.push_back(NightVariable{
				std::get<Variable>(statement.stmt).name,
				EvaluateExpression(current_scope, std::get<Variable>(statement.stmt).value)
			});

			break;
		}
		case StatementType::ASSIGNMENT: {
			NightVariable* night_variable = night::get_variable(current_scope, std::get<Assignment>(statement.stmt).name);
			assert(night_variable != nullptr);

			const NightData assign_expr = EvaluateExpression(current_scope, std::get<Assignment>(statement.stmt).value);

			switch (std::get<Assignment>(statement.stmt).assign_type)
			{
			case '=': {
				night_variable->value = assign_expr;
				break;
			}
			case '+': {
				if (night_variable->value.type == VariableType::STR)
				{
					std::get<std::string>(night_variable->value.data) += assign_expr.to_str();
				}
				else if (night_variable->value.is_num())
				{
					if (!assign_expr.is_num())
						throw BackError(file, line, "variable '" + night_variable->name + "' is of type number; it can only be assigned to numbers when using assignment operator '+='");

					if (night_variable->value.type == VariableType::INT)
						std::get<int>(night_variable->value.data) += (int)assign_expr.get_num();
					else
						std::get<float>(night_variable->value.data) += assign_expr.get_num();
				}
				else
				{
					throw BackError(file, line, "assignment '+=' can only be used on numbers or strings");
				}

				break;
			}
			case '-': {
				if (!night_variable->value.is_num())
					throw BackError(file, line, "assignment '-=' can only be used on numbers");
				if (!assign_expr.is_num())
					throw BackError(file, line, "assignment '-=' can only be used on numbers");

				if (night_variable->value.type == VariableType::INT)
					std::get<int>(night_variable->value.data) -= (int)assign_expr.get_num();
				else
					std::get<float>(night_variable->value.data) -= assign_expr.get_num();

				break;
			}
			case '*': {
				if (!night_variable->value.is_num())
					throw BackError(file, line, "assignment '-=' can only be used on numbers");
				if (!assign_expr.is_num())
					throw BackError(file, line, "assignment '-=' can only be used on numbers");

				if (night_variable->value.type == VariableType::INT)
					std::get<int>(night_variable->value.data) *= (int)assign_expr.get_num();
				else
					std::get<float>(night_variable->value.data) *= assign_expr.get_num();

				break;
			}
			case '/': {
				if (!night_variable->value.is_num())
					throw BackError(file, line, "assignment '/=' can only be used on numbers");
				if (!assign_expr.is_num())
					throw BackError(file, line, "assignment '/=' can only be used on numbers");

				if (night_variable->value.type == VariableType::INT)
					std::get<int>(night_variable->value.data) /= (int)assign_expr.get_num();
				else
					std::get<float>(night_variable->value.data) /= assign_expr.get_num();

				break;
			}
			case '%': {
				if (night_variable->value.type != VariableType::INT)
					throw BackError(file, line, "assignment '%=' can only be used on integers");
				if (assign_expr.type != VariableType::INT)
					throw BackError(file, line, "assignment '%=' can only be used on integers");

				std::get<int>(night_variable->value.data) %= std::get<int>(assign_expr.data);

				break;
			}
			default: {
				assert(false);
			}
			}

			break;
		}
		case StatementType::IF_STATEMENT: {
			for (const Conditional& conditional : std::get<IfStatement>(statement.stmt).chains)
			{
				if (conditional.condition == nullptr || std::get<bool>(EvaluateExpression(current_scope, conditional.condition).data))
				{
					Interpreter interpret(current_scope, conditional.body.statements, return_value);
					break;
				}
			}

			break;
		}
		case StatementType::FUNCTION_DEF: {
			night_functions.push_back(NightFunction{
				std::get<FunctionDef>(statement.stmt).name,
				std::get<FunctionDef>(statement.stmt).parameters,
				std::get<FunctionDef>(statement.stmt).body.statements
			});

			break;
		}
		case StatementType::FUNCTION_CALL: {
			if (std::get<FunctionCall>(statement.stmt).name == "print")
			{
				const NightData data = EvaluateExpression(current_scope, std::get<FunctionCall>(statement.stmt).arguments[0]);

				if (data.type == VariableType::CLASS)
					throw BackError(file, line, "function 'print()' cannot accept a class argument");

				NightPrint(data);

				break;
			}

			const NightFunction* night_function = night::get_container(night_functions, std::get<FunctionCall>(statement.stmt).name);
			assert(night_function != nullptr && "definitions should be checked in the parser");

			for (std::size_t a = 0; a < night_function->params.size(); ++a)
			{
				const NightData eval = EvaluateExpression(current_scope, std::get<FunctionCall>(statement.stmt).arguments[a]);
				current_scope.night_variables.push_back(NightVariable{ night_function->params[a], eval });
			}

			NightData dummy_return_value;
			Interpreter interpret(current_scope, night_function->body, &dummy_return_value);

			exit_function = false;
			break;
		}
		case StatementType::RETURN: {
			assert(return_value != nullptr);
			*return_value = EvaluateExpression(current_scope, std::get<Return>(statement.stmt).expression);

			exit_function = true;
			break;
		}
		case StatementType::WHILE_LOOP: {
			while (std::get<bool>(EvaluateExpression(current_scope, std::get<WhileLoop>(statement.stmt).condition).data))
				Interpreter interpret(current_scope, std::get<WhileLoop>(statement.stmt).body.statements);

			break;
		}
		case StatementType::FOR_LOOP: {
			const NightData range = EvaluateExpression(current_scope, std::get<ForLoop>(statement.stmt).range);

			current_scope.night_variables.push_back(NightVariable{ std::get<ForLoop>(statement.stmt).iterator_name, NightData() });
			NightVariable* iterator = &current_scope.night_variables.back();

			if (range.type == VariableType::STR)
			{
				for (char rangeValue : std::get<std::string>(range.data))
				{
					iterator->value = NightData{ VariableType::STR, std::string(1, rangeValue) };
					Interpreter interpret(current_scope, std::get<ForLoop>(statement.stmt).body.statements);
				}
			}
			else
			{
				for (const NightData& rangeValue : range.extras)
				{
					iterator->value = rangeValue;
					Interpreter interpret(current_scope, std::get<ForLoop>(statement.stmt).body.statements);
				}
			}

			current_scope.night_variables.pop_back();

			break;
		}
		case StatementType::ELEMENT: {
			NightVariable* variable = night::get_variable(current_scope, std::get<Element>(statement.stmt).name);
			assert(variable != nullptr);

			const std::size_t index = std::get<int>(EvaluateExpression(current_scope, std::get<Element>(statement.stmt).index).data);

			// potential bug:
			//
			// check if index is an int, and if it's in bounds
			//
			// test this first

			if (variable->value.type == VariableType::STR)
			{
				const NightData ch = EvaluateExpression(current_scope, std::get<Element>(statement.stmt).assign);

				// potential bug:
				//
				// check if character is a string
				//
				// TESt this first

				if (std::get<std::string>(ch.data).length() != 1)
					throw BackError(file, line, "string elements can only be assigned to one character");

				std::get<std::string>(variable->value.data)[index] = std::get<std::string>(ch.data)[0];
			}
			else
			{
				variable->value.extras[index] = EvaluateExpression(current_scope, std::get<Element>(statement.stmt).assign);
			}

			break;
		}
		case StatementType::METHOD_CALL: {
			NightVariable* variable = night::get_variable(current_scope, std::get<MethodCall>(statement.stmt).name);
			variable->value = EvaluateExpression(current_scope, std::get<MethodCall>(statement.stmt).method_call);

			break;
		}
		}
	}
}

NightData Interpreter::EvaluateExpression(NightScope& current_scope, const std::shared_ptr<Expression>& node)
{
	assert(node != nullptr);

	// if left and right node are NULL, then node must be a value
	if (node->left == nullptr && node->right == nullptr)
	{
		if (node->type == ValueType::ARRAY)
		{
			NightData night_data{ VariableType::ARRAY };

			night_data.extras.reserve(node->extras.size());
			for (const std::shared_ptr<Expression>& element : node->extras)
				night_data.extras.push_back(EvaluateExpression(current_scope, element));

			return night_data;
		}
		if (node->type == ValueType::VARIABLE)
		{
			const NightVariable* variable = night::get_variable(current_scope, node->data);
			assert(variable != nullptr);

			return variable->value;
		}
		if (node->type == ValueType::CALL)
		{
			if (node->data == "input")
			{
				std::string user_input;
				getline(std::cin, user_input);

				return NightData{ VariableType::STR, user_input };
			}

			const NightFunction* night_function = night::get_container(night_functions, node->data);

			assert(night_function != nullptr);
			assert(night_function->params.size() == node->extras.size());

			NightScope function_scope{ &current_scope };

			for (std::size_t a = 0; a < night_function->params.size(); ++a)
			{
				function_scope.night_variables.push_back(NightVariable{
					night_function->params[a],
					EvaluateExpression(current_scope, node->extras[a])
				});
			}

			NightData return_value;
			Interpreter interpret(function_scope, night_function->body, &return_value);

			exit_function = false;
			return return_value;
		}

		switch (node->type)
		{
		case ValueType::BOOL:
			return NightData{ VariableType::BOOL, node->data == "true" };
		case ValueType::INT:
			return NightData{ VariableType::INT, std::stoi(node->data) };
		case ValueType::FLOAT:
			return NightData{ VariableType::FLOAT, std::stof(node->data) };
		case ValueType::STR:
			return NightData{ VariableType::STR, node->data };
		default:
			assert(false);
		}
	}

	assert(node->type == ValueType::OPERATOR);

	// since expressions can only be on one line, this works fine
	const std::string file = node->file;
	const int         line = node->line;

	if (node->data == "+")
	{
		assert(node->left != nullptr && node->right != nullptr);

		const NightData value1 = EvaluateExpression(current_scope, node->left);
		const NightData value2 = EvaluateExpression(current_scope, node->right);

		if (!value1.is_num() && value1.type != VariableType::STR)
			throw BackError(file, line, "operator '+' can only be used on numbers or strings");
		if (!value2.is_num() && value2.type != VariableType::STR)
			throw BackError(file, line, "operator '+' can only be used on numbers or strings");

		if (value1.type == VariableType::STR || value2.type == VariableType::STR)
		{
			return NightData{ VariableType::STR,
				value1.to_str() + value2.to_str() };
		}

		if (value1.type == VariableType::INT && value2.type == VariableType::INT)
		{
			return NightData{ VariableType::INT,
				std::get<int>(value1.data) + std::get<int>(value2.data) };
		}
		else
		{
			return NightData{ VariableType::FLOAT,
				value1.get_num() + value2.get_num() };
		}
	}
	if (node->data == "-")
	{
		if (node->left == nullptr)
		{
			const NightData value = EvaluateExpression(current_scope, node->right);
			if (!value.is_num())
				throw BackError(file, line, "unary operator '-' can only be used on numbers");

			return value.type == VariableType::INT
				? NightData{ value.type, -std::get<int>  (value.data) }
				: NightData{ value.type, -std::get<float>(value.data) };
		}

		const NightData value1 = EvaluateExpression(current_scope, node->left);
		const NightData value2 = EvaluateExpression(current_scope, node->right);

		if (!value1.is_num() || !value2.is_num())
			throw BackError(file, line, "binary operator '-' can only be used on numbers");

		if (value1.type == VariableType::INT && value2.type == VariableType::INT)
		{
			return NightData{ VariableType::INT,
				std::get<int>(value1.data) - std::get<int>(value2.data) };
		}
		else
		{
			return NightData{ VariableType::FLOAT,
				value1.get_num() - value1.get_num() };
		}
	}
	if (node->data == "*")
	{
		const NightData value1 = EvaluateExpression(current_scope, node->left);
		const NightData value2 = EvaluateExpression(current_scope, node->right);

		if (!value1.is_num() || !value2.is_num())
			throw BackError(file, line, "binary operator '*' can only be used on numbers");

		if (value1.type == VariableType::INT && value2.type == VariableType::INT)
		{
			return NightData{ VariableType::INT,
				std::get<int>(value1.data) * std::get<int>(value2.data) };
		}
		else
		{
			return NightData{ VariableType::FLOAT,
				value1.get_num() * value2.get_num() };
		}
	}
	if (node->data == "/")
	{
		const NightData value1 = EvaluateExpression(current_scope, node->left);
		const NightData value2 = EvaluateExpression(current_scope, node->right);

		if (!value1.is_num() || !value2.is_num())
			throw BackError(file, line, "binary operator '/' can only be used on numbers");

		if (value1.type == VariableType::INT && value2.type == VariableType::INT)
		{
			return NightData{ VariableType::INT,
				std::get<int>(value1.data) / std::get<int>(value2.data) };
		}
		else
		{
			return NightData{ VariableType::FLOAT,
				value1.get_num() / value2.get_num() };
		}
	}
	if (node->data == "%")
	{
		const NightData value1 = EvaluateExpression(current_scope, node->left);
		const NightData value2 = EvaluateExpression(current_scope, node->right);

		if (value1.type != VariableType::INT || value2.type != VariableType::INT)
			throw BackError(file, line, "binary operator '%' can only be used on integers");

		return NightData{ VariableType::INT,
			std::get<int>(value1.data) % std::get<int>(value2.data) };
	}
	if (node->data == ">")
	{
		const NightData value1 = EvaluateExpression(current_scope, node->left);
		const NightData value2 = EvaluateExpression(current_scope, node->right);

		if (!value1.is_num() || !value2.is_num())
			throw BackError(file, line, "binary operator '>' can only be used on numbers");

		return NightData{ VariableType::BOOL, value1.get_num() > value2.get_num() };
	}
	if (node->data == "<")
	{
		const NightData value1 = EvaluateExpression(current_scope, node->left);
		const NightData value2 = EvaluateExpression(current_scope, node->right);

		if (!value1.is_num() || !value2.is_num())
			throw BackError(file, line, "binary operator '<' can only be used on numbers");

		return NightData{ VariableType::BOOL, value1.get_num() < value2.get_num() };
	}
	if (node->data == ">=")
	{
		const NightData value1 = EvaluateExpression(current_scope, node->left);
		const NightData value2 = EvaluateExpression(current_scope, node->right);

		if (!value1.is_num() || !value2.is_num())
			throw BackError(file, line, "binary operator '>=' can only be used on numbers");

		return NightData{ VariableType::BOOL, value1.get_num() >= value2.get_num() };
	}
	if (node->data == "<=")
	{
		const NightData value1 = EvaluateExpression(current_scope, node->left);
		const NightData value2 = EvaluateExpression(current_scope, node->right);

		if (!value1.is_num() || !value2.is_num())
			throw BackError(file, line, "binary operator '<=' can only be used on numbers");

		return NightData{ VariableType::BOOL, value1.get_num() <= value2.get_num() };
	}
	if (node->data == "!")
	{
		const NightData value = EvaluateExpression(current_scope, node->right);

		if (value.type != VariableType::BOOL)
			throw BackError(file, line, "unary operator '!' can only be used on booleans");

		return NightData{ VariableType::BOOL, !std::get<bool>(value.data) };
	}
	if (node->data == "||")
	{
		const NightData value1 = EvaluateExpression(current_scope, node->left);
		const NightData value2 = EvaluateExpression(current_scope, node->right);

		if (value1.type != VariableType::BOOL || value2.type != VariableType::BOOL)
			throw BackError(file, line, "binary operator '||' can only be used on booleans");

		return NightData{ VariableType::BOOL,
			std::get<bool>(value1.data) || std::get<bool>(value2.data) };
	}
	if (node->data == "&&")
	{
		const NightData value1 = EvaluateExpression(current_scope, node->left);
		const NightData value2 = EvaluateExpression(current_scope, node->right);

		if (value1.type != VariableType::BOOL || value2.type != VariableType::BOOL)
			throw BackError(file, line, "binary operator '&&' can only be used on booleans");

		return NightData{ VariableType::BOOL,
			std::get<bool>(value1.data) && std::get<bool>(value2.data) };
	}
	if (node->data == "==")
	{
		const NightData value1 = EvaluateExpression(current_scope, node->left);
		const NightData value2 = EvaluateExpression(current_scope, node->right);

		if (value1.type != value2.type)
			throw BackError(file, line, "binary operator '==' can only be used on values with the same type");
		if (value1.type == VariableType::CLASS)
			throw BackError(file, line, "binary operator '==' cannot compare two objects");

		return value1.type == VariableType::ARRAY
			? NightData{ VariableType::BOOL, compare_array(value1, value2) }
			: NightData{ VariableType::BOOL, value1.data == value2.data    };
	}
	if (node->data == "!=")
	{
		const NightData value1 = EvaluateExpression(current_scope, node->left);
		const NightData value2 = EvaluateExpression(current_scope, node->right);

		if (value1.type != value2.type)
			throw BackError(file, line, "binary operator '!=' can only be used on values with the same type");
		if (value1.type == VariableType::CLASS)
			throw BackError(file, line, "binary operator '!=' cannot compare two objects");

		return value1.type == VariableType::ARRAY
			? NightData{ VariableType::BOOL, !compare_array(value1, value2) }
			: NightData{ VariableType::BOOL, value1.data != value2.data    };
	}
	if (node->data == "[]")
	{
		const NightData array = EvaluateExpression(current_scope, node->right);
		const NightData data_index = EvaluateExpression(current_scope, node->extras[0]);

		if (data_index.type != VariableType::INT)
			throw BackError(file, line, "array index is required to be an integer");

		const int index = std::get<int>(data_index.data);

		if (array.type == VariableType::STR)
		{
			if (index < 0 || index >= (int)std::get<std::string>(array.data).length())
				throw BackError(file, line, "string subscript out of range");

			return NightData{ VariableType::STR,
				std::string(1, std::get<std::string>(array.data)[(std::size_t)index]) };
		}
		else
		{
			if (index < 0 || index >= (int)array.extras.size())
				throw BackError(file, line, "array subscript is out of range");

			return array.extras[(std::size_t)index];
		}
	}
	if (node->data == ".")
	{
		assert(node->left != nullptr && node->right != nullptr);

		NightData object = EvaluateExpression(current_scope, node->left);

		const Expression method = *(node->right);

		if (object.type == VariableType::ARRAY)
		{
			if (method.data == "len")
			{
				return NightData{ VariableType::INT,
					std::to_string(object.extras.size()) };
			}
			if (method.data == "push" && method.extras.size() == 1)
			{
				const NightData value = EvaluateExpression(current_scope, method.extras[0]);
				object.extras.push_back(value);

				return object;
			}
			if (method.data == "push" && method.extras.size() == 2)
			{
				const NightData index = EvaluateExpression(current_scope, method.extras[0]);
				const NightData value = EvaluateExpression(current_scope, method.extras[1]);

				if (index.type != VariableType::INT)
					throw BackError(file, line, "index type is required to be an integer");

				object.extras.insert(object.extras.begin() + std::get<int>(index.data), value);
				return object;
			}
			if (method.data == "pop" && method.extras.empty())
			{
				object.extras.pop_back();
				return object;
			}
			if (method.data == "pop" && !method.extras.empty())
			{
				const NightData index = EvaluateExpression(current_scope, method.extras[0]);
				if (index.type != VariableType::INT)
					throw BackError(file, line, "index type is required to be an integer");

				object.extras.erase(object.extras.begin() + std::get<int>(index.data));

				return object;
			}

			assert(false && "method exists in Parser, but not Interpreter");
		}
		if (object.type == VariableType::STR)
		{
			if (method.data == "len")
			{
				return NightData{ VariableType::INT,
					(int)std::get<std::string>(object.data).length() };
			}

			assert(false && "method exists in Parser, but no Interpreter");
		}

		// to do:
		//
		// implement classes here
		//
	}

	assert(false);
	return {};
}

std::vector<NightFunction> Interpreter::night_functions;
std::vector<NightClass>    Interpreter::night_classes;

bool Interpreter::exit_function = false;