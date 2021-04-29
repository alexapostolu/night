#pragma once

#include "night.hpp"
#include "token.hpp"

#include <memory>
#include <vector>
#include <unordered_map>

struct Interpreter
{
	void Interpret(
		std::shared_ptr<NightScope>& current_scope,
		const std::vector<Statement>& _stmts,
		NightData* return_value = nullptr
	);

	NightData evaluate_expression(
		std::shared_ptr<NightScope>& current_scope,
		const std::shared_ptr<Expression>& node // raw pointer?
	);

	std::pair<const std::string, NightData>* get_variable(
		std::shared_ptr<NightScope>& scope,
		const std::string& variable_name
	);

	static NightFunctionContainer night_functions;

	// exit function if a return statement is encountered
	bool exit_function;
};

void interpret_statement(
	const std::shared_ptr<Scope>& curr_scope,
	const Statement& stmt//,
	// return_value?
);