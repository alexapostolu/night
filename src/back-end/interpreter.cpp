#include "../../include/back-end/interpreter.hpp"
#include "../../include/back-end/utils.hpp"
#include "../../include/back-end/token.hpp"
#include "../../include/back-end/stmt.hpp"
#include "../../include/error.hpp"

#include <iostream>
#include <stdexcept>
#include <memory>
#include <optional>
#include <cmath>
#include <string>
#include <vector>

std::optional<Interpreter::Data> Interpreter::interpret_statement(
	InterpreterScope& scope,
	Stmt const& stmt)
{
	switch (stmt.type)
	{
	case StmtType::ASSIGN: {
		StmtAssign const& stmt_assign = std::get<StmtAssign>(stmt.data);

		Data const assign_expr = evaluate_expression(stmt_assign.assign_expr);

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

		return std::nullopt;
	}
	case StmtType::IF: {
		StmtIf const& stmt_if = std::get<StmtIf>(stmt.data);

		for (const Conditional& conditional : stmt_if.chains)
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
	case StmtType::FN: {
		StmtFn const& fn_stmt = std::get<StmtFn>(stmt->data);

		night_funcs.push_back(NightFunction{
			fn_stmt.params,
			fn_stmt.body
		});

		return std::nullopt;
	}
	case StmtType::CALL: {
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
		StmtReturn const& rtn_stmt = std::get<StmtReturn>(stmt.data);

		if (rtn_stmt.expr != nullptr)
			return evaluate_expression(scope, rtn_stmt.expr);
		else
			return std::nullopt;
	}
	case StmtType::WHILE: {
		WhileLoop const& while_loop = std::get<WhileLoop>(stmt.data);

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

		return std::nullopt;
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

Interpreter::Data Interpreter::evaluate_expression(
	InterpreterScope& scope,
	std::shared_ptr<ExprNode> const& expr)
{
	Location const& loc = expr->loc;

	switch (expr->type)
	{
	case ExprNode::LITERAL: {
		ValueLiteral const& val = std::get<ValueLiteral>(expr->data);
		switch (val.type)
		{
		case ValueLiteral::BOOL:
			return { Data::BOOL, std::get<bool>(val.data) };
		case ValueLiteral::INT:
			return { Data::INT, std::get<int>(val.data) };
		case ValueLiteral::FLOAT:
			return { Data::FLOAT, std::get<float>(val.data) };
		case ValueLiteral::STR:
			return { Data::STR, std::get<std::string>(val.data) };
		}
	}
	case ExprNode::ARRAY: {
		ValueArray const& val = std::get<ValueArray>(expr->data);

		std::vector<Data> elem_data(val.elem_exprs.size());
		for (std::size_t a = 0; a < val.elem_exprs.size(); ++a)
			elem_data[a] = evaluate_expression(scope, val.elem_exprs[a]);

		return { Data::ARR, elem_data };
	}
	case ExprNode::VARIABLE: {
		ValueVar const& val = std::get<ValueVar>(expr->data);

		auto* const night_var = scope.get_var(val.name);
		assert(night_var != nullptr);

		return night_var->second.data;
	}
	case ExprNode::CALL: {
		ValueCall const& val = std::get<ValueCall>(expr->data);
	
		auto const night_func = night_funcs.find(val.func_name);
		assert(night_func != night_funcs.end());

		if (night_func->first == "input")
		{
			std::string user_input;
			getline(std::cin, user_input);

			return Data{ Data::STR, user_input };
		}
		if (night_func->first == "int")
		{
			Data const param = evaluate_expression(scope, val.param_exprs.at(0));

			if (param.type == Data::INT)
			{
				return param;
			}
			if (param.type == Data::FLOAT)
			{
				return Data{ Data::INT, (int)std::get<float>(param.data) };
			}
			if (param.type == Data::STR)
			{
				try {
					return Data{ Data::INT, std::stoi(std::get<std::string>(param.data)) };
				}
				catch (std::invalid_argument const&) {
					throw NIGHT_RUNTIME_ERROR(
						"function call 'int', argument number 1, cannot be converted into type 'int'",
						"argument can only be a number in the form of a string",
						night::learn_functions);
				}
			}

			throw NIGHT_RUNTIME_ERROR(
				"function call 'int', argument number 1, is currently type '" + param.to_str() + "'",
				"argument can only be types 'int', 'float', or 'str'",
				night::learn_functions);
		}
		if (night_func->first == "float")
		{
			Data const param = evaluate_expression(scope, val.param_exprs.at(0));

			if (param.type == Data::INT || param.type == Data::FLOAT)
			{
				return Data{ Data::FLOAT, param.get_num() };
			}
			if (param.type == Data::STR)
			{
				try {
					return Data{ Data::FLOAT, std::stof(std::get<std::string>(param.data)) };
				}
				catch (std::invalid_argument const&) {
					throw NIGHT_RUNTIME_ERROR(
						"function call 'float', argument number 1, cannot be converted into type 'float'",
						"argument can only be a number in the form of a string",
						night::learn_functions);
				}
			}

			throw NIGHT_RUNTIME_ERROR(
				"function call 'int', argument number 1, is currently type '" + param.to_str() + "'",
				"argument can only be types 'int', 'float', or 'str'",
				night::learn_functions);
		}
		if (night_func->first == "range")
		{
			Data const start_d = evaluate_expression(scope, val.param_exprs.at(0));
			if (start_d.type != Data::INT) {
				throw NIGHT_RUNTIME_ERROR(
					"function '" + val.func_name + "', argument 1, is type '" + start_d.to_str() + "'",
					"argument can only be type 'int'",
					night::learn_functions);
			}

			Data const end_d = evaluate_expression(scope, val.param_exprs.at(1));
			if (end_d.type != Data::INT) {
				throw NIGHT_RUNTIME_ERROR(
					"function '" + val.func_name + "', argument 2, is type '" + end_d.to_str() + "'",
					"argument can only be type 'int'",
					night::learn_functions);
			}

			int const start = std::get<int>(start_d.data);
			int const end	= std::get<int>(end_d.data);

			std::vector<Data> elems(end - start + 1);
			for (std::size_t a = start; a <= end; ++a)
				elems[a - start] = Data{ Data::INT, (int)a };

			return Data{ Data::ARR, elems };
		}

		// turn parameters into variables
		for (std::size_t a = 0; a < night_func->second.params.size(); ++a)
		{
			scope.vars[night_func->second.params[a]] =
				NightVariable{ evaluate_expression(scope, val.param_exprs.at(a)) };
		}

		for (Stmt const& stmt : night_func->second.body)
		{
			std::optional<Data> rtn_val = interpret_statement(scope, stmt);
			if (rtn_val.has_value())
				return rtn_val.value();
		}

		throw NIGHT_RUNTIME_ERROR(
			"function call '" + val.func_name + "' does not return a value in expression",
			"functions must return a value when used in an expression",
			night::learn_functions);
	}
	case ExprNode::UNARY_OP: {
		UnaryOPNode const& unary_op = std::get<UnaryOPNode>(expr->data);
		
		if (unary_op.data == "-")
		{
			Data const value = evaluate_expression(scope, unary_op.value);
			if (!value.is_num()) {
				throw NIGHT_RUNTIME_ERROR(
					"left have value of operator '-' is currently type '" + value.to_str() + "'",
					"unary operator '-' can only be used on types 'int' or 'float'",
					night::learn_learn);
			}

			return value.type == Data::INT
				? Data{ value.type, -std::get<int>(value.data) }
				: Data{ value.type, -std::get<float>(value.data) };
		}
		if (unary_op.data == "!")
		{
			Data const value = evaluate_expression(scope, unary_op.value);

			if (value.type != Data::BOOL) {
				throw NIGHT_RUNTIME_ERROR(
					"operator  '!' is currently used on type '" + value.to_str() + "'",
					"operator '!' can only be used on type 'bool'",
					night::learn_learn);
			}

			return Data{ Data::BOOL, !std::get<bool>(value.data) };
		}
		if (unary_op.data == "[]")
		{
			Data const index_d = evaluate_expression(scope, unary_op.index);
			if (index_d.type != Data::INT) {
				throw NIGHT_RUNTIME_ERROR(
					"index for subscript operator must be type 'int'",
					"index is currently type '" + index_d.to_str() + "'",
					night::learn_learn);
			}

			int const index = std::get<int>(index_d.data);

			Data const array = evaluate_expression(scope, unary_op.value);
			if (array.type == Data::STR)
			{
				std::string const& str = std::get<std::string>(array.data);

				if (index < 0 || index >= (int)str.length()) {
					throw NIGHT_RUNTIME_ERROR(
						"index for subscript operator is out of range",
						"index " + std::to_string(index) + " is out of range for string length " + std::to_string(std::get<std::string>(array.data).length()) + "",
						night::learn_learn);
				}

				return Data{ Data::STR, std::string(1, str[(std::size_t)index]) };
			}
			if (array.type == Data::ARR)
			{
				std::vector<Data> const& arr = std::get<std::vector<Data>>(array.data);

				if (index < 0 || index >= (int)arr.size()) {
					throw NIGHT_RUNTIME_ERROR(
						"index for subscript operator is out of range",
						"index is value '" + std::to_string(index) + "' but array length is value '" + std::to_string(arr.size()) + "'",
						night::learn_learn);
				}

				return arr[(std::size_t)index];
			}
			
			throw NIGHT_RUNTIME_ERROR(
				"subscript operator can only be used on types 'str' or 'arr'",
				"subscript operator is currently used on type '" + array.to_str() + "'",
				night::learn_learn);
		}
	}
	case ExprNode::BINARY_OP: {
		BinaryOPNode const& binary_op = std::get<BinaryOPNode>(expr->data);

		if (binary_op.data == "-")
		{
			auto const [lhs, rhs] = eval_binary_expr(scope, );
			return Data{ Data::INT, lhs.get_num() - rhs.get_num() };
		}
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
			return eval_binary_num_expr(scope, binary_op,
				[](int x, int y) { return x * y });
		}
		if (node->data == "/")
		{
			return eval_binary_num_expr(scope, binary_op,
				[](int x, int y) { return x / y });
		}
		if (node->data == "%")
		{
			return eval_binary_num_expr(scope, binary_op,
				[](int x, int y) { return std::fmod(x % y) });
		}
		if (node->data == ">")
		{
			return eval_binary_bool_expr(scope, binary_op,
				[](int x, int y) { return x > y) });
		}
		if (node->data == "<")
		{
			return eval_binary_bool_expr(scope, binary_op,
				[](int x, int y) { return x < y) });
		}
		if (node->data == ">=")
		{
			return eval_binary_bool_expr(scope, binary_op,
				[](int x, int y) { return x >= y) });
		}
		if (node->data == "<=")
		{
			return eval_binary_bool_expr(scope, binary_op,
				[](int x, int y) { return x <= y) });
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
			: NightData{ VariableType::BOOL, value1.data == value2.data };
		}
		if (node->data == "!=")
		{
			const NightData value1 = evaluate_expression(current_scope, node->left);
			const NightData value2 = evaluate_expression(current_scope, node->right);

			if (value1.type != value2.type)
				throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_expression, loc, "operator '==' can only be used on values with the same type", "left hand value currently is type '" + value1.type.to_str() + "' but right hand value currently is type '" + value2.type.to_str() + "'");

			return value1.type == VariableType::ARRAY
				? NightData{ VariableType::BOOL, !compare_array(value1, value2) }
			: NightData{ VariableType::BOOL, value1.data != value2.data };
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
	}
	}
}

