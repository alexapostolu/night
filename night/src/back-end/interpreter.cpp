#include "../../include/back-end/interpreter.hpp"
#include "../../include/back-end/utils.hpp"
#include "../../include/back-end/token.hpp"
#include "../../include/back-end/stmt.hpp"
#include "../../include/error.hpp"

#include <iostream>
#include <stdexcept>
#include <functional>
#include <memory>
#include <optional>
#include <cmath>
#include <string>
#include <vector>

// std::optional
Interpreter::Data Interpreter::interpret_statement(
	InterpreterScope& scope,
	Stmt const& stmt)
{
	switch (stmt.type)
	{
	case StmtType::ASSIGN: {
		StmtAssign const& stmt_assign = std::get<StmtAssign>(stmt.data);

		auto const night_var   = scope.get_var(stmt_assign.var_name);
		Data const assign_data = evaluate_expression(scope, stmt_assign.assign_expr);

		switch (stmt_assign.type)
		{
		case StmtAssign::ASSIGN: {
			if (stmt_assign.subscript_chain.size() == 0)
			{
				night_var->second.data = assign_data;
				return std::nullopt;
			}

			if (night_var->second.data.type == Data::STR)
			{
				if (stmt_assign.subscript_chain.size() > 1) {
					throw NIGHT_RUNTIME_ERROR(
						"subscript operator can not be used on letter value",
						"",
						night::learn_strings);
				}

				if (night_var->second.data.type != Data::STR) {
					throw NIGHT_RUNTIME_ERROR("");
				}

				std::string assign_str =
					std::get<std::string>(night_var->second.data.val);

				assign_str
				
				night_var->second.data.val = std::;
				return std::nullopt;
			}
			else if (curr.type == Data::ARR)
			{
				curr = std::get<std::vector<Data> >(curr.data);
			}

			for (auto const& index : stmt_assign.subscript_chain)
			{
				if (curr.type == Data::STR)
				{

				}
				else if (curr.type == Data::ARR)
				{
					curr = std::get<std::vector<Data> >(curr.data);
				}
				
				throw NIGHT_RUNTIME_ERROR();
			}
			night_var->second.data = evaluate_expression(scope, stmt_assign.assign_expr);
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
		case StmtAssign::MINUS: {
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
		case StmtAssign::TIMES: {
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
		case StmtAssign::DIVIDE: {
			if (!night_variable->second.is_num())
				throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "variable '" + night_variable->first + "' can not be assigned using the assignment operator '/='", "the operator can only be used on variables of type 'int' or float'");
			if (!assign_expr.is_num())
				throw RuntimeError(__FILE__, __LINE__, RuntimeError::invalid_type, loc, "expression can not be used with the operator '/='", "the expression contains type '" + assign_expr.type.to_str() + "'; however, the operator '/=' can only assign expressions of type 'int' or 'float'");

			if (night_variable->second.type == VariableType::INT)
				std::get<int>(night_variable->second.data) /= (int)assign_expr.get_num();
			else
				std::get<float>(night_variable->second.data) /= assign_expr.get_num();

			break;
		}
		case StmtAssign::MOD: {
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
		StmtMethod const& method_call = std::get<StmtMethod>(stmt.data);

		auto* const night_variable = get_variable(current_scope, method_call.);
		assert(night_variable != nullptr);

		night_variable->second = evaluate_expression(current_scope, method_call.assign_expr);

		break;
	}
	default: {
		return;
		// return std::nullopt;
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
	
		auto const night_func = night_funcs.find(val.name);
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

			if (param.type == Data::INT)
			{
				return Data{ Data::INT, std::get<int>(param.data) };
			}
			if (param.type == Data::FLOAT)
			{
				return Data{ Data::FLOAT, std::get<float>(param.data) };
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
					"function '" + val.name + "', argument 1, is type '" + start_d.to_str() + "'",
					"argument can only be type 'int'",
					night::learn_functions);
			}

			Data const end_d = evaluate_expression(scope, val.param_exprs.at(1));
			if (end_d.type != Data::INT) {
				throw NIGHT_RUNTIME_ERROR(
					"function '" + val.name + "', argument 2, is type '" + end_d.to_str() + "'",
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
			"function call '" + val.name + "' does not return a value in expression",
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

		switch (binary_op.type)
		{
		case BinaryOPNode::PLUS: {
			Data const left = evaluate_expression(scope, binary_op.left);
			Data const right = evaluate_expression(scope, binary_op.right);

			if (!left.is_num() && left.type != Data::STR && right.type != Data::STR) {
				throw NIGHT_RUNTIME_ERROR(
					"operator '+' can only be used on types 'int', 'float', or 'str'",
					"left hand value of operator '+' currently is type '" + left.to_str() + "'",
					night::learn_learn);
			}

			if (!right.is_num() && right.type != Data::STR&& left.type != Data::STR) {
				throw NIGHT_RUNTIME_ERROR(
					"operator '+' can only be used on types 'int', 'float', or 'str'",
					"right hand value of operator '+' currently is type '" + right.to_str() + "'",
					night::learn_learn);
			}

			if (left.type == Data::STR|| right.type == Data::STR)
			{
				return Data{ Data::STR,
					left.to_str() + right.to_str() };
			}

			if (left.type == Data::INT && right.type == Data::INT)
			{
				return Data{ Data::INT,
					std::get<int>(left.data) + std::get<int>(right.data) };
			}
			if (left.type == Data::FLOAT && right.type == Data::FLOAT)
			{
				return Data{ Data::FLOAT,
					std::get<float>(left.data) + std::get<float>(right.data) };
			}
			if (left.type == Data::FLOAT)
			{
				return Data{ Data::FLOAT,
					std::get<float>(left.data) + std::get<int>(right.data) };
			}
			if (right.type == Data::FLOAT)
			{
				return Data{ Data::FLOAT,
					std::get<int>(left.data) + std::get<float>(right.data) };
			}
		}

		case BinaryOPNode::MINUS: {
			return eval_expr_binary_num(scope, binary_op,
				[](auto x, auto y) { return x - y; });
		}
		case BinaryOPNode::TIMES: {
			return eval_expr_binary_num(scope, binary_op,
				[](auto x, auto y) { return x * y; });
		}
		case BinaryOPNode::DIVIDE: {
			return eval_expr_binary_num(scope, binary_op,
				[](auto x, auto y) { return x / y; });
		}
		case BinaryOPNode::MOD: {
			return eval_expr_binary_num(scope, binary_op,
				[](auto x, auto y) { return std::fmod(x % y); });
		}

		case BinaryOPNode::GREATER: {
			return eval_expr_binary_num_comp(scope, binary_op,
				[](auto x, auto y) { return x > y; });
		}
		case BinaryOPNode::SMALLER: {
			return eval_expr_binary_num_comp(scope, binary_op,
				[](auto x, auto y) { return x < y; });
		}
		case BinaryOPNode::GREATER_EQ: {
			return eval_expr_binary_num_comp(scope, binary_op,
				[](auto x, auto y) { return x >= y; });
		}
		case BinaryOPNode::SMALLER_EQ: {
			return eval_expr_binary_num_comp(scope, binary_op,
				[](auto x, auto y) { return x <= y; });
		}

		case BinaryOPNode::OR: {
			return eval_expr_binary_bool(scope, binary_op,
				[](bool left, bool right) { return left || right; });
		}
		case BinaryOPNode::AND: {
			return eval_expr_binary_bool(scope, binary_op,
				[](bool left, bool right) { return left && right; });
		}

		case BinaryOPNode::EQUAL: {
			return eval_expr_binary_comp(scope, binary_op,
				[](auto left, auto right) { return left == right; });
		}
		case BinaryOPNode::NOT_EQUAL: {
			return eval_expr_binary_comp(scope, binary_op,
				[](auto left, auto right) { return left != right; });
		}
		case BinaryOPNode::DOT: {
			Data object = evaluate_expression(scope, binary_op.left);
			if (object.type != Data::STR && object.type != Data::ARR) {
				throw NIGHT_RUNTIME_ERROR(
					"operator '" + binary_op.data + "' can only be used on objects",
					"operator is currently used on type '" + object.to_str() + "'",
					night::learn_classes);
			}

			if (binary_op.right->type == ExprNode::CALL) {
				throw NIGHT_RUNTIME_ERROR(
					"operator '" + binary_op.data + "' can only be used on objects",
					"operator is currently used on type '" + object.to_str() + "'",
					night::learn_classes);
			}

			ValueCall const& method = std::get<ValueCall>(binary_op.right->data);

			if (object.type == Data::ARR)
			{
				std::vector<Data>& obj_arr =
					std::get<std::vector<Data> >(object.data);

				if (method.name == "len")
				{
					return Data{ Data::INT, (int)obj_arr.size() };
				}
				if (method.name == "push" && method.param_exprs.size() == 1)
				{
					Data const value = evaluate_expression(scope, method.param_exprs[0]);
					obj_arr.push_back(value);

					return object;
				}
				if (method.name == "push" && method.param_exprs.size() == 2)
				{
					Data const value = evaluate_expression(scope, method.param_exprs[0]);
					Data const index = evaluate_expression(scope, method.param_exprs[1]);

					if (index.type != Data::INT) {
						throw NIGHT_RUNTIME_ERROR(
							"function call '" + method.name + "', argument number '2' can only be type 'int'",
							"argument is currently type '" + index.to_str() + "'",
							night::learn_arrays);
					}

					obj_arr.insert(obj_arr.begin() + std::get<int>(index.data), value);

					return object;
				}
				if (method.name == "pop" && method.param_exprs.empty())
				{
					obj_arr.pop_back();
					return object;
				}
				if (method.name == "pop" && !method.param_exprs.size() == 1)
				{
					Data const index = evaluate_expression(scope, method.param_exprs[0]);
					if (index.type != Data::INT) {
						throw NIGHT_RUNTIME_ERROR(
							"index type is required to be type 'int'",
							"index is currently type '" + index.to_str() + "'",
							night::learn_arrays);
					}

					obj_arr.erase(obj_arr.begin() + std::get<int>(index.data));

					return object;
				}

				assert(false && "method exists in Parser, but not Interpreter");
			}
			if (object.type == Data::STR)
			{
				if (method.name == "len")
				{
					return Data{ Data::INT,
						(int)std::get<std::string>(object.data).length() };
				}

				assert(false && "method exists in Parser, but not Interpreter");
			}
		}
		}
	}
	}
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
Interpreter::Data Interpreter::eval_expr_binary_num(
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

	if (lhs.type == Data::INT && lhs.type == Data::INT)
		return Data{ Data::INT, operation(std::get<int>(lhs.data), std::get<int>(rhs.data)) };
	else
		return Data{ Data::FLOAT, operation(lhs.get_num(), rhs.get_num()) };
}

template <typename Operation>
Interpreter::Data Interpreter::eval_expr_binary_num_comp(
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

	return Data{ Data::BOOL, operation(lhs.get_num(), rhs.get_num()) };
}

template <typename Operation>
Interpreter::Data Interpreter::eval_expr_binary_bool(
	InterpreterScope& scope,
	BinaryOPNode const& binary_op,
	Operation const& operation
)
{
	Data const left = evaluate_expression(scope, binary_op.left);
	if (left.type != Data::BOOL) {
		throw NIGHT_RUNTIME_ERROR(
			"left hand value of operator '" + binary_op.data + "' has type '" + left.to_str() + "'",
			"operator can only be used on type 'bool'",
			night::learn_learn);
	}

	Data const right = evaluate_expression(scope, binary_op.right);
	if (right.type != Data::BOOL) {
		throw NIGHT_RUNTIME_ERROR(
			"right hand value of operator '" + binary_op.data + "' has type '" + right.to_str() + "'",
			"operator can only be used on type 'bool'",
			night::learn_learn);
	}

	return Data{ Data::BOOL,
		operation(std::get<bool>(left), std::get<bool>(right)) };
}

template <typename Operation>
Interpreter::Data Interpreter::eval_expr_binary_comp(
	InterpreterScope& scope,
	BinaryOPNode const& binary_op,
	Operation const& operation
)
{
	Data const left  = evaluate_expression(scope, binary_op.left);
	Data const right = evaluate_expression(scope, binary_op.right);

	if (left.type != right.type) {
		throw NIGHT_COMPILE_ERROR(
			"operator '" + binary_op.data + "' can only be used on values with the same type",
			"left hand value has type '" + left.type.to_str() + "' but right hand value has type '" + right.type.to_str() + "'",
			night::learn_learn);
	}

	return left.type == Data::ARR
		? Data{ Data::BOOL, compare_array(left, right) }
		: Data{ Data::BOOL, operation(left.data, right.data) };
}