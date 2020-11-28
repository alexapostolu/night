#pragma once

#include "token.h"
#include "utils.h"

#include <vector>
#include <string>

#define CHECK_EXPR(check_type, rtn_type, expr)                                                                   \
	assert(node->left != nullptr && node->right != nullptr && "binary operator should have two non-NULL nodes"); \
																												 \
	const Expression left  = TypeCheckExpression(file, line, node->left,  variables, functions);				 \
	const Expression right = TypeCheckExpression(file, line, node->right, variables, functions);			     \
																												 \
	if (left.type != check_type)																				 \
		OperatorError(file, line, node->data, left, night::ttos(check_type));									 \
	if (right.type != check_type)																		         \
		OperatorError(file, line, node->data, right, night::ttos(check_type));									 \
																					             				 \
	return Expression{ rtn_type, expr }

inline void OperatorError(
	const std::string& file,
	const int          line,

	const std::string& op,
	const Expression& node,

	const std::string& usedOn
);

// turns a value into an expression
std::shared_ptr<Expression> new_expression(
	const std::string& file,
	const int          line,

	const Value& value,
	const std::shared_ptr<Expression>& left,
	const std::shared_ptr<Expression>& right,

	const std::vector<Variable>&    variables,
	const std::vector<FunctionDef>& functions
);

// turns an array of tokens to an array of extras
std::vector<Value> TokensToValues(
	const std::vector<Token>& tokens,

	const std::vector<Variable>&    variables,
	const std::vector<FunctionDef>& functions,

	int* arrayDepth = nullptr
);

// type checks expression
Expression TypeCheckExpression(
	const std::string& file,
	const int          line,

	const std::shared_ptr<Expression>& node,

	const std::vector<Variable>&    variables,
	const std::vector<FunctionDef>& functions
);

// returns the operator precedence
int GetOperatorPrecedence(
	const ValueType& type,
	const std::string& value
);

// get next group of values
std::shared_ptr<Expression> GetNextGroup(
	const std::string& file,
	const int          line,

	const std::vector<Value>& values,
	std::size_t& index,

	const std::vector<Variable>&    variables,
	const std::vector<FunctionDef>& functions
);

// turns an array of values into an AST
std::shared_ptr<Expression> ParseValues(
	const std::string& file,
	const int          line,
	
	const std::vector<Value>& values,

	const std::vector<Variable>&    variables,
	const std::vector<FunctionDef>& functions
);

// parses an array of tokens into an array of statements
void Parser(
	std::vector<Statement>& statements,
	const std::vector<Token>& tokens,

	bool inFunction = false
);