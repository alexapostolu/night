#pragma once

#include "bytecode.hpp"
#include "type.hpp"

#include <memory>
#include <variant>
#include <optional>
#include <string>

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
	virtual void set_guard();

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

struct ExprVar : public Expr
{
	ExprVar(std::string const& _name);
	bytecode_t to_bytecode() const override;

	std::string name;
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
	int prec() const override;

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
	void set_guard() override;

	ExprBinaryType type;
	bool guard;
};

int prec(ExprBinaryType type);