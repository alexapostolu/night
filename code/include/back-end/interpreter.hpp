#pragma once

#include "stmt.hpp"
#include "token.hpp"

#include <memory>
#include <optional>
#include <vector>
#include <unordered_map>

#define NIGHT_RUNTIME_ERROR(msg, fix) \
	night::error(__FILE__, __LINE__, night::error_runtime, loc, msg, fix);

class Interpreter
{
private:
	struct Data
	{
		enum T {
			//in_CHAR, // for internal use only - makes type checking string operations easier :)
			BOOL,
			INT, FLOAT, // DOUBLE, INT4, INT16, INT32
			STR, ARR
		} type;

		std::variant<
			//char,
			bool,
			int, float,
			std::string,
			std::vector<Data>
		> val;

		bool is_num() const;
		std::string to_str() const;

		static bool compare_data(Data const& data1, Data const& data2);
		static bool compare_array(Data const& data1, Data const& data2);

		static void print(Data const& data);
	};

public:
	Interpreter();

private:
	struct NightVariable;

public:
	using InterpreterScope = Scope<NightVariable>;
	using NightVariableContainer = std::unordered_map<std::string, NightVariable>;

	std::optional<Data> interpret_statements(
		InterpreterScope& upper_scope,
		std::vector<Stmt> const& stmts,
		NightVariableContainer const& vars = {}
	);

	std::optional<Data> interpret_statement(
		InterpreterScope& scope,
		Stmt const& stmt
	);

private:
	std::optional<std::pair<Data*, Data>> interpret_subscript_chain(
		InterpreterScope& scope,
		StmtAssign const& stmt_assign,
		Location const& loc);

	template <typename Operation>
	void interpret_assignment(
		Data* const curr_data,
		Data const& assign_data,
		std::string const& op,
		Operation assign,
		Location const& loc);

	// turns arguments into variables
	NightVariableContainer interpret_arguments(
		InterpreterScope& scope,
		std::vector<std::string> const& param_names,
		ExprContainer const& param_exprs
	);

	Data evaluate_expression(
		InterpreterScope& scope,
		std::shared_ptr<ExprNode> const& expr
	);

	template <typename Operation>
	Data eval_expr_binary_num(
		InterpreterScope& scope,
		BinaryOPNode const& binary_op,
		Operation const& operation,
		bool num_rtn_type
	);

	template <typename Operation>
	Data eval_expr_binary_bool(
		InterpreterScope& scope,
		BinaryOPNode const& binary_op,
		Operation const& operation
	);

	bool eval_expr_binary_comp(
		InterpreterScope& scope,
		BinaryOPNode const& binary_op
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

	using NightFunctionContainer = std::unordered_map<std::string, NightFunction>;

private:
	NightFunctionContainer night_funcs;
	static std::optional<std::pair<int, int> > pair_range;
};