#pragma once

#include "token.hpp"

#include <memory>
#include <vector>
#include <string>

class Parser
{
public:
	Parser(
		std::vector<Statement>& statements,
		const std::vector<Token>& _tokens
	);

private:
	// turns array of tokens to array of values
	std::vector<Value> TokensToValues(
		const std::vector<Token>& tokens
	);

	// returns the operator precedence
	int GetOperatorPrecedence(
		const ValueType& type,
		const std::string& value
	);

	// get next group of values
	std::shared_ptr<Expression> GetNextGroup(
		const std::vector<Value>& values,
		std::size_t& index
	);

	// turns an array of values into an expression
	std::shared_ptr<Expression> ValuesToExpression(
		const std::vector<Value>& values
	);

	// type checks expression
	std::vector<VariableType> TypeCheckExpression(
		const std::shared_ptr<Expression>& node,

		// for parameters, since they likely don't have a types
		const std::string& op_name = {},
		const std::vector<VariableType>& required_types = {},

		bool* turn_into_array = nullptr
	);

	// turns a value into an expression
	std::shared_ptr<Expression> new_expression(
		const Value& value,
		const std::shared_ptr<Expression>& left,
		const std::shared_ptr<Expression>& right
	);

	std::shared_ptr<Expression> ParseTokenExpression(
		const std::size_t start,
		const std::size_t end,

		std::vector<VariableType>& types
	);

	std::shared_ptr<Expression> ExtractCondition(
		std::size_t& close_bracket_index,
		const std::string& statement_type
	);

	std::vector<Statement> ExtractBody(
		const std::size_t close_bracket_index,
		const std::string& statement_type
	);

private:
	// to perform type and usage checks, containers that are encountered must be
	// stored in specific structs called "Check" structs

	struct CheckVariable
	{
		// a note about parameters:
		//
		// to perform type checking, parameters' types must be evaluated when the
		// function is defined
		//
		// they are stored in the same array as normal variables, so the only
		// difference is that they don't have a type
		//
		// their types are giving to them through the expressions they encounter,
		// for example 'param + 5' would mean 'param' is an 'int'
		//
		// if a parameter still doesn't have a type at the end of the function,
		// then it is given all the types
		//
		// once a parameter has types, it behaves like a normal variable

		std::string name;
		std::vector<VariableType> types;

		bool is_param() const;
	};

	struct CheckFunction
	{
		std::string name;
		std::vector<std::vector<VariableType> > parameters;

		// a note about 'return_values':
		//
		// if std::optional doesn't contain a value, then the function is waiting
		// to be assigned return types (perhaps it's a recursive call)
		//
		// if std::optional does contain a value, but the vector is empty, then
		// it is a void function

		std::optional<std::vector<ReturnValue> > return_values;

		bool is_empty() const;
		bool is_void() const;

		std::vector<VariableType> return_types() const;
	};

	struct CheckClass
	{
		std::string name;

		std::vector<CheckVariable> variables;
		std::vector<CheckFunction> methods;
	};

private:
	std::string file;
	int line;

	std::vector<Token> tokens;

	static bool in_function;
	static std::vector<VariableType> return_types;

	static const std::vector<VariableType> all_types;

	static std::vector<CheckVariable> check_variables;
	static std::vector<CheckFunction> check_functions;
	static std::vector<CheckClass>    check_classes;
};