#pragma once

#include <string>
#include <vector>

enum class TokenType
{
	OPERATOR,

	OPEN_BRACKET, CLOSE_BRACKET,
	OPEN_SQUARE, CLOSE_SQUARE,
	OPEN_CURLY, CLOSE_CURLY,

	ASSIGNMENT,

	COMMA,

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
	std::vector<std::string> values;
};

struct Expression;
struct Variable;
struct Conditional;
struct Statement;
struct Scope;

struct Scope
{
	std::vector<Variable> variables;
	std::vector<Statement*> statements;
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

enum class ConditionalType
{
	IF,
	ELSE_IF,
	ELSE
};

struct Conditional
{
	//ConditionalType type;

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
	Variable variable;
	Expression* condition;
	Assignment increment;

	Scope body;
};

enum class StatementType
{
	VARIABLE,
	ASSIGNMENT,
	CONDITIONAL,
	FUNCTION_DEF,
	FUNCTION_CALL,
	WHILE_LOOP,
	FOR_LOOP
};

struct Statement
{
    StatementType type;

	union as {
		as() {};
		~as() {};

		Variable variable;
		Assignment assignment;
		Conditional conditional;
		FunctionCall functionCall;
		FunctionDef functionDef;
		WhileLoop whileLoop;
		ForLoop forLoop;
	} as;
};

#include "error.h"
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