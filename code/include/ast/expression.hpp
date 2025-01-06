#pragma once

#include "statement_scope.hpp"
#include "type.hpp"
#include "util.hpp"
#include "error.hpp"

#include "bytecode.h"

#include <memory>
#include <variant>
#include <optional>
#include <string>

namespace expr
{

class Expression;
using expr_p = std::shared_ptr<Expression>;


/*
 * For each Expression, the following steps must be performed in order,
 *   1. Insert a node for expression parsing
 *   2. Type check its children and itself
 *   3. Optimize its children and itself
 *   4. Generate bytecodes
 */
class Expression
{
public:
	/*
	 * Parameters:
	 *   _loc Location of error messages used in optimize() and generate_codes()
	 */
	Expression(
		Location const& _loc,
		int _precedence_ = 0
	);

	/* Inserts a node into the AST. There are three cases:
	 *   1. Node needs to be inserted in the current position.
	 *      In this case, 'prev' will be assigned to the node, and then the node
	 *        will be assigned to the current position.
	 *   2. Node needs to be inserted as a child.
	 *      In this case, the 'node' will be appropriately inserted as a child node.
	 *   3. Node needs to be inserted further down the AST.
	 *      In this case, the 'node' will be passed down to the children using
	 *        this->child.insert(node).
	 *
	 * Parameters:
	 *   node  The new node to be inserted into the AST.
	 *   prev  The connection between the higher node and this. It is nullptr if
	 *             this is the head node.
	 */
	virtual void insert_node(
		expr_p node,
		expr_p* prev = nullptr
	) = 0;

	int precedence() const;
	void set_guard();

	/* Returns the type of the expression. If std::nullopt is returned, then
	 * type_check() has failed and at least one minor error has been created.
	 * No fatal errors should be thrown here.
	 */
	virtual std::optional<Type> type_check(
		StatementScope& scope
	) noexcept = 0;

	/* Evaluates constant expressions and returns the new expression. Leaves
	 * non-constant expressions unchanged.
	 * Usage:
	 *    expr = expr->optimize(scope);
	 */
	[[nodiscard]]
	virtual expr_p optimize(
		StatementScope const& scope
	) = 0;
	
	virtual bytes_t generate_codes() const = 0;

protected:
	constexpr static int single_precedence = 1000;
	constexpr static int unary_precedence = 100;
	constexpr static int binary_precedence = 10;

	Location loc;
	int precedence_;
};


enum class UnaryOpType
{
	NEGATIVE, NOT
};

struct UnaryOp : public Expression
{
public:
	UnaryOp(
		Location const& _loc,
		std::string const& _operator);

	UnaryOp(
		UnaryOp const& other);

	void insert_node(
		expr_p node,
		expr_p* prev = nullptr) override;

	std::optional<Type> type_check(StatementScope& scope) noexcept override;

	[[nodiscard]]
	expr_p optimize(StatementScope const& scope) override;
	
	bytes_t generate_codes() const;

private:
	UnaryOpType op_type;
	expr::expr_p expr;

	std::optional<byte_t> op_code;
};


enum class BinaryOpType
{
	ADD, SUB, MULT, DIV, MOD,
	LESSER, GREATER,
	LESSER_EQUALS, GREATER_EQUALS,
	EQUALS, NOT_EQUALS,
	AND, OR,
	SUBSCRIPT
};

class BinaryOp : public Expression
{
public:
	BinaryOp(
		Location const& _loc,
		std::string const& _operator
	);

	BinaryOp(
		BinaryOp const& other
	);

	void insert_node(
		expr_p node,
		expr_p* prev = nullptr) override;

	std::optional<Type> type_check(StatementScope& scope) noexcept override;
	expr_p optimize(StatementScope const& scope) override;
	bytes_t generate_codes() const override;

private:
	BinaryOpType op_type;
	expr::expr_p lhs, rhs;

	std::optional<byte_t> cast_lhs, cast_rhs;
	byte_t op_code;
};


class Variable : public Expression
{
public:
	Variable(
		Location const& _loc,
		std::string const& _name,
		std::optional<byte_t> const& _id = std::nullopt);

	void insert_node(
		expr_p node,
		expr_p* prev = nullptr) override;

	std::optional<Type> type_check(StatementScope& scope) noexcept override;

	[[nodiscard]]
	expr_p optimize(StatementScope const& scope) override;
	
	bytes_t generate_codes() const override;

private:
	std::string name;

	std::optional<uint64_t> id;
};


class Array : public Expression
{
public:
	Array(
		Location const& _loc,
		std::vector<expr_p> const& _elements,
		bool _is_str_,
		std::optional<Type> const& _type_convert = std::nullopt,
		std::vector<std::optional<byte_t>> const& _type_conversion = {});

	void insert_node(
		expr_p node,
		expr_p* prev = nullptr) override;

	std::optional<Type> type_check(StatementScope& scope) noexcept override;

	[[nodiscard]]
	expr_p optimize(StatementScope const& scope) override;

	bytes_t generate_codes() const override;

	bool is_str() const;

public:
	std::vector<expr_p> elements;

	// Note: Strings are stored as character arrays, but have special
	// properties such as being able to work with addition. This is why we have
	// a flag for it.
	bool is_str_;

	std::optional<Type> type_convert;
	std::vector<std::optional<byte_t>> type_conversion;
};


class Allocate : public Expression
{
public:
	Allocate(
		Location const& _loc,
		Type::Primitive const _type,
		std::vector<expr_p> const& _sizes
	);

	void insert_node(
		expr_p node,
		expr_p* prev = nullptr
	) override;

	std::optional<Type> type_check(
		StatementScope& scope
	) noexcept override;

	[[nodiscard]]
	expr_p optimize(
		StatementScope const& scope
	) override;

	bytes_t generate_codes() const override;

public:
	Type::Primitive type;
	std::vector<expr_p> sizes;
};


/* Note signed types are represented through a Unary Operator
 */
class Numeric : public Expression
{
public:
	Numeric(
		Location const& _loc,
		Type::Primitive _type,
		std::variant<int64_t, uint64_t, double> const& _val);

	void insert_node(
		expr_p node,
		expr_p* prev = nullptr) override;

	std::optional<Type> type_check(StatementScope& scope) noexcept override;

	[[nodiscard]]
	expr_p optimize(
		StatementScope const& scope) override;
	
	bytes_t generate_codes() const override;

	bool is_true() const;

public:
	std::variant<int64_t, uint64_t, double> val;

private:
	Type::Primitive type;
};

}
