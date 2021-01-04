#pragma once

#include <memory>
#include <optional>
#include <variant>
#include <string>
#include <vector>

enum class TokenType
{
	OPERATOR,

	OPEN_BRACKET, CLOSE_BRACKET,
	OPEN_SQUARE, CLOSE_SQUARE,
	OPEN_CURLY, CLOSE_CURLY,

	ASSIGNMENT,

	COLON, COMMA,

	BOOL_VAL, NUM_VAL, STR_VAL,

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
	std::string data;
};

enum class ValueType
{
	BOOL, NUM, STR,
	ARRAY,

	VARIABLE, CALL,

	OPERATOR,

	OPEN_BRACKET, CLOSE_BRACKET
};

// move this to parser if it isn't used anywhere else
struct Value
{
	ValueType type;

	std::string data;

	// array's are classified as one value, so their elements are stored
	// in 'extras'; ditto for function calls, as 'extras' stores their
	// arguments
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
	BOOL,
	NUM,
	STR,
	ARRAY,
	CLASS
};

struct Variable
{
	std::string name;
	std::shared_ptr<Expression> value;
};

struct Assignment
{
	char type;

	std::string name;
	std::shared_ptr<Expression> value;
};

struct Conditional
{
	std::shared_ptr<Expression> condition;
	std::vector<Statement> body;

	bool is_else() const;
};

struct IfStatement
{
	std::vector<Conditional> chains;
};

// used for FunctionDef and CheckFunction
struct ReturnValue
{
	// note about the name of the return type:
	//
	// if the return type of a function is an object, the class of which the
	// object belongs to needs to be noted in order to perform type checks
	//
	// this is done using the 'name' variable to store the classes name

	// make this std::optional?
	// see how it' sused first, then make it optional
	std::string name;
	VariableType type;
};

struct FunctionDef
{
	std::string name;
	std::vector<std::string> parameters;

	std::vector<Statement> body;
	std::vector<ReturnValue> return_types;
};

struct FunctionCall
{
	std::string name;
	std::vector<std::shared_ptr<Expression> > arguments;
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
	std::string iterator_name;
	std::shared_ptr<Expression> range;

	std::vector<Statement> body;
};

struct Element
{
	std::string name;
	std::shared_ptr<Expression> index;
	std::shared_ptr<Expression> assign;
};

struct MethodCall
{
	std::string name;
	std::shared_ptr<Expression> method_call;
};

enum class StatementType
{
	VARIABLE,
	ASSIGNMENT,
	IF_STATEMENT,
	FUNCTION_DEF,
	FUNCTION_CALL,
	RETURN,
	WHILE_LOOP,
	FOR_LOOP,
	ELEMENT,
	METHOD_CALL
};

struct Statement
{
	StatementType type;

	std::variant<
		Variable,
		Assignment,
		IfStatement,
		FunctionDef,
		FunctionCall,
		Return,
		WhileLoop,
		ForLoop,
		Element,
		MethodCall
	> stmt;
};