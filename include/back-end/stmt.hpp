#pragma once

#include "../back-end/parser.hpp"
#include "../back-end/interpreter.hpp"
#include "../error.hpp"

#include <variant>
#include <memory>
#include <string>
#include <vector>

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
	std::string func_name;
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
	enum T {
		PLUS, MINUS, TIMES, DIVIDE, MOD,
		GREATER, GREATER_EQ, SMALLER, SMALLER_EQ,
		AND, OR, EQUAL, NOT_EQUAL,
		DOT
	} type;

	std::string data;
	std::shared_ptr<ExprNode> left, right;

	void eval_num_types(
		Parser::ParserScope& scope
	) const;

	void throw_binary_op_err(
		Lexer& lexer,
		std::vector<Type> const& types,
		std::string const& side,
		std::string const& used_types
	) const;
};

struct ExprNode
{
	Location loc;

	enum {
		LITERAL, ARRAY, VARIABLE, CALL,
		UNARY_OP, BINARY_OP
	} type;

	std::variant<
		ValueCall,
		ValueVar,
		ValueArray,
		ValueLiteral,
		UnaryOPNode,
		BinaryOPNode
	> data;

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
	} const type;

	std::string var;
	std::shared_ptr<ExprNode> assign_expr;
};

struct StmtMethod
{
	std::shared_ptr<ExprNode> assign_expr;
};

struct Conditional
{
	std::shared_ptr<ExprNode> condition;
	std::vector<Stmt> body;
};

struct StmtIf
{
	std::vector<Conditional> chains;
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

struct StmtWhile
{
	std::shared_ptr<ExprNode> condition;
	std::vector<Stmt> body;
};

struct StmtFor
{
	std::string const it_name;
	std::shared_ptr<ExprNode> range;

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
	WHILE,
	FOR
};

struct Stmt
{
	Location const loc;
	StmtType const type;

	std::variant<
		StmtInit,
		StmtAssign,
		StmtMethod,
		StmtIf,
		StmtFn,
		StmtCall,
		StmtReturn,
		StmtWhile,
		StmtFor
	> data;
};



template <typename VarContainer>
struct Scope
{
	Scope* upper;
	std::unordered_map<std::string, VarContainer> vars;

	std::pair<std::string const, VarContainer>* get_var(
		std::string const& name
	);
};