#pragma once

#include "parser_scope.hpp"
#include "expression.hpp"
#include "bytecode.hpp"
#include "value_type.hpp"
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

	// this function must be called before generate_codes()
	virtual void check(ParserScope& scope) = 0;
	virtual bytecodes_t generate_codes() const = 0;

protected:
	Location loc;
};


class VariableInit : public AST
{
public:
	VariableInit(
		Location const& _loc,
		std::string const& _name,
		std::string const& _type,
		std::shared_ptr<expr::Expression> const& _expr);

	void check(ParserScope& scope) override;
	bytecodes_t generate_codes() const override;

private:
	std::string name;
	value_t type;
	std::shared_ptr<expr::Expression> expr;

	bytecode_t id;
};


class VariableAssign : public AST
{
public:
	VariableAssign(
		Location const& _loc,
		std::string const& _name,
		std::string const& assign_op,
		std::shared_ptr<expr::Expression> const& _expr);

	void check(ParserScope& scope) override;
	bytecodes_t generate_codes() const override;

private:
	std::string name;
	std::string assign_op;
	std::shared_ptr<expr::Expression> expr;

	std::optional<bytecode_t> id;
};


class Conditional : public AST
{
public:
	Conditional(
		Location const& _loc,
		std::vector<
			std::pair<std::shared_ptr<expr::Expression>, AST_Block>
		> const& _conditionals);

	void check(ParserScope& scope) override;
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
		expr::expr_p const& _cond,
		AST_Block const& _block);

	void check(ParserScope& scope) override;
	bytecodes_t generate_codes() const override;

private:
	expr::expr_p cond_expr;
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
		expr::expr_p const& _cond_expr,
		AST_Block const& _block);

	void check(ParserScope& scope) override;
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
		std::string const& _name,
		std::vector<std::string> const& _param_names,
		std::vector<std::string> const& _param_types,
		std::string const& _rtn_type,
		AST_Block const& _block);

	void check(ParserScope& scope) override;
	bytecodes_t generate_codes() const override;

private:
	std::string name;
	std::vector<std::string> param_names;
	std::vector<value_t> param_types;
	std::optional<value_t> rtn_type;
	AST_Block block;

	bytecode_t id;
	std::vector<bytecode_t> param_ids;
};


class Return : public AST
{
public:
	Return(
		Location const& _loc,
		expr::expr_p _expr);

	void check(ParserScope& scope) override;
	bytecodes_t generate_codes() const override;

private:
	expr::expr_p expr;
};


class FunctionCall : public AST, public expr::Expression
{
public:
	FunctionCall(
		Location const& _loc,
		std::string const& _name,
		std::vector<expr::expr_p> const& _arg_exprs);

	void insert_node(
		expr::expr_p const& node,
		expr::expr_p* prev = nullptr);

	std::optional<value_t> type_check(ParserScope const& scope) override;
	void check(ParserScope& scope) override;
	bytecodes_t generate_codes() const override;

	int precedence() const;

private:
	std::string name;
	std::vector<expr::expr_p> arg_exprs;

	bytecode_t id;
};