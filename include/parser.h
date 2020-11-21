#pragma once

#include "token.h"
#include "utils.h"

#include <vector>
#include <string>

// macros are awesome!
#define CHECK_EXPR(type, rtn) if (TypeCheckExpression(file, line, node->right, variables, functions) != type ||                               \
						          TypeCheckExpression(file, line, node->left, variables, functions) != type)                                  \
					              throw Error(file, line, "operator '" + node->data + "' must be used with two " + VarTypeToStr(type) + "s"); \
                              return rtn

// turns a value into an expression
std::shared_ptr<Expression> new_expression(
	const std::string& file,
	const int          line,

	const Value& value,
	const std::shared_ptr<Expression>& left,
	const std::shared_ptr<Expression>& right,

	std::vector<Variable>&    variables,
	std::vector<FunctionDef>& functions
);

// turns an array of tokens to an array of extras
std::vector<Value> TokensToValues(
	const std::vector<Token>& tokens,

	std::vector<Variable>&    variables,
	std::vector<FunctionDef>& functions
);

// checks for function definitions in a scope; if found, throw error
void CheckFunctionDefinition(
	const std::vector<Statement>& statements,

	const std::string& file,
	const int          line,
	const std::string& errorMsg
);

// type checks expression
VariableType TypeCheckExpression(
	const std::string& file,
	const int          line,

	const std::shared_ptr<Expression>& node,

	std::vector<Variable>&    variables,
	std::vector<FunctionDef>& functions
);

// returns the operator precedence
int GetOperatorPrecedence(
	const ValueType& type,
	const std::string& value
);

// returns values within a set of brackets
std::vector<Value> GetBracketExpression(
	const std::string& file,
	const int          line,

	const std::vector<Value>& values,
	std::size_t& index
);

// get next group of values
std::shared_ptr<Expression> GetNextGroup(
	const std::string& file,
	const int          line,

	const std::vector<Value>& values,
	std::size_t& index,

	std::vector<Variable>&    variables,
	std::vector<FunctionDef>& functions
);

// turns an array of values into an AST
std::shared_ptr<Expression> ParseValues(
	const std::string& file,
	const int          line,
	
	const std::vector<Value>& values,

	std::vector<Variable>&    variables,
	std::vector<FunctionDef>& functions
);

// parses an array of tokens into an array of statements
void Parser(
	std::vector<Statement>& statements,
	const std::vector<Token>& tokens,
	bool inFunction = false
);