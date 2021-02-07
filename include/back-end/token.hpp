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

	BOOL, INT, FLOAT, STR,

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
	BOOL, INT, FLOAT, STR,
	ARRAY,

	VARIABLE, CALL,

	OPERATOR,

	OPEN_BRACKET, CLOSE_BRACKET
};

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

struct VariableType
{
	enum Type {
		BOOL,
		INT,
		FLOAT,
		STR,
		ARRAY,
		CLASS
	} type;

	// if it's an object, then this variable stores the name of the class
	std::string class_name;

	VariableType() {}
	VariableType(const Type& _type)
		: type(_type) {}
	VariableType(const Type& _type, const std::string& _name)
		: type(_type), class_name(_name) {}

	std::string to_str() const;

	bool operator==(const Type& _type) const;
	bool operator!=(const Type& _type) const;

	bool operator==(const VariableType& _type) const;
	bool operator!=(const VariableType& _type) const;
};

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

struct CheckVariable
{
	// a note about parameters:
	//
	// to perform type checking, parameters' types must be evaluated when the
	// function is defined
	//
	// they are stored in the same array as normal variables, so the only
	// difference is that they don't have a type: `types.empty()`
	//
	// their types are giving to them through the expressions they encounter,
	// for example 'param + 5' would mean 'param' is an 'int'
	//
	// if a parameter still doesn't have a type at the end of the function,
	// then it is given all the types
	//
	// once a parameter has types, it then behaves like a normal variable

	std::string name;
	std::vector<VariableType> types;

	// a note about 'is_array':
	//
	// if a variable is an array, it won't contain an array type, instead, it
	// will contain all the types of its elements
	//
	// this is useful in determining the types of for loop ranges
	//
	// and to signal that a variable is an array, the variable 'is_array' is
	// used

	bool is_array;
};

struct CheckFunction
{
	std::string name;
	std::vector<std::vector<VariableType> > parameters;

	std::vector<VariableType> return_types;

	bool is_void;

	//bool operator==(const std::string& _name) const;
};

struct CheckClass
{
	std::string name;

	std::vector<CheckVariable> variables;
	std::vector<CheckFunction> methods;
};

struct Scope
{
	std::shared_ptr<Scope> upper_scope;

	std::vector<Statement> statements;
	std::vector<CheckVariable> check_variables; 
};

struct Variable
{
	std::string name;
	std::shared_ptr<Expression> value;
};

struct Assignment
{
	enum {
		ASSIGN,
		PLUS,
		MINUS,
		TIMES,
		DIVIDE,
		MOD
	} assign_type;

	std::string variable_name;
	std::shared_ptr<Expression> assign_expr;
};

struct Conditional
{
	std::shared_ptr<Expression> condition;
	std::shared_ptr<Scope> body;
};

struct IfStatement
{
	std::vector<Conditional> chains;
};

struct FunctionDef
{
	std::string name;
	std::vector<std::string> parameters;

	std::shared_ptr<Scope> body;

	// a note about return_types:
	//
	// function return types have to be deduced when they are defined; this
	// can be done by seeing the return types of the functions, or, if it's a
	// recursive function, then the expressions in which the function is used
	// in can also be used to determine the return value of the function

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
	std::shared_ptr<Scope> body;
};

struct ForLoop
{
	std::string iterator_name;
	std::shared_ptr<Expression> range;

	std::shared_ptr<Scope> body;
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
	std::shared_ptr<Expression> assign_expr;
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