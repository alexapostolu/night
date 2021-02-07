#pragma once

#include "night.hpp"
#include "token.hpp"

#include <memory>
#include <vector>
#include <unordered_map>

class Interpreter
{
public:
	Interpreter(
		std::shared_ptr<NightScope>& current_scope,
		const std::vector<Statement>& statements,
		NightData* return_value = nullptr
	);

private:
	NightData EvaluateExpression(
		std::shared_ptr<NightScope>& current_scope,
		const std::shared_ptr<Expression>& node // raw pointer?
	);

	std::unordered_map<std::string, NightData>::iterator get_variable(
		std::shared_ptr<NightScope>& scope,
		const std::string& variable_name
	);

private:
	static std::vector<NightFunction> night_functions;

	// exit function if a return statement is encountered
	static bool exit_function;
};