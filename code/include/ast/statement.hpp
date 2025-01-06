#pragma once

#include "statement_scope.hpp"
#include "expression.hpp"
#include "bytecode.h"
#include "type.hpp"
#include "error.hpp"
#include "function.h"

#include <vector>
#include <tuple>
#include <memory>
#include <string>

class Statement;
using stmt_p = std::shared_ptr<Statement>;

using conditional_container = std::vector<
	std::pair<expr::expr_p, std::vector<stmt_p>>
>;


/* This class represents *all* valid statements in Night.
 * 
 * check() MUST be called before optimize() and generate_codes().
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
	virtual void check(StatementScope& global_scope) = 0;
	
	/* Return true to keep the statement. Return false to delete the statement.
	 * A statement should be deleted if it's redundant, for example unused
	 * variables or loops that never run.
	 * Sample Usage:
	 *   if (!stmt->optimize(global_scope))
	 *      remove stmt;
	 */
	virtual bool optimize(StatementScope& global_scope) = 0;
	
	virtual bytes_t generate_codes() const = 0;

protected:
	Location loc;
};


/* Initialization of non-arrays.
 * 
 * Use Cases:
 *    my_var int;
 *    my_var int = 3;
 */
class VariableInit : public Statement
{
public:
	VariableInit(
		Location const& _loc,
		std::string const& _name,
		std::string const& _type,
		expr::expr_p const& _expr
	);

	void check(StatementScope& scope) override;
	bool optimize(StatementScope& scope) override;
	bytes_t generate_codes() const override;

public:
	std::string name;

private:
	std::string type;
	Type var_type;
	expr::expr_p expr;

	std::optional<uint64_t> id;
	std::optional<Type> expr_type;
};


/* Initialization of arrays.
 * 
 * Use Cases:
 *    my_var int[2][3];
 *    my_var int[] = [ 1, 2, 3 ];
 */
class ArrayInitialization : public Statement
{
public:
	ArrayInitialization(
		Location const& loc,
		std::string const& _name,
		std::string const& _type,
		std::vector<expr::expr_p> const& _arr_sizes,
		expr::expr_p const& _expr
	);

	void check(StatementScope& scope) override;
	bool optimize(StatementScope& scope) override;
	bytes_t generate_codes() const override;

	// Precondition:
	//    'arr' is not null
	//    The reason for this is if `arr` is null, then there would be no way
	//    to modify the original array as this function does not return a value
	// 'type' is to set type_conversions for Array
	void fill_array(Type const& type, expr::expr_p expr, int depth) const;

public:
	std::string name;

private:
	std::string type;
	Type var_type;
	std::vector<expr::expr_p> arr_sizes;
	std::vector<int> arr_sizes_numerics;
	expr::expr_p expr;

	std::optional<uint64_t> id;
	std::optional<Type> expr_type;

	// true
	//    my_arr int[3] = [ 1, 2, 3 ];
	// false
	//    my_arr int[] = int[my_size];
	bool is_static;
};


class VariableAssign : public Statement
{
public:
	VariableAssign(
		Location const& _loc,
		std::string const& _var_name,
		std::vector<expr::expr_p> const& _subscripts,
		std::string const& _assign_op,
		expr::expr_p const& _expr
	);

	void check(StatementScope& scope) override;
	bool optimize(StatementScope& scope) override;
	bytes_t generate_codes() const override;

private:
	std::string var_name;
	std::vector<expr::expr_p> subscripts;
	std::string assign_op;
	expr::expr_p expr;

	std::optional<Type> assign_type;
	std::optional<uint64_t> id;
};


class Conditional : public Statement
{
public:
	Conditional(
		Location const& _loc,
		conditional_container const& _conditionals
	);

	void check(StatementScope& scope) override;
	bool optimize(StatementScope& scope) override;

	/**
	 * CONDITION		boolean expression for conditional
	 * INT8				8 bit integer value for JUMP_IF_FALSE
	 * JUMP_IF_FALSE	jumps to first line after JUMP
	 *   ...			conditional code
	 *   INT8			8 bit integer value for JUMP
	 *   JUMP			jumps to first line after conditional chain
	 */
	bytes_t generate_codes() const override;

private:
	// If a conditional epxression is nullptr, then it represents and else
	// statement and should be treated the same as a true if statement.
	conditional_container conditionals;
};


class While : public Statement
{
public:
	While(
		Location const& _loc,
		expr::expr_p const& _cond,
		std::vector<stmt_p> const& _block
	);

	void check(StatementScope& scope) override;
	bool optimize(StatementScope& scope) override;

	/** 
	 * CONDITION		boolean expression for while loop condition
	 * INT8				8 bit integer value for JUMP_IF_FALSE
	 * JUMP_IF_FALSE	jumps to first line after JUMP_N
	 *   ...			while loop code
	 *   INT8			8 bit integer value for JUMP_N
	 *   JUMP_N			jumps to first line of CONDITION
	 */
	bytes_t generate_codes() const override;

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
		std::vector<stmt_p> const& _block
	);

	void check(StatementScope& scope) override;
	bool optimize(StatementScope& scope) override;
	bytes_t generate_codes() const override;

private:
	VariableInit var_init;
	While loop;

	StatementScope local_scope;
};


class Function : public Statement
{
public:
	Function(
		Location const& _loc,
		std::string const& _name,
		std::vector<std::tuple<std::string, std::string, bool>> const& _parameters,
		std::string const& _rtn_type,
		int rtn_type_dim,
		std::vector<stmt_p> const& _block
	);

	void check(StatementScope& scope) override;
	bool optimize(StatementScope& scope) override;
	bytes_t generate_codes() const override;

private:
	std::string name;
	std::vector<std::string> param_names;
	std::vector<Type> param_types;
	std::optional<Type> rtn_type;
	std::vector<stmt_p> block;

	std::vector<uint64_t> param_ids;

	StatementFunction* function;
};


class Return : public Statement
{
public:
	Return(
		Location const& _loc,
		expr::expr_p const& _expr
	);

	void check(StatementScope& scope) override;
	bool optimize(StatementScope& scope) override;
	bytes_t generate_codes() const override;

private:
	expr::expr_p expr;
};


namespace expr {

class FunctionCall : public Statement, public expr::Expression
{
public:
	FunctionCall(
		Location const& _loc,
		std::string const& _name,
		std::vector<expr::expr_p> const& _arg_exprs,
		std::optional<uint64_t> const& _id
	);

	void insert_node(
		expr_p node,
		expr_p* prev = nullptr);

	void check(StatementScope& scope) override;
	std::optional<Type> type_check(StatementScope& scope) noexcept override;
	bool optimize(StatementScope& scope) override;
	[[nodiscard]]
	expr_p optimize(StatementScope const& scope) override;
	bytes_t generate_codes() const override;

private:
	std::string name;
	std::vector<expr::expr_p> arg_exprs;

	bool is_expr;

	std::optional<uint64_t> id;
};

}