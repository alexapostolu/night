#include "../../include/back-end/interpreter.hpp"
#include "../../include/back-end/night.hpp"
#include "../../include/back-end/utils.hpp"
#include "../../include/back-end/token.hpp"
#include "../../include/error.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

Interpreter::Interpreter(
	std::shared_ptr<NightScope>& current_scope,
	const std::vector<Statement>& statements,
	NightData* return_value
) {
	for (const Statement& statement : statements)
	{
		if (exit_function)
			break;

		const std::string file = statement.file;
		const int		  line = statement.line;

		switch (statement.type)
		{
		case StatementType::VARIABLE: {
			const Variable* const variable_stmt = &std::get<Variable>(statement.stmt);

			current_scope->night_variables[variable_stmt->name] = NightData{
				EvaluateExpression(current_scope, variable_stmt->value) };

			break;
		}
		case StatementType::ASSIGNMENT: {
			const Assignment* const assignment_stmt = &std::get<Assignment>(statement.stmt);

			auto night_variable = get_variable(current_scope, assignment_stmt->variable_name);
			assert(night_variable != current_scope->night_variables.end());

			const NightData assign_expr = EvaluateExpression(current_scope, assignment_stmt->assign_expr);

			switch (assignment_stmt->assign_type)
			{
			case Assignment::ASSIGN: {
				night_variable->second = assign_expr;
				break;
			}
			case Assignment::PLUS: {
				if (night_variable->second.type == VariableType::STR)
				{
					std::get<std::string>(night_variable->second.data) += assign_expr.to_str();
				}
				else if (night_variable->second.is_num())
				{
					if (!assign_expr.is_num())
						throw RuntimeError(__FILE__, __LINE__, RuntimeError::type_mismatch, file, line, "expression can not be used with the operator '+='", "the expression contains type '" + assign_expr.type.to_str() + "'; however, if the variable is type 'int' or 'float', then the operator '+=' can only assign expressions of type 'int' or 'float'");

					if (night_variable->second.type == VariableType::INT)
						std::get<int>(night_variable->second.data) += (int)assign_expr.get_num();
					else
						std::get<float>(night_variable->second.data) += assign_expr.get_num();
				}
				else
				{
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::type_mismatch, file, line, "variable '" + night_variable->first + "' can not be assigned using the assignment operator '+='", "the variable contains type '" + night_variable->second.type.to_str() + "'; however, the operator '+=' can only be used on variables of type 'int', 'float', or 'string'");
				}

				break;
			}
			case Assignment::MINUS: {
				if (!night_variable->second.is_num())
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::type_mismatch, file, line, "variable '" + night_variable->first + "' can not be assigned using the assignment operator '-='", "the variable contains type '" + night_variable->second.type.to_str() + "'; however, the operator '-=' can only be used on variables of type 'int' or 'float'");
				if (!assign_expr.is_num())
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::type_mismatch, file, line, "expression can not be used with the operator '-='", "the expression contains type '" + assign_expr.type.to_str() + "'; however, the operator '-=' can only assign expressions of type 'int' or 'float'");

				if (night_variable->second.type == VariableType::INT)
					std::get<int>(night_variable->second.data) -= (int)assign_expr.get_num();
				else
					std::get<float>(night_variable->second.data) -= assign_expr.get_num();

				break;
			}
			case Assignment::TIMES: {
				if (!night_variable->second.is_num())
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::type_mismatch, file, line, "variable '" + night_variable->first + "' can not be assigned using the assignment operator '*='", "the variable contains type '" + night_variable->second.type.to_str() + "'; however, the operator '*=' can only be used on variables of type 'int' or 'float'");
				if (!assign_expr.is_num())
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::type_mismatch, file, line, "expression can not be used with the operator '*='", "the expression contains type '" + assign_expr.type.to_str() + "'; however, the operator '*=' can only assign expressions of type 'int' or 'float'");

				if (night_variable->second.type == VariableType::INT)
					std::get<int>(night_variable->second.data) *= (int)assign_expr.get_num();
				else
					std::get<float>(night_variable->second.data) *= assign_expr.get_num();

				break;
			}
			case Assignment::DIVIDE: {
				if (!night_variable->second.is_num())
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::type_mismatch, file, line, "variable '" + night_variable->first+ "' can not be assigned using the assignment operator '/='", "the operator can only be used on variables of type 'int' or float'");
				if (!assign_expr.is_num())
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::type_mismatch, file, line, "expression can not be used with the operator '/='", "the expression contains type '" + assign_expr.type.to_str() + "'; however, the operator '/=' can only assign expressions of type 'int' or 'float'");

				if (night_variable->second.type == VariableType::INT)
					std::get<int>(night_variable->second.data) /= (int)assign_expr.get_num();
				else
					std::get<float>(night_variable->second.data) /= assign_expr.get_num();

				break;
			}
			case Assignment::MOD: {
				if (night_variable->second.type != VariableType::INT)
					throw BackError(file, line, "assignment '%=' can only be used on integers");
				if (assign_expr.type != VariableType::INT)
					throw BackError(file, line, "assignment '%=' can only be used on integers");

				std::get<int>(night_variable->second.data) %= std::get<int>(assign_expr.data);

				break;
			}
			}

			break;
		}
		case StatementType::IF_STATEMENT: {
			for (const Conditional& conditional : std::get<IfStatement>(statement.stmt).chains)
			{
				if (conditional.condition == nullptr || std::get<bool>(EvaluateExpression(current_scope, conditional.condition).data))
				{
					std::shared_ptr<NightScope> if_stmt_scope = std::make_shared<NightScope>(NightScope{ current_scope });
					Interpreter(if_stmt_scope, conditional.body->statements, return_value);
					break;
				}
			}

			break;
		}
		case StatementType::FUNCTION_DEF: {
			night_functions.push_back(NightFunction{
				std::get<FunctionDef>(statement.stmt).name,
				std::get<FunctionDef>(statement.stmt).parameters,
				std::get<FunctionDef>(statement.stmt).body->statements
			});

			break;
		}
		case StatementType::FUNCTION_CALL: {
			const FunctionCall* const function_call = &std::get<FunctionCall>(statement.stmt);
			if (function_call->name == "print")
			{
				const NightData data = EvaluateExpression(current_scope, function_call->arguments[0]);

				if (data.type == VariableType::CLASS)
					throw BackError(file, line, "function 'print' cannot accept a class argument");

				NightPrint(data);

				break;
			}

			auto night_function = std::find_if(night_functions.begin(), night_functions.end(),
				[&](const NightFunction& func) { return func.name == function_call->name; });

			assert(night_function != night_functions.end());

			for (std::size_t a = 0; a < night_function->params.size(); ++a)
			{
				const NightData eval = EvaluateExpression(current_scope, function_call->arguments[a]);
				current_scope->night_variables[night_function->params[a]] = eval;
			}

			NightData dummy_return_value;

			std::shared_ptr<NightScope> function_scope =
				std::make_shared<NightScope>(NightScope{ current_scope });

			Interpreter(function_scope, night_function->body, &dummy_return_value);

			exit_function = false;
			break;
		}
		case StatementType::RETURN: {
			const Return* const return_stmt = &std::get<Return>(statement.stmt);

			assert(return_value != nullptr);
			*return_value = EvaluateExpression(current_scope, return_stmt->expression);

			exit_function = true;
			break;
		}
		case StatementType::WHILE_LOOP: {
			const WhileLoop* const while_loop = &std::get<WhileLoop>(statement.stmt);

			while (std::get<bool>(EvaluateExpression(current_scope, while_loop->condition).data))
			{
				std::shared_ptr<NightScope> while_loop_scope =
					std::make_shared<NightScope>(NightScope{ current_scope });

				Interpreter(while_loop_scope, while_loop->body->statements);
			}

			break;
		}
		case StatementType::FOR_LOOP: {
			const ForLoop* const for_loop = &std::get<ForLoop>(statement.stmt);

			const NightData range = EvaluateExpression(current_scope, for_loop->range);
			if (range.type == VariableType::STR)
			{
				for (char range_value : std::get<std::string>(range.data))
				{
					std::shared_ptr<NightScope> for_loop_scope =
						std::make_shared<NightScope>(NightScope{ current_scope });

					for_loop_scope->night_variables[for_loop->iterator_name] =
						NightData{ VariableType::STR, std::string(1, range_value) };

					Interpreter(for_loop_scope, for_loop->body->statements);
				}
			}
			else
			{
				for (const NightData& range_value : range.extras)
				{
					std::shared_ptr<NightScope> for_loop_scope =
						std::make_shared<NightScope>(NightScope{ current_scope });

					for_loop_scope->night_variables[for_loop->iterator_name] = range_value;

					Interpreter(for_loop_scope, for_loop->body->statements);
				}
			}

			break;
		}
		case StatementType::ELEMENT: {
			const Element* const element = &std::get<Element>(statement.stmt);

			auto night_variable = get_variable(current_scope, element->name);
			assert(night_variable != current_scope->night_variables.end());

			const NightData index_data = EvaluateExpression(current_scope, element->index);

			if (index_data.type != VariableType::INT)
				throw RuntimeError(__FILE__, __LINE__, RuntimeError::type_mismatch, file, line, "subscript index does not evaluate to type 'int'", "subscript index currently is of type '" + index_data.type.to_str() + "'");

			const int index = std::get<int>(index_data.data);

			if (index < 0 || index >= night_variable->second.extras.size())
				throw RuntimeError(__FILE__, __LINE__, RuntimeError::out_of_range, file, line, "subscript index is out of range", "array has a size of '" + std::to_string(night_variable->second.extras.size()));

			if (night_variable->second.type == VariableType::STR)
			{
				const NightData assign_data = EvaluateExpression(current_scope, std::get<Element>(statement.stmt).assign);
				if (assign_data.type != VariableType::STR)
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::type_mismatch, file, line, "string elements can only be assigned to other strings of length 1", "expression currently is type '" + assign_data.type.to_str() + "'");
				if (std::get<std::string>(assign_data.data).length() != 1)
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::type_mismatch, file, line, "string elements can only be assigned to other strings of length 1", "expression currently has length '" + std::to_string(std::get<std::string>(assign_data.data).length()) + "'");

				std::get<std::string>(night_variable->second.data)[index] =
					std::get<std::string>(assign_data.data)[0];
			}
			else
			{
				night_variable->second.extras[index] =
					EvaluateExpression(current_scope, element->assign);
			}

			break;
		}
		case StatementType::METHOD_CALL: {
			const MethodCall* const method_call = &std::get<MethodCall>(statement.stmt);

			auto night_variable = get_variable(current_scope, method_call->name);
			assert(night_variable != current_scope->night_variables.end());

			night_variable->second = EvaluateExpression(current_scope, method_call->assign_expr);

			break;
		}
		}
	}
}

