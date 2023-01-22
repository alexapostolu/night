#pragma once

#include "bytecode.hpp"
#include "type.hpp"

#include <memory>
#include <variant>
#include <optional>

enum class ExprType
{
	VALUE,
	UNARY,
	BINARY
};

struct Expr
{
	Expr(ExprType _type, std::shared_ptr<Expr> const& _lhs, std::shared_ptr<Expr> const& _rhs);

	virtual std::shared_ptr<Expr>& next();
	virtual bytecode_t to_bytecode() const = 0;
	virtual int prec() const;

	ExprType type;
	std::shared_ptr<Expr> lhs, rhs;
};

using expr_p = std::shared_ptr<Expr>;



struct ExprValue : public Expr
{
	ExprValue(ValueType _type, std::variant<char, int> const& _val);
	bytecode_t to_bytecode() const override;

	ValueType type;
	std::variant<char, int> val;
};



enum class ExprUnaryType
{
	NOT
};

struct ExprUnary : public Expr
{
	ExprUnary(ExprUnaryType _type, expr_p const& _val);
	expr_p& next() override;
	bytecode_t to_bytecode() const override;

	ExprUnaryType type;
};



enum class ExprBinaryType
{
	ADD,
	SUB,
	MULT,
	DIV
};

struct ExprBinary : public Expr
{
	ExprBinary(ExprBinaryType _type, expr_p const& _lhs, expr_p const& _rhs);
	expr_p& next() override;
	bytecode_t to_bytecode() const override;

	int prec() const override;

	ExprBinaryType type;
};

int prec(ExprBinaryType type);