#include "../../include/back-end/interpreter.hpp"
#include "../../include/back-end/night.hpp"
#include "../../include/back-end/utils.hpp"
#include "../../include/back-end/token.hpp"
#include "../../include/error.hpp"

#include <iostream>
#include <memory>
#include <cmath>
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

			current_scope->variables[variable_stmt->name] = NightData{
				EvaluateExpression(current_scope, variable_stmt->value) };

			break;
		}
		case StatementType::ASSIGNMENT: {
			const Assignment* const assignment = &std::get<Assignment>(statement.stmt);

			auto* const night_variable = get_variable(current_scope, assignment->variable_name);
			assert(night_variable != nullptr);

			const NightData assign_expr =
				EvaluateExpression(current_scope, assignment->assign_expr);

			switch (assignment->assign_type)
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
						throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "expression can not be used with the operator '+='", "the expression contains type '" + assign_expr.type.to_str() + "'; however, if the variable is type 'int' or 'float', then the operator '+=' can only assign expressions of type 'int' or 'float'");

					if (night_variable->second.type == VariableType::INT)
						std::get<int>(night_variable->second.data) += (int)assign_expr.get_num();
					else
						std::get<float>(night_variable->second.data) += assign_expr.get_num();
				}
				else
				{
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "variable '" + night_variable->first + "' can not be assigned using the assignment operator '+='", "the variable contains type '" + night_variable->second.type.to_str() + "'; however, the operator '+=' can only be used on variables of type 'int', 'float', or 'string'");
				}

				break;
			}
			case Assignment::MINUS: {
				if (!night_variable->second.is_num())
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "variable '" + night_variable->first + "' can not be assigned using the assignment operator '-='", "the variable contains type '" + night_variable->second.type.to_str() + "'; however, the operator '-=' can only be used on variables of type 'int' or 'float'");
				if (!assign_expr.is_num())
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "expression can not be used with the operator '-='", "the expression contains type '" + assign_expr.type.to_str() + "'; however, the operator '-=' can only assign expressions of type 'int' or 'float'");

				if (night_variable->second.type == VariableType::INT)
					std::get<int>(night_variable->second.data) -= (int)assign_expr.get_num();
				else
					std::get<float>(night_variable->second.data) -= assign_expr.get_num();

				break;
			}
			case Assignment::TIMES: {
				if (!night_variable->second.is_num())
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "variable '" + night_variable->first + "' can not be assigned using the assignment operator '*='", "the variable contains type '" + night_variable->second.type.to_str() + "'; however, the operator '*=' can only be used on variables of type 'int' or 'float'");
				if (!assign_expr.is_num())
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "expression can not be used with the operator '*='", "the expression contains type '" + assign_expr.type.to_str() + "'; however, the operator '*=' can only assign expressions of type 'int' or 'float'");

				if (night_variable->second.type == VariableType::INT)
					std::get<int>(night_variable->second.data) *= (int)assign_expr.get_num();
				else
					std::get<float>(night_variable->second.data) *= assign_expr.get_num();

				break;
			}
			case Assignment::DIVIDE: {
				if (!night_variable->second.is_num())
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "variable '" + night_variable->first+ "' can not be assigned using the assignment operator '/='", "the operator can only be used on variables of type 'int' or float'");
				if (!assign_expr.is_num())
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "expression can not be used with the operator '/='", "the expression contains type '" + assign_expr.type.to_str() + "'; however, the operator '/=' can only assign expressions of type 'int' or 'float'");

				if (night_variable->second.type == VariableType::INT)
					std::get<int>(night_variable->second.data) /= (int)assign_expr.get_num();
				else
					std::get<float>(night_variable->second.data) /= assign_expr.get_num();

				break;
			}
			case Assignment::MOD: {
				if (!night_variable->second.is_num())
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "variable '" + night_variable->first + "' can not be assigned using the assignment operator '%='", "the operator can only be used on variables of type 'int' or float'");
				if (!assign_expr.is_num())
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "expression can not be used with the operator '%='", "the expression contains type '" + assign_expr.type.to_str() + "'; however, the operator '%=' can only assign expressions of type 'int' or 'float'");

				if (night_variable->second.type == VariableType::INT)
					std::get<int>(night_variable->second.data) /= (int)assign_expr.get_num();
				else
					std::get<float>(night_variable->second.data) /= assign_expr.get_num();

				break;
			}
			}

			break;
		}
		case StatementType::IF_STATEMENT: {
			const IfStatement* const if_stmt = &std::get<IfStatement>(statement.stmt);

			for (const Conditional& conditional : if_stmt->chains)
			{
				if (conditional.condition == nullptr)
				{
					std::shared_ptr<NightScope> if_stmt_scope =
						std::make_shared<NightScope>(current_scope);

					Interpreter(if_stmt_scope, conditional.body->statements, return_value);

					break;
				}

				const NightData condition_expr =
					EvaluateExpression(current_scope, conditional.condition);

				if (condition_expr.type != VariableType::BOOL)
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "if statement expression does not evaluate to the type 'bool'", "conditions must evaluate to the type 'bool'");

				if (std::get<bool>(condition_expr.data))
				{
					std::shared_ptr<NightScope> if_stmt_scope =
						std::make_shared<NightScope>(current_scope);

					Interpreter(if_stmt_scope, conditional.body->statements, return_value);

					break;
				}
			}

			break;
		}
		case StatementType::FUNCTION_DEF: {
			const FunctionDef* const function_def = &std::get<FunctionDef>(statement.stmt);

			night_functions[function_def->name] = NightFunction{
				function_def->parameters, function_def->body->statements };

			break;
		}
		case StatementType::FUNCTION_CALL: {
			const FunctionCall* const function_call =
				&std::get<FunctionCall>(statement.stmt);

			if (function_call->name == "print")
			{
				const NightData data = EvaluateExpression(current_scope, function_call->arguments[0]);

				if (data.type == VariableType::CLASS)
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "argument 1 of function 'print' can only accept basic types", "argument 1 currently is type '" + data.type.to_str() + "'");

				NightPrint(data);

				break;
			}

			auto night_function = night_functions.find(function_call->name);
			assert(night_function != night_functions.end());

			std::shared_ptr<NightScope> function_scope =
				std::make_shared<NightScope>(current_scope);

			// turning parameters into variables
			for (std::size_t a = 0; a < night_function->second.params.size(); ++a)
			{
				function_scope->variables[night_function->second.params[a]] =
					EvaluateExpression(current_scope, function_call->arguments[a]);
			}

			NightData dummy_return_value;
			Interpreter(function_scope, night_function->second.body, &dummy_return_value);

			exit_function = false;
			break;
		}
		case StatementType::RETURN: {
			const Return* const return_stmt = &std::get<Return>(statement.stmt);

			assert(return_value != nullptr);

			if (return_stmt->expression != nullptr)
				*return_value = EvaluateExpression(current_scope, return_stmt->expression);
			else
				return_value = nullptr;

			exit_function = true;
			break;
		}
		case StatementType::WHILE_LOOP: {
			const WhileLoop* const while_loop =
				&std::get<WhileLoop>(statement.stmt);

			while (true)
			{
				const NightData condition =
					EvaluateExpression(current_scope, while_loop->condition);

				if (condition.type != VariableType::BOOL)
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "while loop condition does not evaluate to type 'bool'", "condition is currently type '" + condition.type.to_str() + "'");

				if (!std::get<bool>(condition.data))
					break;

				std::shared_ptr<NightScope> while_loop_scope =
					std::make_shared<NightScope>(current_scope);

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
						std::make_shared<NightScope>(current_scope);

					for_loop_scope->variables[for_loop->iterator_name] =
						NightData{ VariableType::STR, std::string(1, range_value) };

					Interpreter(for_loop_scope, for_loop->body->statements);
				}
			}
			else
			{
				for (const NightData& range_value : range.extras)
				{
					std::shared_ptr<NightScope> for_loop_scope =
						std::make_shared<NightScope>(current_scope);

					for_loop_scope->variables[for_loop->iterator_name] = range_value;

					Interpreter(for_loop_scope, for_loop->body->statements);
				}
			}

			break;
		}
		case StatementType::ELEMENT: {
			const Element* const element_stmt = &std::get<Element>(statement.stmt);

			auto* const night_variable = get_variable(current_scope, element_stmt->name);
			assert(night_variable != nullptr);

			const NightData index_data =
				EvaluateExpression(current_scope, element_stmt->index[0]);
			
			// multidimensional arrays!!!!
			//
			//
			//
			//

			if (index_data.type != VariableType::INT)
				throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "subscript index does not evaluate to type 'int'", "subscript index currently is of type '" + index_data.type.to_str() + "'");

			const long long index = std::get<int>(index_data.data);

			if (index < 0 || index >= night_variable->second.extras.size())
				throw RuntimeError(__FILE__, __LINE__, RuntimeError::out_of_range, file, line, "subscript index is out of range", "array has a size of '" + std::to_string(night_variable->second.extras.size()));

			if (night_variable->second.type == VariableType::STR)
			{
				const NightData assign_data =
					EvaluateExpression(current_scope, element_stmt->assign);

				if (assign_data.type != VariableType::STR)
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "string elements can only be assigned to other strings of length 1", "expression currently is type '" + assign_data.type.to_str() + "'");
				if (std::get<std::string>(assign_data.data).length() != 1)
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "string elements can only be assigned to other strings of length 1", "expression currently has length '" + std::to_string(std::get<std::string>(assign_data.data).length()) + "'");

				std::get<std::string>(night_variable->second.data)[(std::size_t)index] =
					std::get<std::string>(assign_data.data)[0];
			}
			else
			{
				night_variable->second.extras[(std::size_t)index] =
					EvaluateExpression(current_scope, element_stmt->assign);
			}

			break;
		}
		case StatementType::METHOD_CALL: {
			const MethodCall* const method_call = &std::get<MethodCall>(statement.stmt);

			auto* const night_variable = get_variable(current_scope, method_call->name);
			assert(night_variable != nullptr);

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

	const std::string file = node->file;
	const int         line = node->line;

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
			auto* const night_variable = get_variable(current_scope, node->data);
			assert(night_variable != nullptr);

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

			auto night_function = night_functions.find(node->data);
			assert(night_function != night_functions.end());
			assert(night_function->second.params.size() == node->extras.size());

			std::shared_ptr<NightScope> function_scope =
				std::make_shared<NightScope>(current_scope);

			// turn parameters into variables
			for (std::size_t a = 0; a < night_function->second.params.size(); ++a)
			{
				function_scope->variables[night_function->second.params[a]] =
					EvaluateExpression(current_scope, node->extras[a]);
			}

			NightData return_value;
			NightData* return_value_ptr = &return_value;
			Interpreter(function_scope, night_function->second.body, return_value_ptr); 

			if (return_value_ptr == nullptr)
				throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "");

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

	assert(node->right != nullptr);
	assert(node->type == ValueType::OPERATOR);

	// unary operators

	if (node->data == "-")
	{
		if (node->left == nullptr)
		{
			const NightData value = EvaluateExpression(current_scope, node->right);
			if (!value.is_num())
				throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_expression, file, line, "unary operator '-' can only be used on types 'int' or 'float'", "left have value of operator '-' is currently type '" + value.type.to_str() + "'");

			return value.type == VariableType::INT
				? NightData{ value.type, -std::get<int>(value.data) }
				: NightData{ value.type, -std::get<float>(value.data) };
		}

		const NightData value1 = EvaluateExpression(current_scope, node->left);
		const NightData value2 = EvaluateExpression(current_scope, node->right);

		if (!value1.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "binary operator '-' can only be used on types 'int' or 'float'", "left hand value of operator '-' is currently type '" + value2.type.to_str() + "'");
		if (!value2.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "binary operator '-' can only be used on types 'int' or 'float'", "right hand value of operator '-' is currently type '" + value2.type.to_str() + "'");

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
	if (node->data == "!")
	{
		const NightData value = EvaluateExpression(current_scope, node->right);

		if (value.type != VariableType::BOOL)
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "operator '!' can only be used on type 'bool'", "operator  '!' is currently used on type '" + value.type.to_str() + "'");

		return NightData{ VariableType::BOOL, !std::get<bool>(value.data) };
	}
	if (node->data == "[]")
	{
		const NightData data_index = EvaluateExpression(current_scope, node->extras[0]);
		if (data_index.type != VariableType::INT)
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_expression, file, line, "index for subscript operator must be type 'int'", "index is currently type '" + data_index.to_str() + "'");

		const int index = std::get<int>(data_index.data);
		
		const NightData array = EvaluateExpression(current_scope, node->right);
		if (array.type == VariableType::STR)
		{
			if (index < 0 || index >= (int)std::get<std::string>(array.data).length())
				throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_expression, file, line, "index for subscript operator is out of range", "index is value '" + std::to_string(index) + "' but string length is value '" + std::to_string(std::get<std::string>(array.data).length()) + "'");

			return NightData{ VariableType::STR,
				std::string(1, std::get<std::string>(array.data)[(std::size_t)index]) };
		}
		else if (array.type == VariableType::ARRAY)
		{
			if (index < 0 || index >= (int)array.extras.size())
				throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_expression, file, line, "index for subscript operator is out of range", "index is value '" + std::to_string(index) + "' but array length is value '" + std::to_string(array.extras.size()) + "'");

			return array.extras[(std::size_t)index];
		}
		else
		{
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "subscript operator can only be used on types 'str' or 'arr'", "subscript operator is currently used on type '" + array.type.to_str() + "'");
		}
	}

	// binary operators

	assert(node->left != nullptr);

	if (node->data == "+")
	{
		const NightData value1 = EvaluateExpression(current_scope, node->left);
		const NightData value2 = EvaluateExpression(current_scope, node->right);

		if (!value1.is_num() && value1.type != VariableType::STR && value2.type != VariableType::STR)
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "operator '+' can only be used on types 'int', 'float', or 'str'", "left hand value of operator '+' has type '" + value1.type.to_str() + "'");
		if (!value2.is_num() && value2.type != VariableType::STR && value1.type != VariableType::STR)
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "operator '+' can only be used on types 'int', 'float', or 'str'", "right hand value of operator '+' has type '" + value2.type.to_str() + "'");

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
	if (node->data == "*")
	{
		const NightData value1 = EvaluateExpression(current_scope, node->left);
		const NightData value2 = EvaluateExpression(current_scope, node->right);

		if (!value1.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "operator '*' can only be used on types 'int' or 'float'", "left hand value of operator '*' has type '" + value1.type.to_str() + "'");
		if (!value2.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "operator '*' can only be used on types 'int' or 'float'", "right hand value of operator '*' has type '" + value2.type.to_str() + "'");

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

		if (!value1.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "operator '/' can only be used on types 'int' or 'float'", "left hand value of operator '/' has type '" + value1.type.to_str() + "'");
		if (!value2.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "operator '/' can only be used on types 'int' or 'float'", "right hand value of operator '/' has type '" + value2.type.to_str() + "'");

		if (std::get<int>(value2.data) == 0)
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_expression, file, line, "division by 0 is not allowed");

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

		if (!value1.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "operator '%' can only be used on types 'int' or 'float'", "left hand value of operator '%' has type '" + value1.type.to_str() + "'");
		if (!value2.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "operator '%' can only be used on types 'int' or 'float'", "right hand value of operator '%' has type '" + value2.type.to_str() + "'");

		if (value1.type == VariableType::INT && value2.type == VariableType::INT)
		{
			return NightData{ VariableType::INT,
				std::get<int>(value1.data) & std::get<int>(value2.data) };
		}
		else
		{
			return NightData{ VariableType::FLOAT,
				std::fmod(value1.get_num(), value2.get_num()) };
		}
	}
	if (node->data == ">")
	{
		const NightData value1 = EvaluateExpression(current_scope, node->left);
		const NightData value2 = EvaluateExpression(current_scope, node->right);

		if (!value1.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "operator '>' can only be used on types 'int' or 'float'", "left hand value of operator '>' has type '" + value1.type.to_str() + "'");
		if (!value2.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "operator '>' can only be used on types 'int' or 'float'", "right hand value of operator '>' has type '" + value2.type.to_str() + "'");

		return NightData{ VariableType::BOOL,
			value1.get_num() > value2.get_num() };
	}
	if (node->data == "<")
	{
		const NightData value1 = EvaluateExpression(current_scope, node->left);
		const NightData value2 = EvaluateExpression(current_scope, node->right);

		if (!value1.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "operator '<' can only be used on types 'int' or 'float'", "left hand value of operator '<' has type '" + value1.type.to_str() + "'");
		if (!value2.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "operator '<' can only be used on types 'int' or 'float'", "right hand value of operator '<' has type '" + value2.type.to_str() + "'");

		return NightData{ VariableType::BOOL, value1.get_num() < value2.get_num() };
	}
	if (node->data == ">=")
	{
		const NightData value1 = EvaluateExpression(current_scope, node->left);
		const NightData value2 = EvaluateExpression(current_scope, node->right);

		if (!value1.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "operator '>=' can only be used on types 'int' or 'float'", "left hand value of operator '>=' has type '" + value1.type.to_str() + "'");
		if (!value2.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "operator '>=' can only be used on types 'int' or 'float'", "right hand value of operator '>=' has type '" + value2.type.to_str() + "'");

		return NightData{ VariableType::BOOL, value1.get_num() >= value2.get_num() };
	}
	if (node->data == "<=")
	{
		const NightData value1 = EvaluateExpression(current_scope, node->left);
		if (!value1.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "operator '<=' can only be used on types 'int' or 'float'", "left hand value of operator '<=' has type '" + value1.type.to_str() + "'");

		const NightData value2 = EvaluateExpression(current_scope, node->right);
		if (!value2.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "operator '<=' can only be used on types 'int' or 'float'", "right hand value of operator '<=' has type '" + value2.type.to_str() + "'");

		return NightData{ VariableType::BOOL, value1.get_num() <= value2.get_num() };
	}
	if (node->data == "||")
	{
		const NightData value1 = EvaluateExpression(current_scope, node->left);
		if (value1.type != VariableType::BOOL)
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "operator '||' can only be used on type 'bool'", "left hand value of operator '||' has type '" + value1.type.to_str() + "'");

		const NightData value2 = EvaluateExpression(current_scope, node->right);
		if (value2.type != VariableType::BOOL)
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "operator '||' can only be used on type 'bool'", "right hand value of operator '||' has type '" + value2.type.to_str() + "'");

		return NightData{ VariableType::BOOL,
			std::get<bool>(value1.data) || std::get<bool>(value2.data) };
	}
	if (node->data == "&&")
	{
		const NightData value1 = EvaluateExpression(current_scope, node->left);
		const NightData value2 = EvaluateExpression(current_scope, node->right);

		if (value1.type != VariableType::BOOL)
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "operator '&&' can only be used on type 'bool'", "left hand value of operator '||' has type '" + value1.type.to_str() + "'");
		if (value2.type != VariableType::BOOL)
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "operator '*' can only be used on type 'bool'", "right hand value of operator '||' has type '" + value2.type.to_str() + "'");

		return NightData{ VariableType::BOOL,
			std::get<bool>(value1.data) && std::get<bool>(value2.data) };
	}
	if (node->data == "==")
	{
		const NightData value1 = EvaluateExpression(current_scope, node->left);
		const NightData value2 = EvaluateExpression(current_scope, node->right);

		if (value1.type != value2.type)
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_expression, file, line, "operator '==' can only be used on values with the same type", "left hand value has type '" + value1.type.to_str() + "' but right hand value has type '" + value2.type.to_str() + "'");

		return value1.type == VariableType::ARRAY
			? NightData{ VariableType::BOOL, compare_array(value1, value2) }
			: NightData{ VariableType::BOOL, value1.data == value2.data    };
	}
	if (node->data == "!=")
	{
		const NightData value1 = EvaluateExpression(current_scope, node->left);
		const NightData value2 = EvaluateExpression(current_scope, node->right);

		if (value1.type != value2.type)
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_expression, file, line, "operator '==' can only be used on values with the same type", "left hand value currently is type '" + value1.type.to_str() + "' but right hand value currently is type '" + value2.type.to_str() + "'");

		return value1.type == VariableType::ARRAY
			? NightData{ VariableType::BOOL, !compare_array(value1, value2) }
			: NightData{ VariableType::BOOL, value1.data != value2.data    };
	}
	if (node->data == ".")
	{
		NightData object = EvaluateExpression(current_scope, node->left);

		const Expression method = *(node->right);

		if (object.type == VariableType::ARRAY)
		{
			if (method.data == "len")
			{
				return NightData{ VariableType::INT, (int)object.extras.size() };
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
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_expression, file, line, "argument number '1' in function call '" + method.data + "' can only be type 'int'", "argument number '1' is currently type '" + index.to_str() + "'");

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
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, file, line, "index type is required to be type 'int'", 
						"index is currently type '" + index.type.to_str() + "'");

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
		/*
		// implement classes here
		*/
	}

	assert(false);
	return {};
}

NightFunctionContainer Interpreter::night_functions;

bool Interpreter::exit_function = false;

std::pair<const std::string, NightData>* Interpreter::get_variable(
	std::shared_ptr<NightScope>& current_scope,
	const std::string& variable_name
) {
	NightScope* scope_ptr = current_scope.get();
	while (scope_ptr != nullptr)
	{
		for (auto& variable : scope_ptr->variables)
		{
			if (variable.first == variable_name)
				return &variable;
		}

		scope_ptr = scope_ptr->upper_scope.get();
	}

	return nullptr;
}