#pragma once

#include "token.hpp"

#include <memory>
#include <vector>
#include <string>

class Parser
{
public:
	Parser(
		Scope& scope,
		const std::vector<Token>& _tokens
	);

private:
	// turns an array of tokens into an array of values
	std::vector<Value> TokensToValues(
		const std::vector<Token>& tokens
	);

	// returns the operator precedence
	int GetOperatorPrecedence(
		const ValueType& type,
		const std::string& value
	);

	// gets next group of values
	// a "group" is a value including its unary operators
	//
	// index starts at unary operator and ends at next operator
	std::shared_ptr<Expression> GetNextGroup(
		const std::vector<Value>& values,
		std::size_t& index
	);

	// turns an array of values into an expression
	std::shared_ptr<Expression> ValuesToExpression(
		const std::vector<Value>& values
	);

	// type and usage checks an expression
	std::vector<VariableType> TypeCheckExpression(
		Scope& current_scope,

		const std::shared_ptr<Expression>& node,

		// for parameters, since they don't have types at first
		const std::string& op_name = {},
		const std::vector<VariableType>& required_types = {},

		// in for loops, the types of the iterator is the types of all the
		// elements combined, so instead of returning ARRAY, this will return
		// all the types
		bool is_for_loop_range = false
	);

	// turns a value into an expression node
	std::shared_ptr<Expression> new_expression(
		const Value& value,
		const std::shared_ptr<Expression>& left,
		const std::shared_ptr<Expression>& right
	);

	std::shared_ptr<Expression> ParseTokenExpression(
		Scope& current_scope,

		const std::size_t start,
		const std::size_t end,

		std::vector<VariableType>& types
	);

	std::shared_ptr<Expression> ExtractCondition(
		Scope& current_scope,

		std::size_t& close_bracket_index,
		const std::string& statement_type
	);

	Scope ExtractBody(
		Scope& upper_scope,

		const std::size_t close_bracket_index,
		const std::string& statement_type
	);

private:
	const std::string file;
	const int line;

	const std::vector<Token> tokens;

	static bool in_function;
	static std::vector<VariableType> return_types;

	static const std::vector<VariableType> all_types;

	static std::vector<CheckFunction> check_functions;
	static std::vector<CheckClass>    check_classes;
};