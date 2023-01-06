#include "expression.hpp"

expr_p ExprValue::next()
{
	return nullptr;
}

expr_p ExprUnary::next()
{
	return val;
}

expr_p ExprBinary::next()
{
	return rhs;
}