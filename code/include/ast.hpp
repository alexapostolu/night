#pragma once

#include "parser_scope.hpp"
#include "bytecode.hpp"
#include "expression.hpp"
#include "value.hpp"
#include "error.hpp"

#include <optional>
#include <memory>
#include <string>


class Expression;

class AST;
using AST_Block = std::vector<std::shared_ptr<AST>>;


class AST
{
public:
	AST(Location const& loc);

	virtual bytecodes_t generate_codes(ParserScope const& scope) const = 0;

protected:
	Location loc;
};


class VariableInit : public AST
{
public:
	VariableInit(Location const& _loc,
		std::string const& _name,
		std::shared_ptr<Expression> const& _expr);

	bytecodes_t generate_codes(ParserScope const& scope) const override;

private:
	std::string name;
	std::shared_ptr<Expression> expr;
};


class VariableAssign : public AST
{
public:
	VariableAssign(Location const& _loc,
		std::string const& _name,
		std::shared_ptr<Expression> const& _val,
		std::string const& assign_op);

	bytecodes_t generate_codes(ParserScope const& scope) const override;

private:
	std::string name;
	std::shared_ptr<Expression> expr;
	std::string assign_op;
};


class If : public AST
{
public:
	If(Location const& _loc, std::shared_ptr<Expression> const& _cond_expr, AST_Block const& _block);

	bytecodes_t generate_codes(ParserScope const& scope) const;

private:
	std::shared_ptr<Expression> cond_expr;
	AST_Block block;
};


class While : public AST
{
public:
	While(Location const& _loc, std::shared_ptr<Expression> const& _cond, AST_Block const& _block);

	bytecodes_t generate_codes(ParserScope const& scope) const;

private:
	std::shared_ptr<Expression>  cond_expr;
	AST_Block block;
};


class For : public AST
{
public:
	For(
		Location const& _loc,
		VariableInit const& _var_init,
		std::shared_ptr<Expression> const& _cond_expr,
		VariableAssign const& _var_assign,
		AST_Block const& _block);

	bytecodes_t generate_codes(ParserScope const& scope) const;

private:
	VariableInit var_init;
	std::shared_ptr<Expression> cond_expr;
	VariableAssign var_assign;
	AST_Block block;
};