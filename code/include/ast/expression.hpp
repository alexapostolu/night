#pragma once

#include "parser_scope.hpp"
#include "bytecode.hpp"
#include "value_type.hpp"
#include "error.hpp"

#include <memory>
#include <variant>
#include <optional>
#include <string>

namespace expr
{

class Expression;
using expr_p = std::shared_ptr<Expression>;


enum class ExpressionType
{
	BRACKET,
	BINARY_OP,
	UNARY_OP,
	FUNCTION_CALL,
	ARRAY,
	VARIABLE,
	VALUE,
};

class Expression
{
public:
	Expression(
		ExpressionType _type,
		Location const& _loc);

	virtual void insert_node(
		expr_p const& node,
		expr_p* prev = nullptr) = 0;

	virtual std::optional<ValueType> type_check(ParserScope const& scope) = 0;
	virtual bytecodes_t generate_codes() const = 0;

public:
	virtual int precedence() const = 0;

public:
	bool is_operator() const;
	bool is_value() const;

public:
	bool guard;
	ExpressionType type;

protected:
	Location loc;

	static int subscript_prec;
	static int unary_op_prec;
	static int bin_op_prec;
	static int single_prec;
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
		std::string const& _type,
		expr::expr_p const& _expr = nullptr);

	UnaryOp(
		Location const& _loc,
		UnaryOpType _type,
		expr::expr_p const& _expr = nullptr);

	void insert_node(
		expr::expr_p const& node,
		expr::expr_p* prev = nullptr) override;

	std::optional<ValueType> type_check(ParserScope const& scope) override;
	bytecodes_t generate_codes() const;

	int precedence() const override;

private:
	UnaryOpType type;
	expr::expr_p expr;

	std::optional<ValueType> op_code;
};


enum class BinaryOpType
{
	ADD, SUB, MULT, DIV,
	LESSER, GREATER,
	LESSER_EQUALS, GREATER_EQUALS,
	EQUALS, NOT_EQUALS,
	AND, OR,
	SUBSCRIPT
};

/* This class manages all binary operators, including the subscript operator (index, array)
 * With the exception of the subscript operator, all primitive types are converted into 
 */
class BinaryOp : public Expression
{
public:
	BinaryOp(
		Location const& _loc,
		std::string const& _type,
		expr::expr_p const& _lhs = nullptr,
		expr::expr_p const& _rhs = nullptr);

	BinaryOp(
		Location const& _loc,
		BinaryOpType _type,
		expr::expr_p const& _lhs = nullptr,
		expr::expr_p const& _rhs = nullptr);

	void insert_node(
		expr::expr_p const& node,
		expr::expr_p* prev = nullptr) override;

	bytecodes_t generate_codes() const override;
	std::optional<ValueType> type_check(ParserScope const& scope) override;

public:
	int precedence() const override;

private:
	BinaryOpType type;
	expr::expr_p lhs, rhs;

	std::optional<BytecodeType> cast_lhs, cast_rhs;
	std::optional<ValueType> op_code;
};


class Array : public Expression
{
public:
	Array(
		Location const& _loc,
		std::vector<expr_p> const& _arr);

	void insert_node(
		expr::expr_p const& node,
		expr::expr_p* prev = nullptr) override;

	std::optional<ValueType> type_check(ParserScope const& scope) override;
	bytecodes_t generate_codes() const override;

	int precedence() const override;

private:
	std::vector<expr_p> arr;
};


class Variable : public Expression
{
public:
	Variable(
		Location const& _loc,
		std::string const& _name);

	void insert_node(
		expr::expr_p const& node,
		expr::expr_p* prev = nullptr) override;

	std::optional<ValueType> type_check(ParserScope const& scope) override;
	bytecodes_t generate_codes() const override;

	int precedence() const override;

private:
	std::string name;

	std::optional<bytecode_t> id;
};


// only stores single value literals
// this does not include arrays!
class Value : public Expression
{
public:
	Value(
		Location const& _loc,
		ValueType::PrimType _type,
		std::string const& _val);

	void insert_node(
		expr::expr_p const& node,
		expr::expr_p* prev = nullptr) override;

	std::optional<ValueType> type_check(ParserScope const& scope) override;
	bytecodes_t generate_codes() const override;

	int precedence() const override;

private:
	static bytecodes_t int_to_bytecodes(uint64_t uint64);

private:
	ValueType::PrimType type;
	std::string val;
};

}