template <typename T>
T Interpreter::Data::get_num() const
{
	if (type == Data::INT)
		return std::get<int>(data);
	else
		return std::get<float>(data);
}

std::string Interpreter::Data::to_str() const
{
	switch (type)
	{
	case T::BOOL:
		return "bool";
	case T::INT:
		return "int";
	case T::FLOAT:
		return "float";
	case T::STR:
		return "str";
	case T::ARR:
		return "arr";
	}
}

template <typename Operation>
Interpreter::Data Interpreter::eval_binary_expr(
	InterpreterScope& scope,
	BinaryOPNode const& binary_op,
	Operation const& operation)
{
	Data const lhs = evaluate_expression(scope, binary_op.left);
	if (!lhs.is_num()) {
		throw NIGHT_RUNTIME_ERROR(
			"binary operator '" + binary_op.data + "' can only be used on types 'int' or 'float'",
			"left hand value of operator is currently type '" + lhs.to_str() + "'",
			night::learn_learn);
	}

	Data const rhs = evaluate_expression(scope, binary_op.right);
	if (!rhs.is_num()) {
		throw NIGHT_RUNTIME_ERROR(
			"binary operator '" + binary_op.data + "' can only be used on types 'int' or 'float'",
			"right hand value of operator is currently type '" + lhs.to_str() + "'",
			night::learn_learn);
	}


	return Data{ Data::INT, operation() }

	if (lhs.type == Data::INT && lhs.type == Data::INT)
		return Data{ Data::INT, operation(std::get<int>(lhs.data), std::get<int>(rhs.data)) };
	else
		return Data{ Data::FLOAT, operation(lhs.get_num(), rhs.get_num()) };
}