NightData Interpreter::EvaluateExpression(
	std::shared_ptr<NightScope>& current_scope,
	const std::shared_ptr<Expression>& node)
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
			auto night_variable = get_variable(current_scope, node->data);
			assert(night_variable != current_scope->night_variables.end());

			return night_variable->second;
		}
		if (node->type == ValueType::CALL)
		{
			if (node->data == "input")
			{
				std::string user_input;
				getline(std::cin, user_input);

				return NightData{ VariableType::STR, user_input };
			}

			auto night_function = std::find_if(night_functions.begin(), night_functions.end(),
				[&](const NightFunction& night_func) { return night_func.name == node->data; });

			assert(night_function != night_functions.end());
			assert(night_function->params.size() == node->extras.size());

			std::shared_ptr<NightScope> function_scope =
				std::make_shared<NightScope>(NightScope{ current_scope });

			for (std::size_t a = 0; a < night_function->params.size(); ++a)
			{
				function_scope->night_variables[night_function->params[a]] =
					EvaluateExpression(current_scope, node->extras[a]);
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

bool Interpreter::exit_function = false;

std::unordered_map<std::string, NightData>::iterator Interpreter::get_variable(
	std::shared_ptr<NightScope>& current_scope,
	const std::string& variable_name
) {
	NightScope* scope_ptr = current_scope.get();
	while (scope_ptr != nullptr)
	{
		auto it = scope_ptr->night_variables.find(variable_name);
		if (it != scope_ptr->night_variables.end())
			return it;

		scope_ptr = scope_ptr->upper_scope.get();
	}

	return current_scope->night_variables.end();
}