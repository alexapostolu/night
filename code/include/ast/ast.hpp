#pragma once

#include "parser_scope.hpp"
#include "bytecode.hpp"
#include "expression.hpp"
#include "value.hpp"
#include "error.hpp"

#include <optional>
#include <memory>
#include <string>

namespace expr { class Expression; }

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
		std::shared_ptr<expr::Expression> const& _expr);

	bytecodes_t generate_codes(ParserScope const& scope) const override;

private:
	std::string name;
	std::shared_ptr<expr::Expression> expr;
};


class VariableAssign : public AST
{
public:
	VariableAssign(
		Location const& _loc,
		std::string const& _name,
		std::shared_ptr<expr::Expression> const& _val,
		std::string const& assign_op);

	bytecodes_t generate_codes(ParserScope const& scope) const override;

private:
	std::string name;
	std::shared_ptr<expr::Expression> expr;
	std::string assign_op;
};


class If : public AST
{
public:
	If(Location const& _loc, std::shared_ptr<expr::Expression> const& _cond_expr, AST_Block const& _block);

	bytecodes_t generate_codes(ParserScope const& scope) const;

private:
	std::shared_ptr<expr::Expression> cond_expr;
	AST_Block block;
};



class While : public AST
{
public:
	While(
		Location const& _loc,
		std::shared_ptr<expr::Expression> const& _cond,
		AST_Block const& _block);

	bytecodes_t generate_codes(ParserScope const& scope) const override;

private:
	std::shared_ptr<expr::Expression>  cond_expr;
	AST_Block block;
};


class For : public AST
{
public:
	For(
		Location const& _loc,
		VariableInit const& _var_init,
		std::shared_ptr<expr::Expression> const& _cond_expr,
		VariableAssign const& _var_assign,
		AST_Block const& _block);

	bytecodes_t generate_codes(ParserScope const& scope) const override;

private:
	VariableInit var_init;
	std::shared_ptr<expr::Expression> cond_expr;
	VariableAssign var_assign;
	AST_Block block;
};


class Function : public AST
{
public:
	Function(
		Location const& _loc,
		std::string const& _func_name,
		std::vector<std::string> const& _param_names,
		AST_Block const& _block);

	bytecodes_t generate_codes(ParserScope const& scope) const override;

private:
	std::string func_name;
	std::vector<std::string> param_names;
	AST_Block block;
};


class Return : public AST
{
public:
	Return(
		Location const& _loc,
		std::shared_ptr<expr::Expression> _expr);

	bytecodes_t generate_codes(ParserScope const& scope) const;

private:
	std::shared_ptr<expr::Expression> expr;
};