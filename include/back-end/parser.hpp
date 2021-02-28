#pragma once

#include "token.hpp"

#include <memory>
#include <vector>
#include <string>

class Parser
{
public:
	Parser(
		const std::shared_ptr<Scope>& scope,
		const std::vector<Token>& tokens
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

	// turns a value into an expression node
	std::shared_ptr<Expression> new_expression(
		const Value& value,
		const std::shared_ptr<Expression>& left,
		const std::shared_ptr<Expression>& right
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
	VariableTypeContainer TypeCheckExpression(
		const std::shared_ptr<Scope>& current_scope,

		const std::shared_ptr<Expression>& node,

		// for parameters, since they don't have types at first
		const std::string& op_name = {},
		const VariableTypeContainer& required_types = {},

		// in for loops, the types of the iterator is the types of all the
		// elements combined, so instead of returning ARRAY, this will return
		// all the types
		//
		// also for subscript operators
		bool* is_for_loop_range = nullptr
	);

	// searches current scope for variable, if not found, moves to upper scope;
	// once found, returns its address, or nullptr otherwise
	std::pair<const std::string, CheckVariable>* get_variable(
		const std::shared_ptr<Scope>& scope,
		const std::string& var_name
	);

	// start iterator at bracket position, advances to close bracket position
	void advance_to_close_bracket(
		const std::vector<Token>& units,
		std::vector<Token>::const_iterator& index
	);

private:
	const std::string file;
	const int line;

	static const VariableTypeContainer all_types;

	static std::unordered_map<std::string, CheckFunction> check_functions;
	static std::unordered_map<std::string, CheckClass> check_classes;
};