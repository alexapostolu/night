#include "../../include/back-end/interpreter.hpp"
#include "../../include/back-end/night.hpp"
#include "../../include/back-end/utils.hpp"
#include "../../include/back-end/token.hpp"
#include "../../include/error.hpp"

#include <iostream>
#include <stdexcept>
#include <memory>
#include <cmath>
#include <string>
#include <vector>

void interpret_statement(std::shared_ptr<Scope>& scope, const Stmt& stmt)
{
	static ExprValue return_value;
	static bool exit_function = false;

	if (exit_function)
		return;

	switch (stmt.type)
	{
	case StmtType::ASSIGN: {
		StmtAssign const& stmt_assign = std::get<StmtAssign>(stmt.data);

		NightData const assign_expr =
			evaluate_expression(scope, stmt_assign.assign_expr);

		switch (stmt_assign.type)
		{
		case StmtAssign::ASSIGN: {
			stmt_assign.check_var.expr = stmt_assign.assign_expr;
			break;
		}
		case StmtAssign::PLUS: {
			if (stmt_assign.check_var.value. == ValueType::STR)
			{
				std::get<std::string>(night_variable->second.data) += assign_expr.to_str();
			}
			else if (night_variable->second.is_num())
			{
				if (!assign_expr.is_num())
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "expression can not be used with the operator '+='", "the expression contains type '" + assign_expr.type.to_str() + "'; however, if the variable is type 'int' or 'float', then the operator '+=' can only assign expressions of type 'int' or 'float'");

				if (night_variable->second.type == VariableType::INT)
					std::get<int>(night_variable->second.data) += (int)assign_expr.get_num();
				else
					std::get<float>(night_variable->second.data) += assign_expr.get_num();
			}
			else
			{
				throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "variable '" + night_variable->first + "' can not be assigned using the assignment operator '+='", "the variable contains type '" + night_variable->second.type.to_str() + "'; however, the operator '+=' can only be used on variables of type 'int', 'float', or 'string'");
			}

			break;
		}
		case Assignment::MINUS: {
			if (!night_variable->second.is_num())
				throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "variable '" + night_variable->first + "' can not be assigned using the assignment operator '-='", "the variable contains type '" + night_variable->second.type.to_str() + "'; however, the operator '-=' can only be used on variables of type 'int' or 'float'");
			if (!assign_expr.is_num())
				throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "expression can not be used with the operator '-='", "the expression contains type '" + assign_expr.type.to_str() + "'; however, the operator '-=' can only assign expressions of type 'int' or 'float'");

			if (night_variable->second.type == VariableType::INT)
				std::get<int>(night_variable->second.data) -= (int)assign_expr.get_num();
			else
				std::get<float>(night_variable->second.data) -= assign_expr.get_num();

			break;
		}
		case Assignment::TIMES: {
			if (!night_variable->second.is_num())
				throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "variable '" + night_variable->first + "' can not be assigned using the assignment operator '*='", "the variable contains type '" + night_variable->second.type.to_str() + "'; however, the operator '*=' can only be used on variables of type 'int' or 'float'");
			if (!assign_expr.is_num())
				throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "expression can not be used with the operator '*='", "the expression contains type '" + assign_expr.type.to_str() + "'; however, the operator '*=' can only assign expressions of type 'int' or 'float'");

			if (night_variable->second.type == VariableType::INT)
				std::get<int>(night_variable->second.data) *= (int)assign_expr.get_num();
			else
				std::get<float>(night_variable->second.data) *= assign_expr.get_num();

			break;
		}
		case Assignment::DIVIDE: {
			if (!night_variable->second.is_num())
				throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "variable '" + night_variable->first+ "' can not be assigned using the assignment operator '/='", "the operator can only be used on variables of type 'int' or float'");
			if (!assign_expr.is_num())
				throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "expression can not be used with the operator '/='", "the expression contains type '" + assign_expr.type.to_str() + "'; however, the operator '/=' can only assign expressions of type 'int' or 'float'");

			if (night_variable->second.type == VariableType::INT)
				std::get<int>(night_variable->second.data) /= (int)assign_expr.get_num();
			else
				std::get<float>(night_variable->second.data) /= assign_expr.get_num();

			break;
		}
		case Assignment::MOD: {
			if (!night_variable->second.is_num())
				throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "variable '" + night_variable->first + "' can not be assigned using the assignment operator '%='", "the operator can only be used on variables of type 'int' or float'");
			if (!assign_expr.is_num())
				throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "expression can not be used with the operator '%='", "the expression contains type '" + assign_expr.type.to_str() + "'; however, the operator '%=' can only assign expressions of type 'int' or 'float'");

			if (night_variable->second.type == VariableType::INT)
				std::get<int>(night_variable->second.data) /= (int)assign_expr.get_num();
			else
				std::get<float>(night_variable->second.data) /= assign_expr.get_num();

			break;
		}
		}

		break;
	}
	case StmtType::IF: {
		const IfStatement& if_stmt = std::get<IfStatement>(stmt.data);

		for (const Conditional& conditional : if_stmt.chains)
		{
			if (conditional.condition == nullptr)
			{
				for (auto& body_stmt : conditional.body->statements)
					interpret_statement(conditional.body, body_stmt);

				return;
			}

			const NightData condition_expr =
				evaluate_expression(curr_scope, conditional.condition);

			if (condition_expr.type != ValueType::BOOL) {
				throw NIGHT_RUNTIME_ERROR(
					"if statement condition is not type: 'bool'",
					"condition currently is type: '" + condition_expr.type.to_str() + "'",
					night::learn_conditionals);
			}

			if (std::get<bool>(condition_expr.data))
			{
				for (auto& body_stmt : conditional.body->statements)
					interpret_statement(conditional.body, body_stmt);
			}

			return;
		}

		break;
	}
	case StmtType::FUNC_CALL: {
		const FunctionCall& function_call = std::get<FunctionCall>(stmt.data);

		if (function_call.name == "print")
		{
			const NightData data = evaluate_expression(
				curr_scope, function_call.arguments[0]);

			NightPrint(data);

			break;
		}
		if (function_call.name == "input")
		{
			std::string user_input;
			getline(std::cin, user_input);

			break;
		}
		if (function_call->name == "system")
		{
			const NightData arg = evaluate_expression(curr_scope, function_call.arguments[0]);
			system(std::get<std::string>(arg.data).c_str());

			break;
		}

		auto night_function = night_functions.find(function_call->name);
		assert(night_function != night_functions.end());

		std::shared_ptr<NightScope> function_scope =
			std::make_shared<NightScope>(curr_scope);

		// turning parameters into variables
		for (std::size_t a = 0; a < night_function->second.params.size(); ++a)
		{
			function_scope->variables[night_function->second.params[a]] =
				evaluate_expression(curr_scope, function_call->arguments[a]);
		}

		NightData dummy_return_value;
		Interpret(function_scope, night_function->second.body, &dummy_return_value);

		exit_function = false;
		break;
	}
	case StmtType::RETURN: {
		const Return& return_stmt = std::get<Return>(stmt.data);

		if (return_stmt.expression != nullptr)
			*return_value = evaluate_expression(curr_scope, return_stmt->expression);
		else
			return_value = nullptr;

		exit_function = true;
		break;
	}
	case StmtType::WHILE: {
		const WhileLoop& while_loop = std::get<WhileLoop>(stmt.data);

		while (true)
		{
			const NightData condition =
				evaluate_expression(curr_scope, while_loop.condition);

			if (condition.type != VariableType::BOOL)
				throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "while loop condition does not evaluate to type 'bool'", "condition is currently type '" + condition.type.to_str() + "'");

			if (!std::get<bool>(condition.data))
				break;

			std::shared_ptr<NightScope> while_loop_scope =
				std::make_shared<NightScope>(current_scope);

			Interpret(while_loop_scope, while_loop->body->statements);
		}

		break;
	}
	case StmtType::FOR: {
		const ForLoop* const for_loop = &std::get<ForLoop>(statement.stmt);

		const NightData range = evaluate_expression(current_scope, for_loop->range);
		if (range.type == VariableType::STRING)
		{
			for (char range_value : std::get<std::string>(range.data))
			{
				std::shared_ptr<NightScope> for_loop_scope =
					std::make_shared<NightScope>(current_scope);

				for_loop_scope->variables[for_loop->iterator_name] =
					NightData{ VariableType::STRING, std::string(1, range_value) };

				Interpret(for_loop_scope, for_loop->body->statements);
			}
		}
		else
		{
			for (const NightData& range_value : range.extras)
			{
				std::shared_ptr<NightScope> for_loop_scope =
					std::make_shared<NightScope>(current_scope);

				for_loop_scope->variables[for_loop->iterator_name] = range_value;

				Interpret(for_loop_scope, for_loop->body->statements);
			}
		}

		break;
	}
	case StmtType::METHOD: {
		const MethodCall* const method_call = &std::get<MethodCall>(statement.stmt);

		auto* const night_variable = get_variable(current_scope, method_call->name);
		assert(night_variable != nullptr);

		night_variable->second = evaluate_expression(current_scope, method_call->assign_expr);

		break;
	}
	default: {
		return;
	}
	}
}

