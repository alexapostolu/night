#pragma once

#include "lexer.hpp"
#include "token.hpp"

#include <memory>
#include <tuple>
#include <vector>
#include <string>

#define THROW_ERR_TYPE_CHECK_UNARY(t) \
	throw NIGHT_COMPILE_ERROR( \
		"operator '" + unary_op.data + "' is currently used on " + get_types_as_str(types), \
		"operator '" + unary_op.data + "' can only be used on " + t, \
		night::learn_operators, node->loc);

#define THROW_ERR_TYPE_CHECK_BINARY(side, types)																				  \
	throw NIGHT_COMPILE_ERROR(																									  \
			std::string(side) + " hand value of operator '" + binary_op.data + "' currently contains " + get_types_as_str(types), \
			"operator '" + binary_op.data + "' can only be used on " + types,													  \
			night::learn_operators, node->loc);

struct Parser
{
	static TypeContainer const all_types;
	static CheckFunctionContainer check_funcs;
	static CheckClassContainer check_classes;

	static ExprValue modify_type;
};

bool parse_statement(
	Lexer& lexer,
	std::shared_ptr<Scope>& scope
);

void parse_body(
	Lexer& lexer,
	std::shared_ptr<Scope>& scope
);

// starts at the token before the expression
// ends at the first non-expression token
std::tuple<std::shared_ptr<ExprNode>, TypeContainer> parse_expression(
	Lexer& lexer,
	std::vector<ExprValue> const& required_types = {}
);

std::tuple<ExprContainer, std::vector<TypeContainer> > parse_arguments(
	Lexer& lexer,
	std::shared_ptr<Scope>& curr_scope
);

TypeContainer parser_type_check_expr(
	std::shared_ptr<Scope> const& curr_scope,
	std::shared_ptr<ExprNode> const& node,

	// for parameters as they don't have types at first
	TypeContainer const& required_types = {}
);

void parser_type_check_num_expr(
	std::shared_ptr<Scope> const& curr_scope,
	std::shared_ptr<ExprNode> const& node
);

// ask fuzzy about this
// a note about 'is_reachable()':
/*
// 'is_reachable()' is used to determine whether a function parameter's
// types should be modified by an expression
//
// the types of a parameter is determined by the expressions in which it
// is used in
//
// however, expressions in a function's "local" scope are not counted as
// those expressions may never be encountered
//
	def func(x) {
		if (false)
			print(x / 3)

		return x
	}
//
// in the example, 'x' may never encounter the division operator, so
// adding type 'int' to 'x' could cause some false errors
//
	set b = true;
	func(b)        # error! 'b' must contain type: 'int'
//
// for this reason, only expressions in a function's "global" scope can
// affect the types of a parameter
//
	def func(x) {
		print(x && true) # 'x' contains type: 'bool'

		if (false)
			print(x / 3) # does not affect types of 'x'

		return x;
	}
//
// in a future update, variables' types will be specific to their scope,
// making type checking way more effective
*/
bool is_reachable(
	std::shared_ptr<Scope> const& scope
);


// returns true if 'op1' is evaluated before (has lower precedence than) 'op2'
bool lower_precedence(
	std::string const& op1,
	std::string const& op2
);

// searches current scope for variable, if not found, moves to upper scope;
std::pair<std::string const, CheckVariable>* get_variable(
	std::shared_ptr<Scope> const& scope,
	std::string const& var_name
);