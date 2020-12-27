#pragma once

#include "token.hpp"
#include "utils.hpp"

#include <memory>
#include <vector>
#include <string>

// turns array of tokens to array of values
std::vector<Value> TokensToValues(
	const std::vector<Token>& tokens
);

// turns a value into an expression
std::shared_ptr<Expression> new_expression(
	const std::string& file,
	const int          line,

	const Value& value,
	const std::shared_ptr<Expression>& left,
	const std::shared_ptr<Expression>& right
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
	std::size_t& index
);

// turns an array of values into an expression
std::shared_ptr<Expression> ValuesToExpression(
	const std::string& file,
	const int          line,

	const std::vector<Value>& values
);

// type checks expression
std::vector<VariableType> TypeCheckExpression(
	const std::string& file,
	const int          line,

	const std::shared_ptr<Expression>& node,

	const std::vector<CheckVariable>& variables,
	const std::vector<CheckFunction>& functions,
	const std::vector<CheckClass>& classes,

	std::vector<CheckVariable>& parameters
);

// parses an array of tokens into an array of statements
void Parser(
	std::vector<Statement>& statements,
	const std::vector<Token>& tokens
);