#pragma once

#include "night.h"
#include "token.h"

#include <memory>
#include <vector>

// "mAcRoS aRe BaD, dOn'T uSe ThEm, reeeeeeee" - haha macro go brrrrrrrrrrrr
#define EVAL_EXPR(expr, rtn)														\
	const Expression expr1 = EvaluateExpression(node->left,  variables, functions); \
	const Expression expr2 = EvaluateExpression(node->right, variables, functions); \
	return Expression{ rtn, expr };

// evaluates an expression
Expression EvaluateExpression(
	const std::shared_ptr<Expression>& node,

	std::vector<NightVariable>&       variables, // can't be const due to function parameters
	const std::vector<NightFunction>& functions
);

// interprets expressions
void Interpreter(
	const std::vector<Statement>& statements,
	Expression* returnValue = nullptr
);