#pragma once

#include "parser_scope.hpp"
#include "expression.hpp"
#include "bytecode.hpp"
#include "value_type.hpp"
#include "error.hpp"

#include <vector>
#include <memory>
#include <string>

class Statement;
using stmt_p = std::shared_ptr<Statement>;


/* check() MUST be called before optimize() and generate_codes().
 * 
 * Usage:
 *   statement->check(scope);
 * 
 *   if (night::error::get().has_minor_errors())
 *       throw night::error::get();
 * 
 *   if (statement->optimize(scope))
 *	     auto codes = statement->generate_codes();
 */
class Statement
{
public:
	Statement(Location const& loc);

	/* This method MUST be called before optimize() and generate_codes().
	 * 
	 * The purpose of this method is to ensure the statement is correct for
	 * optimizing and generating bytecodes. After this method is called, and if
	 * there are no minor errors, then it is guaranteed for optimize() and
	 * generate_codes() to work.
	 * 
	 * This method also initializes certain std::optional member variables,
	 * such as variable id for the VariableInit statement.
	 */
	virtual void check(ParserScope& global_scope) = 0;
	
	/* Return true to keep the statement. Return false to delete the statement.
	 * A statement should be deleted if it's redundant, for example unused
	 * variables or loops that never run.
	 * Sample Usage:
	 *   if (!stmt->optimize(global_scope))
	 *      remove stmt;
	 */
	virtual bool optimize(ParserScope& global_scope) = 0;
	
	virtual bytecodes_t generate_codes() const = 0;

protected:
	Location loc;
};


class VariableInit : public Statement
{
public:
	VariableInit(
		Location const& _loc,
		std::string const& _name,
		std::string const& _type,
		std::vector<expr::expr_p> const& _arr_sizes,
		expr::expr_p const& expr);

	void check(ParserScope& scope) override;
	bool optimize(ParserScope& scope) override;
	bytecodes_t generate_codes() const override;

private:
	bytecodes_t populate_array(expr::expr_p arr, int level) const;

public:
	std::string name;

private:
	std::string type;
	ValueType var_type;
	std::vector<expr::expr_p> arr_sizes;
	expr::expr_p expr;

	std::optional<bytecode_t> id;
	std::optional<ValueType> expr_type;
};


class VariableAssign : public Statement
{
public:
	VariableAssign(
		Location const& _loc,
		std::string const& _var_name,
		std::string const& _assign_op,
		expr::expr_p const& _expr);

	void check(ParserScope& scope) override;
	bool optimize(ParserScope& scope) override;
	bytecodes_t generate_codes() const override;

private:
	std::string var_name;
	std::string assign_op;
	expr::expr_p expr;

	std::optional<ValueType> assign_type;
	std::optional<bytecode_t> id;
};


class Conditional : public Statement
{
public:
	Conditional(
		Location const& _loc,
		std::vector<
			std::pair<expr::expr_p, std::vector<stmt_p>>
		> const& _conditionals);

	void check(ParserScope& scope) override;
	bool optimize(ParserScope& scope) override;
	bytecodes_t generate_codes() const override;

private:
	std::vector<
		std::pair<expr::expr_p, std::vector<stmt_p>>
	> conditionals;
};


class While : public Statement
{
public:
	While(
		Location const& _loc,
		expr::expr_p const& _cond,
		std::vector<stmt_p> const& _block);

	void check(ParserScope& scope) override;
	bool optimize(ParserScope& scope) override;
	bytecodes_t generate_codes() const override;

private:
	expr::expr_p cond_expr;
	std::vector<stmt_p> block;
};


class For : public Statement
{
public:
	// params:
	//   _block should already include VariableAssign statement
	For(
		Location const& _loc,
		VariableInit const& _var_init,
		expr::expr_p const& _cond_expr,
		std::vector<stmt_p> const& _block);

	void check(ParserScope& scope) override;
	bool optimize(ParserScope& scope) override;
	bytecodes_t generate_codes() const override;

private:
	VariableInit var_init;
	While loop;
};


class Function : public Statement
{
public:
	Function(
		Location const& _loc,
		std::string const& _name,
		std::vector<std::pair<std::string, std::string>> const& _parameters,
		std::string const& _rtn_type,
		std::vector<stmt_p> const& _block);

	void check(ParserScope& scope) override;
	bool optimize(ParserScope& scope) override;
	bytecodes_t generate_codes() const override;

private:
	std::string name;
	std::vector<std::string> param_names;
	std::vector<ValueType> param_types;
	std::optional<ValueType> rtn_type;
	std::vector<stmt_p> block;

	bytecode_t id;
	std::vector<bytecode_t> param_ids;
};


class Return : public Statement
{
public:
	Return(
		Location const& _loc,
		expr::expr_p const& _expr);

	void check(ParserScope& scope) override;
	bool optimize(ParserScope& scope) override;
	bytecodes_t generate_codes() const override;

private:
	expr::expr_p expr;
};


class ArrayMethod : public Statement
{
public:
	ArrayMethod(
		Location const& _loc,
		std::string const& _var_name,
		std::string const& _assign_op,
		std::vector<expr::expr_p> const& _subscripts,
		expr::expr_p const& _assign_expr);

	void check(ParserScope& scope) override;
	bool optimize(ParserScope& scope) override;
	bytecodes_t generate_codes() const override;

private:
	std::string var_name;
	std::string assign_op;
	std::vector<expr::expr_p> subscripts;
	expr::expr_p assign_expr;

	std::optional<bytecode_t> id;
	std::optional<ValueType> assign_type;
};


namespace expr {

class FunctionCall : public Statement, public expr::Expression
{
public:
	FunctionCall(
		Location const& _loc,
		std::string const& _name,
		std::vector<expr::expr_p> const& _arg_exprs,
		std::optional<bytecode_t> const& _id = std::nullopt);

	void insert_node(
		expr_p node,
		expr_p* prev = nullptr);

	void check(ParserScope& scope) override;
	std::optional<ValueType> type_check(ParserScope const& scope) noexcept override;
	bool optimize(ParserScope& scope) override;
	[[nodiscard]]
	expr_p optimize(ParserScope const& scope) override;
	bytecodes_t generate_codes() const override;

	int precedence() const;

private:
	std::string name;
	std::vector<expr::expr_p> arg_exprs;

	std::optional<bytecode_t> id;

	bool is_expr;
};

}