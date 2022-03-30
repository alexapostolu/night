#pragma once

#include "../error.hpp"

#include <iostream>
#include <variant>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

struct ExprNode;

using ExprContainer = std::vector<std::shared_ptr<ExprNode> >;



struct ValueLiteral
{
	enum T {
		BOOL,
		INT, FLOAT,
		STR,
	} type;

	std::variant<
		bool,
		int, float,
		std::string
	> data;
};

struct ValueArray
{
	ExprContainer elem_exprs;
};

struct ValueVar
{
	std::string name;
};

struct ValueCall
{
	std::string name;
	ExprContainer param_exprs;
};

struct UnaryOPNode
{
	enum T {
		NEGATIVE,
		NOT,
		SUBSCRIPT
	} type;

	std::string data;
	std::shared_ptr<ExprNode> value;
	std::shared_ptr<ExprNode> index;
};

struct BinaryOPNode
{
	Location loc;

	enum T {
		PLUS, MINUS, TIMES, DIVIDE, MOD,
		GREATER, GREATER_EQ, SMALLER, SMALLER_EQ,
		AND, OR, EQUAL, NOT_EQUAL,
		DOT, RANGE
	} type;

	std::string data;
	std::shared_ptr<ExprNode> left, right;
};

struct ExprNode
{
	Location loc;

	enum ExNTypes {
		LITERAL, ARRAY, VARIABLE, CALL,
		UNARY_OP, BINARY_OP
	} type;

	typedef std::variant<
		ValueCall,
		ValueVar,
		ValueArray,
		ValueLiteral,
		UnaryOPNode,
		BinaryOPNode
	> ExNDataTypes;

	ExNDataTypes data;

	ExprNode() = default;

	ExprNode(Location _loc, ExNTypes _type, ExNDataTypes _data)
		: loc(_loc), type(_type), data(_data) {}

	// returns right or value of expr_node
	std::shared_ptr<ExprNode>& travel_ast();

	bool is_value() const;
};



struct Stmt;

struct StmtInit
{
	std::string				  const name;
	std::shared_ptr<ExprNode> const expr;
};

struct StmtAssign
{
	enum T {
		ASSIGN,
		PLUS,
		MINUS,
		TIMES,
		DIVIDE,
		MOD
	} type;

	std::string var_name;
	std::vector<std::shared_ptr<ExprNode> > subscript_chain;
	std::shared_ptr<ExprNode> assign_expr;
};

struct StmtMethod
{
	std::shared_ptr<ExprNode> assign_expr;
};

struct StmtIfConditional
{
	std::shared_ptr<ExprNode> condition;
	std::vector<Stmt> body;
};

struct StmtIf
{
	std::vector<StmtIfConditional> chains;
};

struct StmtFn
{
	std::string				 const name;
	std::vector<std::string> const params;
	std::vector<Stmt>		 const body;
};

struct StmtCall
{
	std::string								const name;
	std::vector<std::shared_ptr<ExprNode> > const args;
};

struct StmtReturn
{
	std::shared_ptr<ExprNode> expr;
};

enum class StmtLoopSectionType
{
	INIT, RANGE, CONDITIONAL
};

struct StmtLoopSection
{
	StmtLoopSectionType type;

	std::string it_name;
	std::shared_ptr<ExprNode> expr;
};

struct StmtLoop
{
	std::vector<StmtLoopSection> sections;
	std::vector<Stmt> body;
};

enum class StmtType
{
	INIT,
	ASSIGN,
	METHOD,
	IF,
	FN,
	CALL,
	RETURN,
	LOOP
};

struct Stmt
{
	Location loc;
	StmtType type;

	std::variant<
		StmtInit,
		StmtAssign,
		StmtMethod,
		StmtIf,
		StmtFn,
		StmtCall,
		StmtReturn,
		StmtLoop
	> data;
};



template <typename VarContainer>
struct Scope
{
	Scope* upper;
	std::unordered_map<std::string, VarContainer> vars;

	std::pair<std::string const, VarContainer>* get_var(std::string const& name)
	{
		for (Scope<VarContainer>* curr_scope = this; curr_scope != nullptr;)
		{
			for (auto& var : curr_scope->vars)
			{
				if (var.first == name)
					return &var;
			}

			curr_scope = curr_scope->upper;
		}

		return nullptr;
	}
};