NightData evaluate_expression(
	std::shared_ptr<ExprNode>& current_scope,
	const std::shared_ptr<ExprNode>& node)
{
	const Location loc = node->loc;

	// if left and right node are NULL, then node must be a value
	if (node->left == nullptr && node->right == nullptr)
	{
		switch (node->type)
		{
		case ValueType::ARRAY: {
			NightData night_data{ VariableType::ARRAY };

			night_data.extras.reserve(node->extras.size());
			for (const std::shared_ptr<Expression>& element : node->extras)
				night_data.extras.push_back(evaluate_expression(current_scope, element));

			return night_data;
		}
		case ValueType::VAR: {
			auto* const night_variable = get_variable(current_scope, node->data);
			assert(night_variable != nullptr);

			return night_variable->second;
		}
		case ValueType::CALL: {
			if (node->data == "input")
			{
				std::string user_input;
				getline(std::cin, user_input);

				return NightData{ VariableType::STRING, user_input };
			}
			if (node->data == "int")
			{
				const NightData param = evaluate_expression(current_scope, node->extras[0]);
				if (param.type == VariableType::INT)
				{
					return param;
				}
				if (param.type == VariableType::FLOAT)
				{
					return NightData{ VariableType::INT, (int)std::get<float>(param.data) };
				}
				if (param.type == VariableType::STRING)
				{
					try {
						return NightData{ VariableType::INT, std::stoi(std::get<std::string>(param.data)) };
					}
					catch (const std::invalid_argument&) {
						throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "value '" + node->extras[0]->data + "' can not be converted into type int");
					}
				}
			}
			if (node->data == "float")
			{
				const NightData param = evaluate_expression(current_scope, node->extras[0]);
				if (param.type == VariableType::INT || param.type == VariableType::FLOAT)
				{
					return NightData{ VariableType::FLOAT, param.get_num() };
				}
				if (param.type == VariableType::STRING)
				{
					try {
						return NightData{ VariableType::FLOAT, std::stof(std::get<std::string>(param.data)) };
					}
					catch (const std::invalid_argument&) {
						throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "value '" + node->extras[0]->data + "' can not be converted into type float");
					}
				}
			}
			if (node->data == "range")
			{
				assert(node->extras.size() == 2);

				const NightData start = evaluate_expression(current_scope, node->extras[0]);
				if (start.type != VariableType::INT)
					throw NIGHT_RUNTIME_ERROR("argument 1 in function must be type 'int'", "range ");
				assert(start.type == VariableType::INT);

				const NightData end = evaluate_expression(current_scope, node->extras[1]);
				assert(end.type == VariableType::INT);

				NightData result{ VariableType::ARRAY };
				result.extras.reserve(std::get<int>(end.data) - std::get<int>(start.data) + 1);

				for (int a = std::get<int>(start.data); a <= std::get<int>(end.data); ++a)
					result.extras.push_back(NightData{ VariableType::INT, a });

				return result;
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
					evaluate_expression(current_scope, node->extras[a]);
			}

			NightData return_value;
			NightData* return_value_ptr = &return_value;
			Interpret(function_scope, night_function->second.body, return_value_ptr);

			if (return_value_ptr == nullptr)
				throw NIGHT_RUNTIME_ERROR("function '" + night_function->first + "' does not return a value", "functions used in expressions must return a value", Learn::FUNCTIONS);

			exit_function = false;
			return return_value;
		}
		case ValueType::BOOL: {
			return NightData{ VariableType::BOOL, node->data == "true" };
		}
		case ValueType::INT: {
			return NightData{ VariableType::INT, std::stoi(node->data) };
		}
		case ValueType::FLOAT: {
			return NightData{ VariableType::FLOAT, std::stof(node->data) };
		}
		case ValueType::STRING: {
			return NightData{ VariableType::STRING, node->data };
		}
		default: {
			assert(false);
		}
		}
	}

	assert(node->right != nullptr);
	assert(node->type == ValueType::OPERATOR);

	// unary operators

	if (node->data == "-")
	{
		if (node->left == nullptr)
		{
			const NightData value = evaluate_expression(current_scope, node->right);
			if (!value.is_num())
				throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_expression, loc, "unary operator '-' can only be used on types 'int' or 'float'", "left have value of operator '-' is currently type '" + value.type.to_str() + "'");

			return value.type == VariableType::INT
				? NightData{ value.type, -std::get<int>(value.data) }
				: NightData{ value.type, -std::get<float>(value.data) };
		}

		const NightData value1 = evaluate_expression(current_scope, node->left);
		const NightData value2 = evaluate_expression(current_scope, node->right);

		if (!value1.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "binary operator '-' can only be used on types 'int' or 'float'", "left hand value of operator '-' is currently type '" + value2.type.to_str() + "'");
		if (!value2.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "binary operator '-' can only be used on types 'int' or 'float'", "right hand value of operator '-' is currently type '" + value2.type.to_str() + "'");

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
		const NightData value = evaluate_expression(current_scope, node->right);

		if (value.type != VariableType::BOOL)
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "operator '!' can only be used on type 'bool'", "operator  '!' is currently used on type '" + value.type.to_str() + "'");

		return NightData{ VariableType::BOOL, !std::get<bool>(value.data) };
	}
	if (node->data == "[]")
	{
		const NightData data_index = evaluate_expression(current_scope, node->extras[0]);
		if (data_index.type != VariableType::INT)
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_expression, loc, "index for subscript operator must be type 'int'", "index is currently type '" + data_index.to_str() + "'");

		const int index = std::get<int>(data_index.data);
		
		const NightData array = evaluate_expression(current_scope, node->right);
		if (array.type == VariableType::STRING)
		{
			if (index < 0 || index >= (int)std::get<std::string>(array.data).length())
				throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_expression, loc, "index for subscript operator is out of range", "index is value '" + std::to_string(index) + "' but string length is value '" + std::to_string(std::get<std::string>(array.data).length()) + "'");

			return NightData{ VariableType::STRING,
				std::string(1, std::get<std::string>(array.data)[(std::size_t)index]) };
		}
		else if (array.type == VariableType::ARRAY)
		{
			if (index < 0 || index >= (int)array.extras.size())
				throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_expression, loc, "index for subscript operator is out of range", "index is value '" + std::to_string(index) + "' but array length is value '" + std::to_string(array.extras.size()) + "'");

			return array.extras[(std::size_t)index];
		}
		else
		{
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "subscript operator can only be used on types 'str' or 'arr'", "subscript operator is currently used on type '" + array.type.to_str() + "'");
		}
	}

	// binary operators

	assert(node->left != nullptr);

	if (node->data == "+")
	{
		const NightData value1 = evaluate_expression(current_scope, node->left);
		const NightData value2 = evaluate_expression(current_scope, node->right);

		if (!value1.is_num() && value1.type != VariableType::STRING && value2.type != VariableType::STRING)
			throw NIGHT_COMPILE_ERROR("operator '+' can only be used on types 'int', 'float', or 'str'", "left hand value of operator '+' currently is type '" + value1.type.to_str() + "'", Learn::LEARN);
		if (!value2.is_num() && value2.type != VariableType::STRING && value1.type != VariableType::STRING)
			throw NIGHT_COMPILE_ERROR("operator '+' can only be used on types 'int', 'float', or 'str'", "right hand value of operator '+' currently is type '" + value2.type.to_str() + "'", Learn::LEARN);

		if (value1.type == VariableType::STRING || value2.type == VariableType::STRING)
		{
			return NightData{ VariableType::STRING,
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
		const NightData value1 = evaluate_expression(current_scope, node->left);
		const NightData value2 = evaluate_expression(current_scope, node->right);

		if (!value1.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "operator '*' can only be used on types 'int' or 'float'", "left hand value of operator '*' has type '" + value1.type.to_str() + "'");
		if (!value2.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "operator '*' can only be used on types 'int' or 'float'", "right hand value of operator '*' has type '" + value2.type.to_str() + "'");

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
		const NightData value1 = evaluate_expression(current_scope, node->left);
		const NightData value2 = evaluate_expression(current_scope, node->right);

		if (!value1.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "operator '/' can only be used on types 'int' or 'float'", "left hand value of operator '/' has type '" + value1.type.to_str() + "'");
		if (!value2.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "operator '/' can only be used on types 'int' or 'float'", "right hand value of operator '/' has type '" + value2.type.to_str() + "'");

		if (std::get<int>(value2.data) == 0)
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_expression, loc, "division by 0 is not allowed");

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
		const NightData value1 = evaluate_expression(current_scope, node->left);
		const NightData value2 = evaluate_expression(current_scope, node->right);

		if (!value1.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "operator '%' can only be used on types 'int' or 'float'", "left hand value of operator '%' has type '" + value1.type.to_str() + "'");
		if (!value2.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "operator '%' can only be used on types 'int' or 'float'", "right hand value of operator '%' has type '" + value2.type.to_str() + "'");

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
		const NightData value1 = evaluate_expression(current_scope, node->left);
		const NightData value2 = evaluate_expression(current_scope, node->right);

		if (!value1.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "operator '>' can only be used on types 'int' or 'float'", "left hand value of operator '>' has type '" + value1.type.to_str() + "'");
		if (!value2.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "operator '>' can only be used on types 'int' or 'float'", "right hand value of operator '>' has type '" + value2.type.to_str() + "'");

		return NightData{ VariableType::BOOL,
			value1.get_num() > value2.get_num() };
	}
	if (node->data == "<")
	{
		const NightData value1 = evaluate_expression(current_scope, node->left);
		const NightData value2 = evaluate_expression(current_scope, node->right);

		if (!value1.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "operator '<' can only be used on types 'int' or 'float'", "left hand value of operator '<' has type '" + value1.type.to_str() + "'");
		if (!value2.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "operator '<' can only be used on types 'int' or 'float'", "right hand value of operator '<' has type '" + value2.type.to_str() + "'");

		return NightData{ VariableType::BOOL, value1.get_num() < value2.get_num() };
	}
	if (node->data == ">=")
	{
		const NightData value1 = evaluate_expression(current_scope, node->left);
		const NightData value2 = evaluate_expression(current_scope, node->right);

		if (!value1.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "operator '>=' can only be used on types 'int' or 'float'", "left hand value of operator '>=' has type '" + value1.type.to_str() + "'");
		if (!value2.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "operator '>=' can only be used on types 'int' or 'float'", "right hand value of operator '>=' has type '" + value2.type.to_str() + "'");

		return NightData{ VariableType::BOOL, value1.get_num() >= value2.get_num() };
	}
	if (node->data == "<=")
	{
		const NightData value1 = evaluate_expression(current_scope, node->left);
		if (!value1.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "operator '<=' can only be used on types 'int' or 'float'", "left hand value of operator '<=' has type '" + value1.type.to_str() + "'");

		const NightData value2 = evaluate_expression(current_scope, node->right);
		if (!value2.is_num())
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "operator '<=' can only be used on types 'int' or 'float'", "right hand value of operator '<=' has type '" + value2.type.to_str() + "'");

		return NightData{ VariableType::BOOL, value1.get_num() <= value2.get_num() };
	}
	if (node->data == "||")
	{
		const NightData value1 = evaluate_expression(current_scope, node->left);
		if (value1.type != VariableType::BOOL)
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "operator '||' can only be used on type 'bool'", "left hand value of operator '||' has type '" + value1.type.to_str() + "'");

		const NightData value2 = evaluate_expression(current_scope, node->right);
		if (value2.type != VariableType::BOOL)
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "operator '||' can only be used on type 'bool'", "right hand value of operator '||' has type '" + value2.type.to_str() + "'");

		return NightData{ VariableType::BOOL,
			std::get<bool>(value1.data) || std::get<bool>(value2.data) };
	}
	if (node->data == "&&")
	{
		const NightData value1 = evaluate_expression(current_scope, node->left);
		const NightData value2 = evaluate_expression(current_scope, node->right);

		if (value1.type != VariableType::BOOL)
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "operator '&&' can only be used on type 'bool'", "left hand value of operator '||' has type '" + value1.type.to_str() + "'");
		if (value2.type != VariableType::BOOL)
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "operator '*' can only be used on type 'bool'", "right hand value of operator '||' has type '" + value2.type.to_str() + "'");

		return NightData{ VariableType::BOOL,
			std::get<bool>(value1.data) && std::get<bool>(value2.data) };
	}
	if (node->data == "==")
	{
		const NightData value1 = evaluate_expression(current_scope, node->left);
		const NightData value2 = evaluate_expression(current_scope, node->right);

		if (value1.type != value2.type)
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_expression, loc, "operator '==' can only be used on values with the same type", "left hand value has type '" + value1.type.to_str() + "' but right hand value has type '" + value2.type.to_str() + "'");

		return value1.type == VariableType::ARRAY
			? NightData{ VariableType::BOOL, compare_array(value1, value2) }
			: NightData{ VariableType::BOOL, value1.data == value2.data    };
	}
	if (node->data == "!=")
	{
		const NightData value1 = evaluate_expression(current_scope, node->left);
		const NightData value2 = evaluate_expression(current_scope, node->right);

		if (value1.type != value2.type)
			throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_expression, loc, "operator '==' can only be used on values with the same type", "left hand value currently is type '" + value1.type.to_str() + "' but right hand value currently is type '" + value2.type.to_str() + "'");

		return value1.type == VariableType::ARRAY
			? NightData{ VariableType::BOOL, !compare_array(value1, value2) }
			: NightData{ VariableType::BOOL, value1.data != value2.data    };
	}
	if (node->data == ".")
	{
		NightData object = evaluate_expression(current_scope, node->left);

		const Expression method = *(node->right);

		if (object.type == VariableType::ARRAY)
		{
			if (method.data == "len")
			{
				return NightData{ VariableType::INT, (int)object.extras.size() };
			}
			if (method.data == "push" && method.extras.size() == 1)
			{
				const NightData value = evaluate_expression(current_scope, method.extras[0]);
				object.extras.push_back(value);

				return object;
			}
			if (method.data == "push" && method.extras.size() == 2)
			{
				const NightData index = evaluate_expression(current_scope, method.extras[0]);
				const NightData value = evaluate_expression(current_scope, method.extras[1]);

				if (index.type != VariableType::INT)
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_expression, loc, "argument number '1' in function call '" + method.data + "' can only be type 'int'", "argument number '1' is currently type '" + index.to_str() + "'");

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
				const NightData index = evaluate_expression(current_scope, method.extras[0]);
				if (index.type != VariableType::INT)
					throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "index type is required to be type 'int'", 
						"index is currently type '" + index.type.to_str() + "'");

				object.extras.erase(object.extras.begin() + std::get<int>(index.data));

				return object;
			}

			assert(false && "method exists in Parser, but not Interpreter");
		}
		if (object.type == VariableType::STRING)
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