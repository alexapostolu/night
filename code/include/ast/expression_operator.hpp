#pragma once

#include "expression.hpp"

namespace expr
{

enum class UnaryOpType
{
	NEGATIVE,
	NOT
};

struct UnaryOp : public Expression
{
public:
	UnaryOp(
		Location const& _loc,
		std::string const& _operator
	);

	UnaryOp(
		UnaryOp const& other
	);

	void insert_node(
		expr_p node,
		expr_p* prev = nullptr
	) override;

	std::optional<Type> type_check(
		StatementScope& scope
	) noexcept override;

	/*
	 * Optimizes the expression, and if it is Numeric, then evaluates the
	 * expression.
	 */
	[[nodiscard]]
	expr_p optimize(
		StatementScope const& scope
	) override;

	/*
	 * Generates bytes in the following order,
	 *   1) Expression
	 *   3) Operator
	 *
	 * expr and expr_type must be initialized before this function is called.
	 */
	bytecodes_t generate_codes() const override;

private:
	/*
	 * Returns the byte type of the operator.
	 *
	 * Used in generate_codes().
	 */
	bytecode_t generate_operator_byte() const;

	std::string operator_type_to_str() const;

private:
	// Given an operator as a string, provides the operator type.
	static std::unordered_map<std::string, UnaryOpType> const operators;

	// Initialized in constructor.
	UnaryOpType operator_type;

	// Initialized in insert_node().
	expr_p expr;

	// Initialized in type_check().
	// Used to determine type of operator bytecode in generate_codes().
	std::optional<Type> expr_type;
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
		expr_p* prev = nullptr
	) override;

	std::optional<Type> type_check(
		StatementScope& scope
	) noexcept override;

	/*
	 * Optimizes in the following order,
	 *   1) Optimizes left and right hand expressions
	 *   2) If both are strings and operator is ADD, then perform string
	 *        concatenation.
	 *   2) If both are Numeric, evaluate the binary expression
	 *   3) Otherwise, return self
	 *
	 * lhs and rhs must be initialized before this function is called.
	 */
	[[nodiscard]]
	expr_p optimize(
		StatementScope const& scope
	) override;

	/*
	 * Generates bytes in the following order,
	 *   1) Left hand side and its type cast if it has one
	 *   2) Right hand side and its type cast if it has one
	 *   3) Operator
	 *
	 * lhs, lhs_type, rhs and rhs_type must be initialized before this function is called.
	 */
	bytecodes_t generate_codes() const override;

private:
	/*
	 * String concatenation only works with the addition operator on two strings.
	 */
	std::optional<Type> type_check_string_concat();

	/*
	 * Arithmetic only works on two primitives.
	 */
	std::optional<Type> type_check_arithmetic();

	/*
	 * Modulus only works on two integers.
	 */
	std::optional<Type> type_check_mod() const;

	/*
	 * Comparisons only works on two non-arrays.
	 */
	std::optional<Type> type_check_comparision();

	/*
	 * Booleans only works on two primitives.
	 */
	std::optional<Type> type_check_boolean();

	/*
	 * Subscript only works with integer indices on an array or string.
	 */
	std::optional<Type> type_check_subscript() const;

	/*
	 * Returns a pair of string Arrays if operator is ADD and both left and right
	 * hand side expressions are strings. Otherwise return a pair of NULLs.
	 *
	 * Used in optimize().
	 */
	std::pair<std::shared_ptr<Array>, std::shared_ptr<Array>> is_string_concatenation() const;

	/*
	 * Returns the index and string if operator is SUBSCRIPT and left hand side
	 * expression is a numeric and right hand side expression is an array.
	 * Otherwise return a pair of NULLs.
	 *
	 * Used in optimize().
	 */
	std::pair<std::shared_ptr<Numeric>, std::shared_ptr<Array>> is_array_subscript() const;

	/*
	 * Returns the byte type of the operator.
	 *
	 * Used in generate_codes().
	 */
	bytecode_t generate_operator_byte() const;

	std::string operator_type_to_str() const;

private:
	// Given an operator as a string, provides the precedence and operator type.
	static std::unordered_map<std::string, std::tuple<int, BinaryOpType>> const operators;

	// Initialized in constructor.
	BinaryOpType operator_type;

	// Initialized in insert_node().
	expr_p lhs, rhs;

	// Initialized in type_check().
	// Used to determine type of operator bytecode in generate_codes().
	std::optional<Type> lhs_type, rhs_type;

	// Optionally initialized in type_check().
	std::optional<bytecode_t> cast_lhs, cast_rhs;
};

} // night::
