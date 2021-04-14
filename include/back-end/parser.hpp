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
	//
	// if no type can be determined for a variable in an expression, then it
	// is skipped
	VariableTypeContainer type_check_expr(
		const std::shared_ptr<Scope>& current_scope,

		const std::shared_ptr<Expression>& node,

		// for parameters, since they don't have types at first
		const VariableTypeContainer& required_types = {}
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

	// a note about 'is_reachable()':
	/*
	// 'is_reachable()' is used to determine whether a function parameter's
	// types should be modified by an expression
	//
	// the types of a parameter is determined by the expressions in which it
	// is used in
	//
	// however, expressions in a function's "local" scope are not counted as
	// those expressions may never be encountered
	//
		def func(x) {
			if (false)
				print(x / 3)

			return x
		}
	//
	// in the example, 'x' may never encounter the division operator, so
	// adding type 'int' to 'x' could cause some false errors
	//
		set b = true;
		func(b)        # error! 'b' must contain type: 'int'
	//
	// for this reason, only expressions in a function's "global" scope can
	// affect the types of a parameter
	//
		def func(x) {
			print(x && true) # 'x' contains type: 'bool'

			if (false)
				print(x / 3) # does not affect types of 'x'

			return x;
		}
	//
	// in a future update, variables' types will be specific to their scope,
	// making type checking way more effective
	*/
	bool is_reachable(const std::shared_ptr<Scope>& scope) const;

private:
	const Location loc;

	static const VariableTypeContainer all_types;

	static CheckFunctionContainer check_functions;
	static CheckClassContainer check_classes;
};