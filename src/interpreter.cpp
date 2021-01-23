#include "../include/interpreter.hpp"
#include "../include/night.hpp"
#include "../include/utils.hpp"
#include "../include/error.hpp"
#include "../include/token.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

Interpreter::Interpreter(const std::vector<Statement>& statements, NightData* return_value)
{
	const std::size_t variables_size = night_variables.size();
 
	for (const Statement& statement : statements)
	{
		const std::string file = statement.file;
		const int		  line = statement.line;

		switch (statement.type)
		{
		case StatementType::VARIABLE: {
			night_variables.push_back(NightVariable{
				std::get<Variable>(statement.stmt).name,
				EvaluateExpression(std::get<Variable>(statement.stmt).value)
			});

			break;
		}
		case StatementType::ASSIGNMENT: {
			NightVariable* night_variable = night::get_container(night_variables, std::get<Assignment>(statement.stmt).name);
			assert(night_variable != nullptr && "definitions should be checked in the parser");

			const NightData assign_expr = EvaluateExpression(std::get<Assignment>(statement.stmt).value);

			switch (std::get<Assignment>(statement.stmt).assign_type)
			{
			case '=':
				night_variable->value = assign_expr;
				break;
			case '+':
				if (night_variable->value.type == VariableType::STR)
				{
					night_variable->value.data += assign_expr.data;
				}
				else if (night_variable->value.type == VariableType::NUM)
				{
					if (assign_expr.type != VariableType::NUM)
						throw BackError(file, line, "assignment '+=' can only be used on numbers and strings");

					night_variable->value.data = std::to_string(
						std::stof(night_variable->value.data) + std::stof(assign_expr.data)
					);
				}
				else
				{
					throw BackError(file, line, "assignment '+=' can only be used on numbers or strings");
				}
				break;
			case '-':
				if (night_variable->value.type != VariableType::NUM)
					throw BackError(file, line, "assignment '-=' can only be used on numbers");
				if (assign_expr.type != VariableType::NUM)
					throw BackError(file, line, "assignment '-=' can only be used on numbers");

				night_variable->value.data = std::to_string(
					std::stof(night_variable->value.data) - std::stof(assign_expr.data)
				);
				
				break;
			case '*':
				if (night_variable->value.type != VariableType::NUM)
					throw BackError(file, line, "assignment '-=' can only be used on numbers");
				if (assign_expr.type != VariableType::NUM)
					throw BackError(file, line, "assignment '-=' can only be used on numbers");

				night_variable->value.data = std::to_string(
					std::stof(night_variable->value.data) * std::stof(assign_expr.data)
				);

				break;
			case '/':
				if (night_variable->value.type != VariableType::NUM)
					throw BackError(file, line, "assignment '/=' can only be used on numbers");
				if (assign_expr.type != VariableType::NUM)
					throw BackError(file, line, "assignment '/=' can only be used on numbers");

				night_variable->value.data = std::to_string(
					std::stof(night_variable->value.data) / std::stof(assign_expr.data)
				);

				break;
			case '%':
				if (night_variable->value.type != VariableType::NUM)
					throw BackError(file, line, "assignment '%=' can only be used on numbers");
				if (assign_expr.type != VariableType::NUM)
					throw BackError(file, line, "assignment '%=' can only be used on numbers");

				night_variable->value.data = std::to_string(
					std::stoi(night_variable->value.data) % std::stoi(assign_expr.data)
				);

				break;
			default:
				assert(false && "missing assignment operator");
			}

			break;
		}
		case StatementType::IF_STATEMENT: {
			for (const Conditional& conditional : std::get<IfStatement>(statement.stmt).chains)
			{
				if (conditional.condition == nullptr || EvaluateExpression(conditional.condition).data == "true")
				{
					Interpreter(conditional.body, return_value);
					break;
				}
			}

			break;
		}
		case StatementType::FUNCTION_DEF: {
			night_functions.push_back(NightFunction{
				std::get<FunctionDef>(statement.stmt).name,
				std::get<FunctionDef>(statement.stmt).parameters,
				std::get<FunctionDef>(statement.stmt).body
			});

			break;
		}
		case StatementType::FUNCTION_CALL: {
			if (std::get<FunctionCall>(statement.stmt).name == "print")
			{
				const NightData data = EvaluateExpression(std::get<FunctionCall>(statement.stmt).arguments[0]);

				if (data.type == VariableType::CLASS)
					throw BackError(file, line, "function 'print()' cannot accept a class argument");

				NightPrint(data);

				break;
			}

			const NightFunction* function = night::get_container(night_functions, std::get<FunctionCall>(statement.stmt).name);
			assert(function != nullptr && "definitions should be checked in the parser");

			for (std::size_t a = 0; a < function->params.size(); ++a)
			{
				const NightData eval = EvaluateExpression(std::get<FunctionCall>(statement.stmt).arguments[a]);
				night_variables.push_back(NightVariable{ function->params[a], eval });
			}

			Interpreter(function->body);

			break;
		}
		case StatementType::RETURN: {
			assert(return_value != nullptr && "return_value should not be NULL");
			*return_value = EvaluateExpression(std::get<Return>(statement.stmt).expression);

			night_variables.erase(
				std::begin(night_variables) + variables_size,
				std::end(night_variables)
			);

			return;
		}
		case StatementType::WHILE_LOOP: {
			while (EvaluateExpression(std::get<WhileLoop>(statement.stmt).condition).data == "true")
				Interpreter(std::get<WhileLoop>(statement.stmt).body);

			break;
		}
		case StatementType::FOR_LOOP: {
			const NightData range = EvaluateExpression(std::get<ForLoop>(statement.stmt).range);

			night_variables.push_back(NightVariable{ std::get<ForLoop>(statement.stmt).iterator_name, NightData() });
			NightVariable* iterator = &night_variables.back();

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

			night_variables.pop_back();

			break;
		}
		case StatementType::ELEMENT: {
			NightVariable* variable = night::get_container(night_variables, std::get<Element>(statement.stmt).name);
			assert(variable != nullptr && "definitions should be checked in the parser");

			const std::size_t index = std::stoi(EvaluateExpression(std::get<Element>(statement.stmt).index).data);

			if (variable->value.type == VariableType::STR)
				variable->value.data[index] = EvaluateExpression(std::get<Element>(statement.stmt).assign).data[0];
			else
				variable->value.extras[index] = EvaluateExpression(std::get<Element>(statement.stmt).assign);

			break;
		}
		case StatementType::METHOD_CALL: {
			NightVariable* variable = night::get_container(night_variables, std::get<MethodCall>(statement.stmt).name);
			variable->value = EvaluateExpression(std::get<MethodCall>(statement.stmt).method_call);

			break;
		}
		default: {
			assert(false && "statement missing");
		}
		}
	}

	night_variables.erase(
		std::begin(night_variables) + variables_size,
		std::end(night_variables)
	);
}

