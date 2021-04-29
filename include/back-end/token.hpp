#pragma once

#include "../error.hpp"

#include <memory>
#include <functional>
#include <variant>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

struct CheckVariable;
struct CheckFunction;
struct CheckClass;

using CheckVariableContainer = std::unordered_map<std::string, CheckVariable>;
using CheckFunctionContainer = std::unordered_map<std::string, CheckFunction>;
using CheckClassContainer    = std::unordered_map<std::string, CheckClass>;

struct ExprValue;
struct ExprNode;

using TypeContainer = std::vector<ExprValue>;
using ExprContainer = std::vector<std::shared_ptr<ExprNode> >;



enum class TokenType
{
	OPERATOR,

	OPEN_BRACKET, CLOSE_BRACKET,
	OPEN_SQUARE, CLOSE_SQUARE,
	OPEN_CURLY, CLOSE_CURLY,

	ASSIGN,

	COLON, COMMA, DOT,

	BOOL, INT, FLOAT, STRING,

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
	Location loc;

	TokenType type;
	std::string data;
};

struct Stmt;

enum class ValueType
{
	BOOL,
	INT, FLOAT,
	ARR, STR,
};

struct ExprNode;

struct ExprValue
{
	ValueType type;

	std::variant<
		bool,
		int, float,
		std::string
	> data;

	std::vector<std::shared_ptr<ExprNode> > elem_types;
	// std::string class_name;
};

namespace night {

bool contains(const TypeContainer& container, const ValueType& type)
{
	for (auto const& data : container)
	{
		if (data.type == type)
			return true;
	}

	return false;
}

template <typename... ValueTypes>
bool contains(const TypeContainer& container, const ValueType& val, const ValueTypes&... vals)
{
	for (auto const& data : container)
	{
		if (data.type == val)
			return contains(container, vals...);
	}

	return false;
}

const ExprValue* find(const TypeContainer& container, const ValueType& type)
{
	for (std::size_t i = 0; i < container.size(); ++i)
	{
		if (container[i].type == type)
			return &container[i];
	}

	return nullptr;
}

};

struct ExprVar
{
	std::string name;
};

struct ExprNode;

struct ExprCall
{
	std::string name;
	std::vector<std::vector<ExprNode> > params;
};

enum UnaryOPType
{
	NEGATIVE,
	NOT,
	SUBSCRIPT
};

struct ExprUnaryOP
{
	UnaryOPType const type;
	std::string const data;
	std::shared_ptr<ExprNode> value;
};

enum BinaryOPType
{
	PLUS, MINUS, TIMES, DIVIDE, MOD,
	GREATER, GREATER_EQ, SMALLER, SMALLER_EQ,
	AND, OR, EQUAL, NOT_EQUAL,
	DOT
};

struct ExprBinaryOP
{
	BinaryOPType const type;
	std::string const data;
	std::shared_ptr<ExprNode> left, right;
};

struct ExprNode
{
	const Location loc;

	enum {
		VALUE, VAR, CALL,
		UNARY_OP, BINARY_OP
	} type;

	std::variant<
		ExprValue,
		ExprVar,
		ExprCall,
		ExprUnaryOP,
		ExprBinaryOP
	> data;
};



struct Scope
{
	const std::shared_ptr<Scope> upper_scope;

	std::vector<Stmt> stmts;

	CheckVariableContainer vars;

	Scope(const std::shared_ptr<Scope>& _upper_scope);
};

struct StmtAssign
{
	enum Type {
		ASSIGN,
		PLUS,
		MINUS,
		TIMES,
		DIVIDE,
		MOD
	} const type;

	CheckVariable& check_var;
	std::vector<ExprNode> indicies;
	std::shared_ptr<ExprNode> assign_expr;
};

struct StmtMethod
{
	std::shared_ptr<ExprNode> assign_expr;
};

struct Conditional
{
	std::shared_ptr<ExprNode> condition;
	std::shared_ptr<Scope> body;
};

struct StmtIf
{
	std::vector<Conditional> chains;
};

struct StmtFuncCall
{
	//std::string const name;
	CheckFunction& call;
	std::vector<std::shared_ptr<ExprNode> > args;
};

struct StmtReturn
{
	std::shared_ptr<ExprNode> expr;
};

struct StmtWhile
{
	std::shared_ptr<ExprNode> condition;
	std::shared_ptr<Scope> body;
};

struct StmtFor
{
	std::string const it_name;
	std::shared_ptr<ExprNode> range;

	std::shared_ptr<Scope> body;
};

enum class StmtType
{
	ASSIGN,
	METHOD,
	IF,
	FUNC_CALL,
	RETURN,
	WHILE,
	FOR
};

struct Stmt
{
	Location const loc;

	StmtType const type;

	std::variant<
		StmtAssign,
		StmtMethod,
		StmtIf,
		StmtFuncCall,
		StmtReturn,
		StmtWhile,
		StmtFor
	> data;
};

struct CheckVariable
{
	// a note about parameters:
	/*
	// to perform type checking, parameters' types must be evaluated when the
	// function is defined
	//
	// they are stored in the same container as normal variables, so the only
	// difference is that they don't have a type
	//
	// they can be differentiated from normal variables using the method:
	// 'needs_types()'
	//
	// their types are giving to them through the expressions they encounter,
	// for example 'param || true' would mean 'param' is a boolean
	//
	// if a parameter still doesn't have a type at the end of the function,
	// then it is given all the types
	//
	// once a parameter has types, it then behaves like a normal variable
	*/
	TypeContainer types;

	NightData value;
};

struct CheckFunction
{
	std::vector<TypeContainer> params;

	std::shared_ptr<Scope> body;

	// a note about return types:
	/*
	// function return types have to be deduced when they are defined
	//
	// this is done by examining the return statement(s) of the function
	//
	// if no return types can be deduced, then the return type is treated as
	// whatever type is required for it to be
	*/
	TypeContainer rtn_types;

	bool is_void;
};

struct CheckClass
{
	CheckVariableContainer vars;
	CheckFunctionContainer methods;
};

std::pair<std::string const, CheckFunction> make_check_function(
	std::string const& name,

	std::vector<TypeContainer> const& params = {},
	TypeContainer			   const& rtn_types = {}
);

std::pair<std::string const, CheckClass> make_check_class(
	std::string const& name,

	CheckVariableContainer const& vars,
	CheckFunctionContainer const& methods
);