#include <memory>
#include <variant>

struct Expr;
using expr_p = std::shared_ptr<Expr>;

enum class ExprType
{
	VALUE,
	UNARY,
	BINARY
};

struct Expr
{
	ExprType type;
	virtual expr_p next() = 0;
};



enum class ExprValueType
{
	CHAR,
	INT
};

struct ExprValue : public Expr
{
	std::variant<char, int> val;
	expr_p next();
};



enum class ExprUnaryType
{
	NOT
};

struct ExprUnary : public Expr
{
	ExprUnaryType type;
	expr_p val;

	expr_p next();
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
	ExprBinaryType type;

	expr_p lhs;
	expr_p rhs;

	expr_p next();
};