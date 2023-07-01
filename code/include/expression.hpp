#pragma once

#include "bytecode.hpp"
#include "scope.hpp"
#include "error.hpp"
#include "value.hpp"
#include "ast.hpp"

#include <memory>
#include <variant>
#include <optional>
#include <string>


class Expression
{
public:
	Expression();

	virtual bytecodes_t generate_codes(ParserScope const& scope) const = 0;
	virtual std::optional<value_t> type_check(ParserScope const& scope) const = 0;

protected:
	std::shared_ptr<Expression> lhs, rhs;
};


enum class ExprBinaryType
{
	ADD, SUB, MULT, DIV,
	DOT
};

class ExpressionBinary : public Expression
{
public:
	ExpressionBinary(
		ExprBinaryType _type,
		std::shared_ptr<Expression> const& _lhs = nullptr,
		std::shared_ptr<Expression> const& _rhs = nullptr);

	bytecodes_t generate_codes(ParserScope const& scope) const;
	std::optional<value_t> type_check(ParserScope const& scope) const override;

private:
	ExprBinaryType type;
};


class ExpressionValue : public Expression
{
public:
	bytecodes_t generate_codes(ParserScope const& scope) const;
};










enum class ExprType
{
	VALUE,
	UNARY,
	BINARY
};

class Expr
{
public:
	Expr(ExprType _type,
		std::shared_ptr<Expr> const& _lhs,
		std::shared_ptr<Expr> const& _rhs);

public:
	virtual std::shared_ptr<Expr>& next();

	virtual bytecodes_t to_bytecode() const = 0;
	
	// if expressions is good, returns type
	// otherwise returns std::nullopt and handles the error internally
	virtual std::optional<ValueType> type_check(Scope const& scope) const = 0;
	
	virtual int prec() const;
	
	void set_guard();

protected:
	bool gaurd;

	ExprType type;
	std::shared_ptr<Expr> lhs, rhs;
	Location loc;
};

using expr_p = std::shared_ptr<Expr>;



struct ExpressionValue : public Expression
{
	ExpressionValue(Value const& _val);
	bytecodes_t to_bytecode() const override;
	std::optional<ValueType> type_check(Scope const& scope) const override;

private:
	Value val;
};

struct ExprVar : public Expr
{
	ExprVar(std::string const& _name);
	bytecodes_t to_bytecode() const override;
	std::optional<ValueType> type_check(Scope const& scope) const override;

	std::string name;
	int index;
};



enum class ExprUnaryType
{
	NEGATIVE,
	NOT
};

struct ExprUnary : public Expr
{
	ExprUnary(ExprUnaryType _type, expr_p const& _val);
	expr_p& next() override;
	bytecodes_t to_bytecode() const override;
	std::optional<ValueType> type_check(Scope const& scope) const override;
	int prec() const override;

	ExprUnaryType unary_type;
};



enum class ExprBinaryType
{
	ADD,
	SUB,
	MULT,
	DIV,
	DOT
};

struct ExprBinary : public Expr
{
	ExprBinary(ExprBinaryType _type, expr_p const& _lhs, expr_p const& _rhs);
	expr_p& next() override;
	bytecodes_t to_bytecode() const override;
	std::optional<ValueType> type_check(Scope const& scope) const override;

	int prec() const override;
	void set_guard() override;

	ExprBinaryType binary_type;
	bool guard;
};

int prec(ExprBinaryType type);
std::string const& expr_bin_type_to_string(ExprBinaryType type);

void number_to_bytecode(bytecodes_t& codes, int64_t num);