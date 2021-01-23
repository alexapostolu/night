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

	ASSIGN,

	COLON, COMMA,

	BOOL, NUM, STR,

	VAR,

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

	// arrays are classified as one value, so their elements are stored
	// in 'extras'; same for function calls, as 'extras' stores their
	// arguments
	std::vector<std::vector<Value> > extras;
};

struct Statement;

struct Expression
{
	const std::string file;
	const int line;

	ValueType type;

	std::string data;
	std::vector<std::shared_ptr<Expression> > extras;

	std::shared_ptr<Expression> left;
	std::shared_ptr<Expression> right;
};

struct VariableType
{
	enum Type {
		BOOL,
		NUM,
		STR,
		ARRAY,
		CLASS
	} type;

	// if it's an object, then this variable stores the name of the class
	std::string class_name;

	VariableType() {}
	VariableType(const Type& _type)
		: type(_type) {}

	bool operator==(const Type& _type) const;
	bool operator!=(const Type& _type) const;

	bool operator==(const VariableType& _type) const;
	bool operator!=(const VariableType& _type) const;
};

struct Variable
{
	std::string name;
	std::shared_ptr<Expression> value;
};

struct Assignment
{
	char assign_type;

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

struct FunctionDef
{
	std::string name;
	std::vector<std::string> parameters;

	std::vector<Statement> body;
	std::vector<VariableType> return_types;
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
	const std::string file;
	const int line;

	const StatementType type;

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