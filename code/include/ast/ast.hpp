#pragma once

#include "expression.hpp"
#include "bytecode.hpp"
#include "value.hpp"
#include "error.hpp"

#include <memory>
#include <vector>
#include <string>

namespace expr { class Expression; }

class AST;
using AST_Block = std::vector<std::shared_ptr<AST>>;


class AST
{
public:
	AST(Location const& loc);

	virtual bytecodes_t generate_codes() const = 0;

protected:
	Location loc;
};


class VariableInit : public AST
{
public:
	VariableInit(
		Location const& _loc,
		bytecode_t _id,
		std::shared_ptr<expr::Expression> const& _expr);

	bytecodes_t generate_codes() const override;

private:
	bytecode_t id;
	std::shared_ptr<expr::Expression> expr;
};


class VariableAssign : public AST
{
public:
	VariableAssign(
		Location const& _loc,
		bytecode_t _id,
		std::shared_ptr<expr::Expression> const& _val,
		std::string const& assign_op);

	bytecodes_t generate_codes() const override;

private:
	bytecode_t id;
	std::shared_ptr<expr::Expression> expr;
	std::string assign_op;
};


class Conditional : public AST
{
public:
	Conditional(
		Location const& _loc,
		std::vector<
			std::pair<std::shared_ptr<expr::Expression>, AST_Block>
		> const& _conditionals);

	bytecodes_t generate_codes() const override;

private:
	std::vector<
		std::pair<std::shared_ptr<expr::Expression>, AST_Block>
	> conditionals;
};


class While : public AST
{
public:
	While(
		Location const& _loc,
		std::shared_ptr<expr::Expression> const& _cond,
		AST_Block const& _block);

	bytecodes_t generate_codes() const override;

private:
	std::shared_ptr<expr::Expression> cond_expr;
	AST_Block block;
};


class For : public AST
{
public:
	// params:
	//   _block should already include VariableAssign statement
	For(
		Location const& _loc,
		VariableInit const& _var_init,
		std::shared_ptr<expr::Expression> const& _cond_expr,
		AST_Block const& _block);

	bytecodes_t generate_codes() const override;

private:
	VariableInit var_init;
	While loop;
};


class Function : public AST
{
public:
	Function(
		Location const& _loc,
		std::string const& _func_name,
		std::vector<bytecode_t> const& _param_ids,
		AST_Block const& _block);

	bytecodes_t generate_codes() const override;

private:
	std::string func_name;
	std::vector<bytecode_t> param_ids;
	AST_Block block;
};


class Return : public AST
{
public:
	Return(
		Location const& _loc,
		std::shared_ptr<expr::Expression> _expr);

	bytecodes_t generate_codes() const;

private:
	std::shared_ptr<expr::Expression> expr;
};


class FunctionCall : public AST
{
public:
	FunctionCall(
		Location const& _loc,
		std::string const& _func_name,
		std::vector<std::shared_ptr<expr::Expression>> const& _params,
		std::vector<bytecode_t> const& param_ids);

	bytecodes_t generate_codes() const;

private:
	std::string func_name;
	std::vector<VariableInit> params;
};