NightData Interpreter::EvaluateExpression(const std::shared_ptr<Expression>& node)
{
	assert(node != nullptr && "node shouldn't be NULL");

	// if left and right node are NULL, then node must be a value
	if (node->left == nullptr && node->right == nullptr)
	{
		if (node->type == ValueType::ARRAY)
		{
			NightData night_data{ VariableType::ARRAY };

			night_data.extras.reserve(node->extras.size());
			for (const std::shared_ptr<Expression>& element : node->extras)
				night_data.extras.push_back(EvaluateExpression(element));

			return night_data;
		}
		if (node->type == ValueType::VARIABLE)
		{
			const NightVariable* variable = night::get_container(night_variables, node->data);
			assert(variable != nullptr && "definition checks should be handled in the parser");

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
			assert(night_function != nullptr && "definition checks should be handled in the parser");
			assert(night_function->params.size() == node->extras.size() && "parameter checks should be handled in the parser");

			const std::size_t variables_size = night_variables.size();

			// change value of parameter if it is a recursive call
			// if it's not a recursive call, then create a new variable
			for (std::size_t a = 0; a < night_function->params.size(); ++a)
			{
				NightVariable* parameter = night::get_container(night_variables, night_function->params[a]);

				if (parameter == nullptr) // non-recursive
				{
					night_variables.push_back(NightVariable{
						night_function->params[a],
						EvaluateExpression(node->extras[a])
					});
				}
				else // recursive
				{
					parameter->value = EvaluateExpression(node->extras[a]);
				}
			}

			// remove non-recursive variables out of scope here

			NightData return_value;
			Interpreter(night_function->body, &return_value);

			// remove function parameters and variables out of scope
			// and if this is a recursive call, it won't affect anything
			night_variables.erase(
				std::begin(night_variables) + variables_size,
				std::end(night_variables)
			);

			return return_value;
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

	// since expressions can only be on one line, this works fine
	const std::string file = node->file;
	const int         line = node->line;

	if (node->data == "+")
	{
		const NightData value1 = EvaluateExpression(node->left);
		const NightData value2 = EvaluateExpression(node->right);

		if (value1.type != VariableType::NUM && value1.type != VariableType::STR)
			throw BackError(file, line, "operator '+' can only be used on numbers or strings");
		if (value2.type != VariableType::NUM && value2.type != VariableType::STR)
			throw BackError(file, line, "operator '+' can only be used on numbers or strings");

		if (value1.type == VariableType::STR || value2.type == VariableType::STR)
		{
			return NightData{ VariableType::STR, value1.data + value2.data };
		}
		else
		{
			return NightData{
				VariableType::NUM,
				std::to_string(std::stof(value1.data) + std::stof(value2.data))
			};
		}
	}
	if (node->data == "-")
	{
		if (node->left == nullptr)
		{
			NightData value = EvaluateExpression(node->right);

			if (value.type != VariableType::NUM)
				throw BackError(file, line, "unary operator '-' can only be used on numbers");

			value.data = std::to_string(-std::stof(value.data));

			return value;
		}

		const NightData value1 = EvaluateExpression(node->left);
		const NightData value2 = EvaluateExpression(node->right);

		if (value1.type != VariableType::NUM || value2.type != VariableType::NUM)
			throw BackError(file, line, "binary operator '-' can only be used on numbers");

		return NightData{
			VariableType::NUM,
			std::to_string(std::stof(value1.data) - std::stof(value2.data))
		};
	}
	if (node->data == "*")
	{
		const NightData value1 = EvaluateExpression(node->left);
		const NightData value2 = EvaluateExpression(node->right);

		if (value1.type != VariableType::NUM || value2.type != VariableType::NUM)
			throw BackError(file, line, "binary operator '*' can only be used on numbers");

		return NightData{
			VariableType::NUM,
			std::to_string(std::stof(value1.data) * std::stof(value2.data))
		};
	}
	if (node->data == "/")
	{
		const NightData value1 = EvaluateExpression(node->left);
		const NightData value2 = EvaluateExpression(node->right);

		if (value1.type != VariableType::NUM || value2.type != VariableType::NUM)
			throw BackError(file, line, "binary operator '/' can only be used on numbers");

		return NightData{
			VariableType::NUM,
			std::to_string(std::stof(value1.data) / std::stof(value2.data))
		};
	}
	if (node->data == "%")
	{
		const NightData value1 = EvaluateExpression(node->left);
		const NightData value2 = EvaluateExpression(node->right);

		if (value1.type != VariableType::NUM || value2.type != VariableType::NUM)
			throw BackError(file, line, "binary operator '%' can only be used on numbers");

		return NightData{
			VariableType::NUM,
			std::to_string(std::stoi(value1.data) % std::stoi(value2.data))
		};
	}
	if (node->data == ">")
	{
		const NightData value1 = EvaluateExpression(node->left);
		const NightData value2 = EvaluateExpression(node->right);

		if (value1.type != VariableType::NUM || value2.type != VariableType::NUM)
			throw BackError(file, line, "binary operator '>' can only be used on numbers");

		return NightData{
			VariableType::BOOL,
			std::stof(value1.data) > std::stof(value2.data) ? "true" : "false"
		};
	}
	if (node->data == "<")
	{
		const NightData value1 = EvaluateExpression(node->left);
		const NightData value2 = EvaluateExpression(node->right);

		if (value1.type != VariableType::NUM || value2.type != VariableType::NUM)
			throw BackError(file, line, "binary operator '<' can only be used on numbers");

		return NightData{
			VariableType::BOOL,
			std::stof(value1.data) < std::stof(value2.data) ? "true" : "false"
		};
	}
	if (node->data == ">=")
	{
		const NightData value1 = EvaluateExpression(node->left);
		const NightData value2 = EvaluateExpression(node->right);

		if (value1.type != VariableType::NUM || value2.type != VariableType::NUM)
			throw BackError(file, line, "binary operator '>=' can only be used on numbers");

		return NightData{
			VariableType::BOOL,
			std::stof(value1.data) >= std::stof(value2.data) ? "true" : "false"
		};
	}
	if (node->data == "<=")
	{
		const NightData value1 = EvaluateExpression(node->left);
		const NightData value2 = EvaluateExpression(node->right);

		if (value1.type != VariableType::NUM || value2.type != VariableType::NUM)
			throw BackError(file, line, "binary operator '<=' can only be used on numbers");

		return NightData{
			VariableType::BOOL,
			std::stof(value1.data) <= std::stof(value2.data) ? "true" : "false"
		};
	}
	if (node->data == "!")
	{
		NightData value = EvaluateExpression(node->right);

		if (value.type != VariableType::BOOL)
			throw BackError(file, line, "unary operator '!' can only be used on booleans");

		value.data = value.data == "true" ? "false" : "true";
		return value;
	}
	if (node->data == "||")
	{
		const NightData value1 = EvaluateExpression(node->left);
		const NightData value2 = EvaluateExpression(node->right);

		if (value1.type != VariableType::BOOL || value2.type != VariableType::BOOL)
			throw BackError(file, line, "binary operator '||' can only be used on booleans");

		return NightData{
			VariableType::BOOL,
			value1.data == "true" || value2.data == "true" ? "true" : "false"
		};
	}
	if (node->data == "&&")
	{
		const NightData value1 = EvaluateExpression(node->left);
		const NightData value2 = EvaluateExpression(node->right);

		if (value1.type != VariableType::BOOL || value2.type != VariableType::BOOL)
			throw BackError(file, line, "binary operator '&&' can only be used on booleans");

		return NightData{
			VariableType::BOOL,
			value1.data == "true" && value2.data == "true" ? "true" : "false"
		};
	}
	if (node->data == "==")
	{
		const NightData value1 = EvaluateExpression(node->left);
		const NightData value2 = EvaluateExpression(node->right);

		if (value1.type != value2.type)
			throw BackError(file, line, "binary operator '==' can only be used on values with the same type");
		if (value1.type == VariableType::CLASS) // I'll have operator overloading soon :)
			throw BackError(file, line, "binary operator '==' cannot compare two objects");

		return NightData{
			VariableType::BOOL,
			value1.data == value2.data ? "true" : "false"
		};
	}
	if (node->data == "!=")
	{
		const NightData value1 = EvaluateExpression(node->left);
		const NightData value2 = EvaluateExpression(node->right);

		if (value1.type != value2.type)
			throw BackError(file, line, "binary operator '!=' can only be used on values with the same type");
		if (value1.type == VariableType::CLASS)
			throw BackError(file, line, "binary operator '!=' cannot compare two objects");

		return NightData{
			VariableType::BOOL,
			value1.data != value2.data ? "true" : "false"
		};
	}
	if (node->data == "[]")
	{
		const NightData array = EvaluateExpression(node->right);
		const long long index = std::stoll(EvaluateExpression(node->extras[0]).data);

		if (array.type == VariableType::STR)
		{
			if (index < 0 || index >= array.data.length())
				throw BackError(file, line, "string subscript out of range");

			return NightData{ VariableType::STR, std::string(1, array.data[(std::size_t)index]) };
		}
		else
		{
			if (index < 0 || index >= array.extras.size())
				throw BackError(file, line, "array subscript is out of range");

			return array.extras[(std::size_t)index];
		}
	}
	if (node->data == ".")
	{
		assert(node->left != nullptr && node->right != nullptr && "binary node can't have NULL left or right");

		NightData object = EvaluateExpression(node->left);
		const Expression method = *node->right;

		const bool is_array = object.type != VariableType::BOOL && object.type != VariableType::NUM && object.type != VariableType::STR;

		if (is_array && method.data == "len")
		{
			return NightData{ VariableType::NUM, std::to_string(object.extras.size()) };
		}
		if (is_array && method.data == "push")
		{
			const NightData value = EvaluateExpression(method.extras[0]);
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
			const NightData index = EvaluateExpression(method.extras[0]);
			object.extras.erase(object.extras.begin() + std::stoi(index.data));

			return object;
		}
		if (object.type == VariableType::STR && method.data == "len")
		{
			return NightData{ VariableType::NUM, std::to_string(object.data.length()) };
		}
	}

	assert(false && "operator missing");
	return {};
}

std::vector<NightVariable> Interpreter::night_variables;
std::vector<NightFunction> Interpreter::night_functions;
std::vector<NightClass>    Interpreter::night_classes;