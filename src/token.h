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

	VARIABLE, CALL,

	OPERATOR,

	OPEN_BRACKET, CLOSE_BRACKET
};

struct Value
{
	ValueType type;
	std::string data;

	std::vector<std::vector<Value> > extras;
};

struct Expression;
struct Variable;
struct Conditional;
struct Statement;

struct Expression
{
	ValueType type;

	std::string data;
	std::vector<Expression> extras;

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

struct Conditional
{
	Expression* condition;
	std::vector<Statement> body;

	std::vector<Conditional> chains;
};

struct FunctionDef
{
	std::string name;
	std::vector<std::string> parameters;
	std::vector<Statement> body;
};

struct FunctionCall
{
	std::string name;
	std::vector<Expression*> parameters;
};

struct Return
{
	Expression* expression;
};

struct WhileLoop
{
	Expression* condition;
	std::vector<Statement> body;
};

struct ForLoop
{
	std::string index;
	Expression* range;

	std::vector<Statement> body;
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
	RETURN,
	WHILE_LOOP,
	FOR_LOOP,
	ELEMENT
};

struct Statement
{
    StatementType type;

	std::variant<
		Variable,
		Assignment,
		Conditional,
		FunctionDef,
		FunctionCall,
		Return,
		WhileLoop,
		ForLoop,
		Element
	> stmt;
};