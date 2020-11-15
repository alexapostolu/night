#pragma once

#include "night.h"
#include "token.h"

#include <vector>

// "mAcRoS aRe BaD, dOn'T uSe ThEm, reeeeeeee" - haha macro go brrrrrrrrrrrr
#define EVAL_EXPR(expr) const Expression expr1 = EvaluateExpression(node->left, variables, functions);  \
						const Expression expr2 = EvaluateExpression(node->right, variables, functions); \
						return Expression{ expr1.type, expr };

// evaluates an expression
Expression EvaluateExpression(const Expression* node, std::vector<NightVariable>& variables, std::vector<NightFunction>& functions);

// interprets expressions
void Interpreter(const std::vector<Statement>& statements, Expression* returnValue = nullptr);