#pragma once

#include "night.hpp"
#include "token.hpp"

#include <memory>
#include <vector>

// "mAcRoS aRe BaD, dOn'T uSe ThEm, reeeeeeee" - haha macro go brrrrrrrrrrrr
#define EVAL_EXPR(expr, rtn)											     	   \
	const NightData expr1 = EvaluateExpression(node->left,  variables, functions); \
	const NightData expr2 = EvaluateExpression(node->right, variables, functions); \
	return NightData{ rtn, expr };

// evaluates an expression
NightData EvaluateExpression(
	const std::shared_ptr<Expression>&node,

	std::vector<NightVariable>&variables, // can't be const due to function parameters
	const std::vector<NightFunction>&functions,

	// removes variables on return statements to prevent recursion errors
	bool remove_variables = false
);

// interprets expressions
void Interpreter(
	const std::vector<Statement>& statements,
	NightData* returnValue = nullptr
);