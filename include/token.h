#pragma once

#include <memory>
#include <variant>
#include <string>
#include <vector>

enum class TokenType
{
	OPERATOR,

	OPEN_BRACKET, CLOSE_BRACKET,
	OPEN_SQUARE, CLOSE_SQUARE,
	OPEN_CURLY, CLOSE_CURLY,

	COMMA,

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

struct Statement;

struct Expression
{
	ValueType type;

	std::string data;
	std::vector<std::shared_ptr<Expression> > extras;

	std::shared_ptr<Expression> left;
	std::shared_ptr<Expression> right;
};

enum class VariableType
{
	BOOL, BOOL_ARR,
	NUM, NUM_ARR,
	STRING, STRING_ARR,
	COORD
};

struct Variable
{
	VariableType type;
	std::string name;
	std::shared_ptr<Expression> value;
};

struct Assignment
{
	std::string name;
	std::shared_ptr<Expression> value;
};

struct Conditional
{
	std::shared_ptr<Expression> condition;
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
	std::vector<std::shared_ptr<Expression> > parameters;
};

struct Return
{
	std::shared_ptr<Expression> expression;
};

struct WhileLoop
{
	std::shared_ptr<Expression> condition;
	std::vector<Statement> body;
};

struct ForLoop
{
	std::string iterator;
	std::shared_ptr<Expression> range;

	std::vector<Statement> body;
};

struct Element
{
	std::string name;
	std::shared_ptr<Expression> index;
	std::shared_ptr<Expression> assign;
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