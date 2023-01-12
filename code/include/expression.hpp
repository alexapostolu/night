#pragma once

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
	Expr(ExprType _type);

	virtual std::optional<std::shared_ptr<Expr>&> next();
	virtual int prec() const;
	virtual bool gaurd() const;

	ExprType type;
};

using expr_p = std::shared_ptr<Expr>;



enum class ExprConstantType
{
	CHAR,
	INT
};

struct ExprConstant
{
	ExprConstantType type;
	std::variant<char, int> val;
};



enum class ExprValueType
{
	CONSTANT,
	EXPRESSION
};

struct ExprValue : public Expr
{
	ExprValue(ExprValueType _type, std::variant<ExprConstant, expr_p> const& _val);

	ExprValueType type;
	std::variant<ExprConstant, expr_p> val;
};



enum class ExprUnaryType
{
	NOT
};

struct ExprUnary : public Expr
{
	ExprUnary(ExprUnaryType _type, expr_p const& _val);

	std::optional<expr_p&> next();

	ExprUnaryType type;
	expr_p val;
};



enum class ExprBinaryType
{
	ADD = 1,
	SUB = 1,
	MULT = 2,
	DIV = 2
};

struct ExprBinary : public Expr
{
	ExprBinary(ExprBinaryType _type, expr_p const& _lhs, expr_p const& _rhs);
	
	std::optional<expr_p&> next();
	int prec() const;
	bool gaurd() const;

	ExprBinaryType type;

	bool guard;

	expr_p lhs;
	expr_p rhs;
};