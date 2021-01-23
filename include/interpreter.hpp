#pragma once

#include "night.hpp"
#include "token.hpp"

#include <memory>
#include <vector>

class Interpreter
{
public:
	Interpreter(
		const std::vector<Statement>& statements,
		NightData* return_value = nullptr
	);

private:
	NightData EvaluateExpression(
		const std::shared_ptr<Expression>& node // raw pointer?
	);

private:
	static std::vector<NightVariable> night_variables;
	static std::vector<NightFunction> night_functions;
	static std::vector<NightClass>    night_classes;
};