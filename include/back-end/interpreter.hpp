#pragma once

#include "stmt.hpp"
#include "token.hpp"

#include <memory>
#include <vector>
#include <unordered_map>

#define NIGHT_RUNTIME_ERROR(msg, fix, link) \
	night::error(Location{ __FILE__, __LINE__ }, night::error_runtime, loc, msg, fix, link);

class Interpreter
{
private:
	struct Data
	{
		enum T {
			BOOL,
			INT, FLOAT, // DOUBLE, INT4, INT16, INT32
			STR, ARR
		} type;

		std::variant<
			bool,
			int, float,
			std::string,
			std::vector<Data>
		> data;

		bool is_num() const;
		std::string to_str() const;

		// if type is an integer or float, return data
		//
		// use only if type could be an int or float
		//
		// if type is CERTAINLY an int, then use std::get<int>()
		template <typename T>
		T get_num() const;
	};

	struct NightVariable;
	using InterpreterScope = Scope<NightVariable>;

public:
	std::optional<Data> interpret_statement(
		InterpreterScope& scope,
		Stmt const& stmt
	);

private:
	Data evaluate_expression(
		InterpreterScope& scope,
		std::shared_ptr<ExprNode> const& expr
	);

	template <typename Operation>
	Data eval_binary_num_expr(
		InterpreterScope& scope,
		BinaryOPNode const& binary_op,
		Operation const& operation
	);

	template <typename Operation>
	Data eval_binary_bool_expr(
		InterpreterScope& scope,
		BinaryOPNode const& binary_op,
		std::string const& used_types,
		Operation const& operation
	);

private:
	struct NightVariable
	{
		Data data;
	};

	struct NightFunction
	{
		std::vector<std::string> params;
		std::vector<Stmt> body;
	};

	using NightVariableContainer = std::unordered_map<std::string, NightVariable>;
	using NightFunctionContainer = std::unordered_map<std::string, NightFunction>;

private:
	NightFunctionContainer night_funcs;
};