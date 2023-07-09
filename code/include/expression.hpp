#pragma once

#include "parser_scope.hpp"
#include "bytecode.hpp"
#include "error.hpp"
#include "value.hpp"
#include "ast.hpp"

#include <memory>
#include <variant>
#include <optional>
#include <string>

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
		Location const& _loc,
		std::shared_ptr<Expression> const& _lhs = nullptr,
		std::shared_ptr<Expression> const& _rhs = nullptr);

	virtual void insert_node(
		std::shared_ptr<Expression>& prev,
		std::shared_ptr<Expression> const& node) = 0;

	virtual bytecodes_t generate_codes(ParserScope const& scope) const = 0;
	virtual std::optional<value_t> type_check(ParserScope const& scope) const = 0;
	// virtual void optimize(ParserScope const& scope) = 0;

public:
	virtual int precedence() const = 0;

public:
	bool is_operator() const;
	bool is_value() const;

public:
	ExpressionType type;
protected:
	Location loc;
	std::shared_ptr<Expression> lhs, rhs;
};


class Bracket : public Expression
{
public:
	Bracket(
		Location const& _loc,
		std::shared_ptr<Expression> const& _expr);

	void insert_node(std::shared_ptr<Expression> const& node);
	bytecodes_t generate_codes(ParserScope const& scope) const;
	std::optional<value_t> type_check(ParserScope const& scope) const;

private:
	std::shared_ptr<Expression> expr;
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
		UnaryOpType _type,
		std::shared_ptr<Expression> const& _expr = nullptr);

	void insert_node(
		std::shared_ptr<Expression>& prev,
		std::shared_ptr<Expression> const& node);

	bytecodes_t generate_codes(ParserScope const& scope) const;
	std::optional<value_t> type_check(ParserScope const& scope) const override;

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
		BinaryOpType _type,
		std::shared_ptr<Expression> const& _lhs = nullptr,
		std::shared_ptr<Expression> const& _rhs = nullptr);

	void insert_node(
		std::shared_ptr<Expression>& prev,
		std::shared_ptr<Expression> const& node);

	bytecodes_t generate_codes(ParserScope const& scope) const;
	std::optional<value_t> type_check(ParserScope const& scope) const override;

public:
	int precedence() const;

private:
	ExprBinaryType type;
};


class ExpressionValue : public Expression
{
public:
	bytecodes_t generate_codes(ParserScope const& scope) const;

private:
	Value val;
};


class ExpressionVariable : public Expression
{
public:

private:
	bytecode_t id;
};




int prec(ExprBinaryType type);
std::string const& expr_bin_type_to_string(ExprBinaryType type);

void number_to_bytecode(bytecodes_t& codes, int64_t num);