#pragma once

#include "parser_scope.hpp"
#include "bytecode.hpp"
#include "value.hpp"
#include "error.hpp"

#include <memory>
#include <variant>
#include <optional>
#include <string>

namespace expr
{

enum class ExpressionType
{
	BRACKET,
	BINARY_OP,
	UNARY_OP,
	VALUE,
	VARIABLE,
};

class Expression
{
public:
	Expression(
		ExpressionType _type,
		Location const& _loc);

	virtual void insert_node(
		std::shared_ptr<Expression> const& node,
		std::shared_ptr<Expression>* prev = nullptr) = 0;

	virtual bytecodes_t generate_codes() const = 0;
	virtual std::optional<val::value_t> type_check(ParserScope const& scope) const = 0;
	// virtual void optimize(ParserScope const& scope) = 0;

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
		std::shared_ptr<Expression> const& _expr = nullptr);

	void insert_node(
		std::shared_ptr<Expression> const& node,
		std::shared_ptr<Expression>* prev = nullptr) override;

	bytecodes_t generate_codes() const;
	std::optional<val::value_t> type_check(ParserScope const& scope) const override;

public:
	int precedence() const override;

private:
	UnaryOpType type;
	std::shared_ptr<Expression> expr;
};


enum class BinaryOpType
{
	ADD, SUB, MULT, DIV,
	DOT
};

class BinaryOp : public Expression
{
public:
	BinaryOp(
		Location const& _loc,
		std::string const& _type,
		std::shared_ptr<Expression> const& _lhs = nullptr,
		std::shared_ptr<Expression> const& _rhs = nullptr);

	void insert_node(
		std::shared_ptr<Expression> const& node,
		std::shared_ptr<Expression>* prev = nullptr) override;

	bytecodes_t generate_codes() const override;
	std::optional<val::value_t> type_check(ParserScope const& scope) const override;

public:
	int precedence() const override;

private:
	BinaryOpType type;
	std::shared_ptr<Expression> lhs, rhs;
};


class Value : public Expression
{
public:
	Value(
		Location const& _loc,
		val::Value const& _val);

	void insert_node(
		std::shared_ptr<Expression> const& node,
		std::shared_ptr<Expression>* prev = nullptr)override;

	bytecodes_t generate_codes() const override;
	std::optional<val::value_t> type_check(ParserScope const& scope) const override;

public:
	int precedence() const override;

private:
	val::Value val;
};


class Variable : public Expression
{
public:
	Variable(
		Location const& _loc,
		std::string const& _name,
		bytecode_t _id);

	void insert_node(
		std::shared_ptr<Expression> const& node,
		std::shared_ptr<Expression>* prev = nullptr) override;

	bytecodes_t generate_codes() const override;
	std::optional<val::value_t> type_check(ParserScope const& scope) const override;

public:
	int precedence() const override;

private:
	std::string name;
	bytecode_t id;
};

}