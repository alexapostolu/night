#pragma once

#include "parser/statement_scope.hpp"
#include "parser/ast/expression.hpp"
#include "common/bytecode.hpp"
#include "common/type.hpp"
#include "common/token.hpp"
#include "common/error.hpp"

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
	
	virtual bytecodes_t generate_codes() const = 0;
};


/*
 * Initialization of non-array variables.
 * 
 * When expression is null, a default value will be provided respectful to the
 * type.
 * 
 * Examples,
 *    my_var int;
 *    my_var int = 3;
 */
class VariableInit : public Statement
{
public:
	VariableInit(
		std::string const& _name,
		Location const& _name_loc,
		std::string const& _type,
		expr::expr_p const& _expr
	);

	void check(
		StatementScope& scope
	) override;

	bool optimize(
		StatementScope& scope
	) override;
	
	/*
	 * Bytes are generated in the following order,
	 *   1) Expression bytes
	 *   2) ID bytes
	 *   3) STORE
	 */
	bytecodes_t generate_codes() const override;

private:
	std::string name;
	Location name_loc;

	Type type;
	expr::expr_p expr;

	// Initialized in check().
	std::optional<night::id_t> id;

	// Initialized in check().
	std::optional<Type> expr_type;
};


/*
 * Initialization of arrays.
 * 
 * The dimensions of the array are purely for type checking, and do not impact
 * bytecode generation.
 * 
 * Examples,
 *    my_var int[2][3];
 *    my_var int[] = [ 1, 2, 3 ];
 */
class ArrayInitialization : public Statement
{
public:
	ArrayInitialization(
		std::string const& _name,
		Location const& _name_loc,
		std::string const& _type,
		std::vector<expr::expr_p> const& _arr_sizes,
		expr::expr_p const& _expr
	);

	void check(
		StatementScope& scope
	) override;

	bool optimize(
		StatementScope& scope
	) override;

	/*
	 * Bytes are generated in the following order,
	 *   1) Expression bytes
	 *   2) ID bytes
	 *   3) STORE
	 */
	bytecodes_t generate_codes() const override;

	// Precondition:
	//    'expr' is not null
	// 'type' is to set type_conversions for Array
	void fill_array(Type const& type, expr::expr_p expr, int depth) const;

private:
	std::string name;
	Location name_loc;
	
	Type type;
	std::vector<expr::expr_p> arr_sizes;
	std::vector<int> arr_sizes_numerics;
	expr::expr_p expr;

	// Initialized in check().
	std::optional<night::id_t> id;

	// Initialized in check().
	std::optional<Type> expr_type;

	// true
	//    my_arr int[3] = [ 1, 2, 3 ];
	// false
	//    my_arr int[] = int[my_size];
	bool is_static;
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
	bytecodes_t generate_codes() const override;

private:
	Location loc;

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
	bytecodes_t generate_codes() const override;

private:
	Location loc;

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
	bytecodes_t generate_codes() const override;

private:
	VariableInit var_init;
	While loop;

	StatementScope local_scope;
};


struct Parameter
{
	Parameter(
		std::string const& _name,
		Type const& _type,
		Location const& _location
	);

	std::string name;
	Type type;

	Location location;
};

class Function : public Statement
{
public:
	Function(
		std::string const& _name,
		Location const& _name_location,
		std::vector<Parameter> const& _parameters,
		std::string const& _rtn_type,
		int rtn_type_dim,
		std::vector<stmt_p> const& _body
	);

	void check(StatementScope& scope) override;
	bool optimize(StatementScope& scope) override;
	bytecodes_t generate_codes() const override;

private:
	std::string name;
	Location name_location;

	std::vector<Parameter> parameters;

	std::optional<Type> rtn_type;
	
	std::vector<stmt_p> body;

	// Initialized in check().
	std::optional<night::id_t> id;

	// Initialized in check().
	std::vector<night::id_t> parameter_ids;
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
	bytecodes_t generate_codes() const override;

private:
	Location loc;

	expr::expr_p expr;
};


namespace expr {

class ExpressionStatement : public Statement, public expr::Expression
{
public:
	ExpressionStatement(
		expr::expr_p const& _expr,
		Location const& loc
	);

	void insert_node(
		expr_p node,
		expr_p* prev = nullptr
	) override;

	void check(StatementScope& scope) override;
	std::optional<Type> type_check(StatementScope& scope) noexcept override;
	bool optimize(StatementScope& scope) override;
	[[nodiscard]]
	expr_p optimize(StatementScope const& scope) override;
	bytecodes_t generate_codes() const override;

private:
	expr::expr_p expr;
};

} // expr::
