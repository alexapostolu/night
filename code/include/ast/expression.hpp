#pragma once

#include "parser_scope.hpp"
#include "bytecode.hpp"
#include "value_type.hpp"
#include "error.hpp"

#include <functional>
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
	BINARY_OP,
	UNARY_OP,
	FUNCTION_CALL,
	VARIABLE,
	ARRAY,
	LITERAL,
};

class Expression
{
public:
	Expression(
		Location const& _loc,
		ExpressionType _type);

	virtual void insert_node(
		expr_p node,
		expr_p* prev = nullptr) = 0;

	/* Returns the type of the expression. If std::nullopt is returned, then
	 * type_check() has failed and at least one minor error has been created.
	 * No fatal errors should be thrown here.
	 */
	virtual std::optional<ValueType> type_check(
		ParserScope const& scope) noexcept = 0;

	/* Evaluates constant expressions and returns the new expression. Leaves
	 * non-constant expressions unchanged.
	 * Usage:
	 *    expr = expr->optimize(scope);
	 */
	[[nodiscard]]
	virtual expr_p optimize(
		ParserScope const& scope) = 0;
	
	virtual bytecodes_t generate_codes() const = 0;

	virtual int precedence() const = 0;

public:
	ExpressionType type;
	bool guard;

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
		std::string const& _op_type,
		expr_p const& _expr = nullptr,
		std::optional<BytecodeType> const& _op_code = std::nullopt);

	UnaryOp(
		Location const& _loc,
		UnaryOpType _type,
		expr_p const& _expr = nullptr,
		std::optional<BytecodeType> const& _op_code = std::nullopt);

	void insert_node(
		expr_p node,
		expr_p* prev = nullptr) override;

	std::optional<ValueType> type_check(ParserScope const& scope) noexcept override;

	[[nodiscard]]
	expr_p optimize(ParserScope const& scope) override;
	
	bytecodes_t generate_codes() const;

	int precedence() const override;

private:
	UnaryOpType op_type;
	expr::expr_p expr;

	std::optional<BytecodeType> op_code;
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
		std::string const& _type,
		expr_p const& _lhs = nullptr,
		expr_p const& _rhs = nullptr,
		std::optional<BytecodeType> const& _cast_lhs = std::nullopt,
		std::optional<BytecodeType> const& _cast_rhs = std::nullopt,
		BytecodeType const& _op_code = (BytecodeType)0);

	BinaryOp(
		Location const& _loc,
		BinaryOpType _op_type,
		expr_p const& _lhs = nullptr,
		expr_p const& _rhs = nullptr,
		std::optional<BytecodeType> const& _cast_lhs = std::nullopt,
		std::optional<BytecodeType> const& _cast_rhs = std::nullopt,
		BytecodeType const& _op_code = (BytecodeType)0);

	void insert_node(
		expr_p node,
		expr_p* prev = nullptr) override;

	std::optional<ValueType> type_check(
		ParserScope const& scope) noexcept override;
	expr_p optimize(ParserScope const& scope) override;
	bytecodes_t generate_codes() const override;

public:
	int precedence() const override;

private:
	BinaryOpType op_type;
	expr::expr_p lhs, rhs;

	std::optional<BytecodeType> cast_lhs, cast_rhs;
	BytecodeType op_code;
};


class Variable : public Expression
{
public:
	Variable(
		Location const& _loc,
		std::string const& _name,
		std::optional<bytecode_t> const& _id = std::nullopt);

	void insert_node(
		expr_p node,
		expr_p* prev = nullptr) override;

	std::optional<ValueType> type_check(
		ParserScope const& scope) noexcept override;

	[[nodiscard]]
	expr_p optimize(ParserScope const& scope) override;
	
	bytecodes_t generate_codes() const override;

	int precedence() const override;

private:
	std::string name;

	std::optional<bytecode_t> id;
};


class Array : public Expression
{
public:
	Array(
		Location const& _loc,
		std::vector<expr_p> const& _elements,
		bool _is_str_,
		std::optional<ValueType> const& _type_convert = std::nullopt,
		std::vector<std::optional<BytecodeType>> const& _type_conversion = {});

	void insert_node(
		expr_p node,
		expr_p* prev = nullptr) override;

	std::optional<ValueType> type_check(
		ParserScope const& scope) noexcept override;

	[[nodiscard]]
	expr_p optimize(ParserScope const& scope) override;

	bytecodes_t generate_codes() const override;

	int precedence() const override;

	bool is_str() const;

public:
	std::vector<expr_p> elements;

	// Note: Strings are stored as character arrays, but have special
	// properties such as being able to work with addition. This is why we have
	// a flag for it.
	bool is_str_;

	std::optional<ValueType> type_convert;
	std::vector<std::optional<BytecodeType>> type_conversion;
};


/* Note signed types are represented through a Unary Operator
 */
class Numeric : public Expression
{
public:
	Numeric(
		Location const& _loc,
		ValueType::PrimType _type,
		std::variant<int64_t, uint64_t, double> const& _val);

	void insert_node(
		expr_p node,
		expr_p* prev = nullptr) override;

	std::optional<ValueType> type_check(
		ParserScope const& scope) noexcept override;

	[[nodiscard]]
	expr_p optimize(
		ParserScope const& scope) override;
	
	bytecodes_t generate_codes() const override;

	int precedence() const override;

	bool is_true() const;

public:
	std::variant<int64_t, uint64_t, double> val;

private:
	ValueType::PrimType type;
};

}
