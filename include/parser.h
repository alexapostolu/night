#pragma once

#include "token.h"
#include "utils.h"

#include <vector>
#include <string>

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

	const std::vector<CheckVariable>& variables,
	const std::vector<CheckFunction>& functions,

	int* arrayDepth = nullptr
);

// type checks expression
VariableType TypeCheckExpression(
	const std::string& file,
	const int          line,

	const std::shared_ptr<Expression>& node,

	const std::vector<CheckVariable>& variables,
	const std::vector<CheckFunction>& functions,

	const std::vector<CheckVariable>& parameters
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

	const std::vector<CheckVariable>& variables,
	const std::vector<CheckFunction>& functions
);

// turns an array of values into an AST
std::shared_ptr<Expression> ParseValues(
	const std::string& file,
	const int          line,
	
	const std::vector<Value>& values,

	const std::vector<CheckVariable>&    variables,
	const std::vector<CheckFunction>& functions
);

// parses an array of tokens into an array of statements
void Parser(
	std::vector<Statement>& statements,
	const std::vector<Token>& tokens,

	bool inFunction = false
);