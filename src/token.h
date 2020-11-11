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

	RANGE, ARROW,
	COLON, COMMA,

	ASSIGNMENT,

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

	bool operator!=(const Value& other)
	{
		return type == other.type && value == other.value && values == other.values;
	}
};

struct Expression;
struct Variable;
struct Conditional;
struct Statement;
struct Scope;

struct Scope
{
	//std::vector<Variable> variables;
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
	std::string name;
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
};