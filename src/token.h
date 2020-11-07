#pragma once

#include <string>
#include <vector>
#include <variant>

enum class TokenType
{
	OPERATOR,

	OPEN_BRACKET, CLOSE_BRACKET,
	OPEN_SQUARE, CLOSE_SQUARE,
	OPEN_CURLY, CLOSE_CURLY,

	ASSIGNMENT,

	COLON, COMMA,
	RANGE, ARROW,

	BOOL_VAL, NUM_VAL, STRING_VAL,

	VARIABLE,

	SET,

	IF, ELSE,

	WHILE, FOR,

	DEF,
	RETURN,

	IMPORT,

	EOL
};

struct Token
{
	std::string file;
	int line;

	TokenType type;
	std::string value;
};

enum class ValueType
{
	BOOL, BOOL_ARR,
	NUM, NUM_ARR,
	STRING, STRING_ARR,

	VARIABLE, ARRAY, ELEMENT, CALL,

	OPERATOR,

	OPEN_BRACKET, CLOSE_BRACKET
};

struct Value
{
	ValueType type;
	std::string value;
	std::vector<Value> values;
};

struct Expression;
struct Variable;
struct Conditional;
struct Statement;
struct Scope;

struct Scope
{
	std::vector<Variable> variables;
	std::vector<Statement> statements;
};

struct Expression
{
	Value value;
	Expression* left;
	Expression* right;
};

enum class VariableType
{
	BOOL, BOOL_ARR,
	NUM, NUM_ARR,
	STRING, STRING_ARR
};

struct Variable
{
	std::string name;
	Expression* value;
};

struct Assignment
{
	std::string name;
	Expression* value;
};

struct FunctionCall
{
	std::string name;
	std::vector<Expression*> parameters;
};

struct FunctionDef
{
	std::string name;
	std::vector<std::string> parameters;
	Scope body;
};

struct Conditional
{
	Expression* condition;
	Scope body;

	std::vector<Conditional> chains;
};

struct WhileLoop
{
	Expression* condition;
	Scope body;
};

struct ForLoop
{
	std::string index;
	Expression* range;

	Scope body;
};

struct Element
{
	std::string array;
	Expression* index;
	Expression* assign;
};

enum class StatementType
{
	VARIABLE,
	ASSIGNMENT,
	CONDITIONAL,
	FUNCTION_DEF,
	FUNCTION_CALL,
	WHILE_LOOP,
	FOR_LOOP,
	ELEMENT
};

#include "error.h"
struct Statement
{
    StatementType type;

	std::variant<
		Variable,
		Assignment,
		Conditional,
		FunctionCall,
		FunctionDef,
		WhileLoop,
		ForLoop,
		Element
	> stmt;

	/*

	union {
		Variable variable;
		Assignment assignment;
		Conditional conditional;
		FunctionCall functionCall;
		FunctionDef functionDef;
		WhileLoop whileLoop;
		ForLoop forLoop;
	};

	Statement(const StatementType& _type)
		: type(_type) {}

	Statement(const Statement& _statement)
		: type (_statement.type)
	{
		switch (_statement.type)
		{
		case StatementType::VARIABLE:
			variable = _statement.variable;
			break;
		case StatementType::ASSIGNMENT:
			as.assignment = _statement.assignment;
			break;
		case StatementType::CONDITIONAL:
			as.conditional = _statement.as.conditional;
			break;
		case StatementType::FUNCTION_DEF:
			as.functionDef = _statement.as.functionDef;
			break;
		case StatementType::FUNCTION_CALL:
			as.functionCall = _statement.as.functionCall;
			break;
		case StatementType::WHILE_LOOP:
			as.whileLoop = _statement.as.whileLoop;
			break;
		case StatementType::FOR_LOOP:
			forLoop = _statement.forLoop;
			break;
		default:
			assert(true, "you forgot a type");
		}
	}

	Statement& operator=(const Statement& _statement)
	{
		type = _statement.type;
		switch (_statement.type)
		{
		case StatementType::VARIABLE:
			variable = _statement.variable;
			break;
		case StatementType::ASSIGNMENT:
			assignment = _statement.assignment;
			break;
		case StatementType::CONDITIONAL:
			conditional = _statement.conditional;
			break;
		case StatementType::FUNCTION_DEF:
			functionDef = _statement.functionDef;
			break;
		case StatementType::FUNCTION_CALL:
			functionCall = _statement.functionCall;
			break;
		case StatementType::WHILE_LOOP:
			whileLoop = _statement.whileLoop;
			break;
		case StatementType::FOR_LOOP:
			forLoop = _statement.forLoop;
			break;
		default:
			assert(true, "you forgot a type");
		}

		return *this;
	}

	~Statement()
	{
		switch (type)
		{
		case StatementType::VARIABLE:
			variable.~Variable();
			break;
		case StatementType::ASSIGNMENT:
			assignment.~Assignment();
			break;
		case StatementType::CONDITIONAL:
			conditional.~Conditional();
			break;
		case StatementType::FUNCTION_CALL:
			functionCall.~FunctionCall();
			break;
		case StatementType::FUNCTION_DEF:
			functionDef.~FunctionDef();
			break;
		case StatementType::WHILE_LOOP:
			whileLoop.~WhileLoop();
			break;
		case StatementType::FOR_LOOP:
			forLoop.~ForLoop();
			break;
		default:
			assert(true, "you forgot a type");
		}
	}

	*/
};

// variable type to string
std::string VarTypeToStr(const VariableType& type)
{
	switch (type)
	{
	case VariableType::BOOL:
		return "boolean";
	case VariableType::BOOL_ARR:
		return "boolean array";
	case VariableType::NUM:
		return "number";
	case VariableType::NUM_ARR:
		return "number array";
	case VariableType::STRING:
		return "string";
	case VariableType::STRING_ARR:
		return "string array";
	default:
		assert(true, "you forgot a type");
	}
}