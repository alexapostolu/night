#include <memory>
#include <variant>

enum class ExprType
{
	VALUE,
	UNARY,
	BINARY
};

struct Expr
{
	Expr(ExprType _type);

	virtual expr_p& next() = 0;
	virtual int prec() const = 0;

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

	expr_p& next();
	int prec() const;

	ExprValueType type;
	std::variant<ExprConstant, expr_p> val;
};



enum class ExprUnaryType
{
	NOT = 1
};

struct ExprUnary : public Expr
{
	ExprUnary(ExprUnaryType _type, expr_p const& _val);

	expr_p& next();
	int prec() const;

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
	ExprBinary(ExprBinaryType _type, std::vector<expr_p> const& _lhs, std::vector<expr_p> const& _rhs);
	
	expr_p& next();
	int prec() const;

	ExprBinaryType type;

	// we use std::vector for brackets
	std::vector<expr_p> lhs;
	std::vector<expr_p> rhs;
};