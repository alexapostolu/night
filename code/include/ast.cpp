#include "ast.hpp"

If::If(Location const& _loc, std::shared_ptr<Expression> const& _cond_expr, AST_Block const& _block)
	: AST(_loc), cond_expr(_cond_expr), block(_block)
{
}