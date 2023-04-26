#include "parser.hpp"
#include "lexer.hpp"
#include "scope.hpp"
#include "bytecode.hpp"
#include "expression.hpp"
#include "error.hpp"

#include <iostream>
#include <variant>
#include <string>
#include <assert.h>
#include <unordered_map>

bytecodes_t parse_stmts(Lexer& lexer, Scope& upper_scope, bool* curly_enclosed)
{
	Scope scope{ upper_scope.vars };

	switch (lexer.eat().type)
	{
	case TokenType::OPEN_CURLY:
	{
		*curly_enclosed = true;

		bytecodes_t bytecodes;

		while (lexer.eat().type != TokenType::CLOSE_BRACKET)
		{
			if (lexer.eat().type == TokenType::END_OF_FILE)
				throw NIGHT_CREATE_FATAL("missing closing curly bracket");

			auto codes = parse_stmt(lexer, scope);
			bytecodes.insert(std::end(bytecodes), std::begin(codes), std::end(codes));
		}

		return bytecodes;
	}
	case TokenType::END_OF_FILE:
		return {};
	default:
		return parse_stmt(lexer, scope);
	}
}

bytecodes_t parse_stmt(Lexer& lexer, Scope& scope)
{
	switch (lexer.curr().type)
	{
	case TokenType::VARIABLE:
		return parse_var(lexer, scope);

	case TokenType::IF:
		return parse_if(lexer, scope, false);

	case TokenType::ELIF:
		return parse_if(lexer, scope, true);

	case TokenType::ELSE:
		return parse_else(lexer, scope);

	case TokenType::FOR:
		return parse_for(lexer, scope);

	case TokenType::WHILE:
		return parse_while(lexer, scope);

	case TokenType::RETURN:
		return parse_rtn(lexer, scope);

	case TokenType::CLOSE_CURLY:
		throw std::runtime_error("in parse_stmt: close curly bracket indicates empty curlys");

	case TokenType::END_OF_FILE:
		throw std::runtime_error("in parse_stmt: EOF should be handled in caller");

	default:
		throw NIGHT_CREATE_FATAL("unknown syntax '" + lexer.curr().str + "'");
	}
}

bytecodes_t parse_var(Lexer& lexer, Scope& scope)
{
	assert(lexer.curr().type == TokenType::VARIABLE);
	
	bytecodes_t codes;
	std::string var_name = lexer.curr().str;

	lexer.eat();

	// default values

	if (lexer.curr().type == TokenType::BOOL_TYPE ||
		lexer.curr().type == TokenType::CHAR_TYPE ||
		lexer.curr().type == TokenType::INT_TYPE)
	{
		BytecodeType var_type;
		ValueType val_type;
		switch (lexer.curr().type)
		{
		case TokenType::BOOL_TYPE:
			var_type = BytecodeType::BOOL_ASSIGN;
			val_type = ValueType::BOOL;
			break;
		case TokenType::CHAR_TYPE:
			var_type = BytecodeType::CHAR_ASSIGN;
			val_type = ValueType::CHAR;
			break;
		case TokenType::INT_TYPE:
			var_type = BytecodeType::INT_ASSIGN;
			val_type = ValueType::INT;
			break;
		default:
			throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected variable type or assignment");
		}

		if (lexer.eat().type == TokenType::SEMICOLON)
		{
			scope.vars[var_name] = val_type;

			codes.push_back({ lexer.loc, BytecodeType::CONSTANT, 0 });
			codes.push_back({ lexer.loc, var_type, (int)std::distance(std::begin(scope.vars), scope.vars.find(var_name)) });

			type_check::var_defined(lexer, scope, var_name);
			return codes;
		}
	}

	if (lexer.curr().type != TokenType::ASSIGN)
	{
		throw NIGHT_CREATE_FATAL("expected assignment or variable type after variable name '" + var_name + "'");
	}

	parse_var_assign(lexer, scope, codes, var_name);
	if (lexer.curr().type != TokenType::SEMICOLON)
		throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "' expected variable type or assignment");

	return codes;
}

bytecodes_t parse_if(Lexer& lexer, Scope& scope, bool is_elif)
{
	assert(lexer.curr().type == TokenType::IF);

	bytecodes_t codes;

	lexer.expect(TokenType::OPEN_BRACKET);

	auto expr = parse_expr_toks(lexer, scope);
	auto type = parse_expr(expr, codes);

	lexer.expect(TokenType::CLOSE_BRACKET);

	bool curly_enclosed = false;
	auto stmt_codes = parse_stmts(lexer, scope, &curly_enclosed);
	if (!curly_enclosed && stmt_codes.empty())
		NIGHT_CREATE_MINOR("if statement missing body");

	codes.push_back({ lexer.loc, is_elif ? BytecodeType::ELIF : BytecodeType::IF, (int)stmt_codes.size() });
	codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));

	if (type != ValueType::BOOL)
	{
		NIGHT_CREATE_MINOR("found '" + val_type_to_str(type) + "' condition, expected boolean condition");
	}

	return codes;
}

bytecodes_t parse_else(Lexer& lexer, Scope& scope)
{	
	bytecodes_t codes;

	auto stmt_codes = parse_stmts(lexer, scope);

	codes.push_back({ lexer.loc, BytecodeType::ELSE, (int)stmt_codes.size() });
	codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));

	return codes;
}

bytecodes_t parse_for(Lexer& lexer, Scope& scope)
{
	assert(lexer.curr().type == TokenType::FOR);

	lexer.expect(TokenType::OPEN_BRACKET);

	bytecodes_t codes;

	// variable

	std::string var_name = lexer.eat().str;
	auto bytes = parse_var(lexer, scope);
	codes.insert(std::end(codes), std::begin(bytes), std::end(bytes));

	lexer.expect(TokenType::SEMICOLON);
	
	// condition

	auto cond = parse_expr_toks(lexer, scope);
	auto cond_type = parse_expr(cond, codes);

	lexer.expect(TokenType::SEMICOLON);

	// increment

	lexer.expect(TokenType::VARIABLE);
	lexer.expect(TokenType::ASSIGN);

	parse_var_assign(lexer, scope, codes, var_name);

	lexer.expect(TokenType::CLOSE_BRACKET);

	// statements

	auto stmt_codes = parse_stmts(lexer, scope);

	codes.push_back({ lexer.loc, BytecodeType::FOR, (int)stmt_codes.size() });
	codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));

	// type checks

	type_check::var_defined(lexer, scope, var_name);

	if (cond_type != ValueType::BOOL)
	{
		NIGHT_CREATE_MINOR("found '" + val_type_to_str(cond_type) + "' expression, expected boolean expression");
	}

	return codes;
}

bytecodes_t parse_while(Lexer& lexer, Scope& scope)
{
	assert(lexer.curr().type == TokenType::WHILE);

	bytecodes_t codes;

	lexer.expect(TokenType::OPEN_BRACKET);

	auto cond_expr = parse_expr_toks(lexer, scope);
	auto cond_type = parse_expr(cond_expr, codes);

	lexer.expect(TokenType::CLOSE_BRACKET);

	// statements

	auto stmt_codes = parse_stmts(lexer, scope);
	codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));

	if (cond_type != ValueType::BOOL)
	{
		NIGHT_CREATE_MINOR("found '" + val_type_to_str(cond_type) + "' expression, expected boolean expression");
	}

	return codes;
}

bytecodes_t parse_func(Lexer& lexer, Scope& scope)
{
	assert(lexer.curr().type == TokenType::DEF);

	auto const& var_name = lexer.expect(TokenType::VARIABLE).str;

	lexer.expect(TokenType::OPEN_BRACKET);

	bytecodes_t codes;

	// parameters

	parse_comma_sep_stmts(lexer, scope, codes);

	// return values
	
	lexer.eat();
	if (lexer.curr().type != TokenType::BOOL_TYPE ||
		lexer.curr().type != TokenType::CHAR_TYPE ||
		lexer.curr().type != TokenType::INT_TYPE ||
		lexer.curr().type != TokenType::STR_TYPE)
	{
		throw NIGHT_CREATE_FATAL("expected type after parameters");
	}

	TokenType rtn_type = lexer.curr().type;

	// body

	bool curly_enclosed = false;
	auto stmt_codes = parse_stmts(lexer, scope, &curly_enclosed);
	if (!curly_enclosed)
		NIGHT_CREATE_MINOR("function body must be enclosed by curly brackets");

	codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));

	codes.push_back({ lexer.loc, BytecodeType::RETURN, 0 });

	return codes;
}

bytecodes_t parse_rtn(Lexer& lexer, Scope& scope)
{
	bytecodes_t codes;

	auto expr = parse_expr_toks(lexer, scope);
	auto type = parse_expr(expr, codes);

	codes.push_back({ lexer.loc, BytecodeType::RETURN });

	return codes;
}

void parse_var_assign(Lexer& lexer, Scope& scope, bytecodes_t& codes, std::string const& var_name)
{
	assert(lexer.curr().type == TokenType::ASSIGN);
	
	BytecodeType assign_type;
	if (lexer.curr().str == "=")
	{
		if (scope.vars[var_name] == ValueType::BOOL)
			assign_type = BytecodeType::BOOL_ASSIGN;
		if (scope.vars[var_name] == ValueType::CHAR)
			assign_type = BytecodeType::CHAR_ASSIGN;
		if (scope.vars[var_name] == ValueType::INT)
			assign_type = BytecodeType::INT_ASSIGN;
	}
	else if (lexer.curr().str == "+=")
	{
		assign_type = BytecodeType::ADD_ASSIGN;
	}
	else if (lexer.curr().str == "-=")
	{
		assign_type = BytecodeType::SUB_ASSIGN;
	}
	else if (lexer.curr().str == "*=")
	{
		assign_type = BytecodeType::MULT_ASSIGN;
	}
	else if (lexer.curr().str == "/=")
	{
		assign_type = BytecodeType::DIV_ASSIGN;
	}
	else
	{
		throw std::runtime_error("parse_var_assign unhandled case " + lexer.curr().str);
	}

	auto expr = parse_expr_toks(lexer, scope);
	if (!expr)
	{
		throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected expression after assignment");
	}

	auto expr_type = parse_expr(expr, codes);

	codes.push_back({ lexer.loc, BytecodeType::ADD_ASSIGN, (int)std::distance(std::begin(scope.vars), scope.vars.find(var_name)) });


	type_check::var_undefined(lexer, scope, var_name);
	type_check::var_assign_type(lexer, scope, var_name, assign_type);
	type_check::var_expr_type(lexer, scope, var_name, expr_type);
}

void parse_comma_sep_stmts(Lexer& lexer, Scope& scope, bytecodes_t& codes)
{
	assert(lexer.curr().type == TokenType::OPEN_BRACKET);

	while (true)
	{
		auto const& var_name = lexer.expect(TokenType::VARIABLE).str;

		lexer.eat();
		if (lexer.curr().type == TokenType::BOOL_TYPE ||
			lexer.curr().type == TokenType::CHAR_TYPE ||
			lexer.curr().type == TokenType::INT_TYPE)
		{
			BytecodeType var_type;
			ValueType val_type;
			switch (lexer.curr().type)
			{
			case TokenType::BOOL_TYPE:
				var_type = BytecodeType::BOOL_ASSIGN;
				val_type = ValueType::BOOL;
				break;
			case TokenType::CHAR_TYPE:
				var_type = BytecodeType::CHAR_ASSIGN;
				val_type = ValueType::CHAR;
				break;
			case TokenType::INT_TYPE:
				var_type = BytecodeType::INT_ASSIGN;
				val_type = ValueType::INT;
				break;
			default:
				throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected variable type or assignment");
			}

			Scope func_scope;
			func_scope.vars[var_name] = val_type;

			  NIGHT_CREATE_FATAL("expected variable type");
		}
	}

	if (lexer.curr().type != TokenType::CLOSE_BRACKET)
	{

	}
}

expr_p parse_expr_toks(Lexer& lexer, Scope& scope, bool bracket)
{
	expr_p head(nullptr);

	while (true)
	{
		switch (lexer.eat().type)
		{
		case TokenType::CHAR_LIT:
		{
			auto val = std::make_shared<ExprValue>(ValueType::CHAR, lexer.curr().str[0]);
			parse_expr_single(head, val);

			break;
		}
		case TokenType::INT_LIT:
		{
			auto val = std::make_shared<ExprValue>(ValueType::INT, std::stoi(lexer.curr().str));
			parse_expr_single(head, val);

			break;
		}
		case TokenType::VARIABLE:
		{
			auto val = std::make_shared<ExprVar>(lexer.curr().str);
			parse_expr_single(head, val);

			break;
		}
		case TokenType::UNARY_OP:
		{
			auto val = std::make_shared<ExprUnary>(str_to_unary_type(lexer.curr().str), nullptr);
			parse_expr_single(head, val);

			break;
		}
		case TokenType::BINARY_OP:
		{
			auto tok_type = str_to_binary_type(lexer.curr().str);

			assert(head);
			if (!head->next())
			{
				head = std::make_shared<ExprBinary>(tok_type, head, nullptr);
			}
			else
			{
				expr_p curr(head);

				assert(curr->next());
				while (curr->next()->next() && prec(tok_type) >= curr->next()->prec())
					curr = curr->next();

				if (curr == head && curr->prec() >= prec(tok_type))
					head = std::make_shared<ExprBinary>(tok_type, head, nullptr);
				else
					curr->next() = std::make_shared<ExprBinary>(tok_type, curr->next(), nullptr);
			}

			break;
		}
		case TokenType::OPEN_BRACKET:
		{
			auto val = parse_expr_toks(lexer, scope, true);
			val->set_guard();

			parse_expr_single(head, val);

			break;
		}
		case TokenType::CLOSE_BRACKET:
		{
			return head;
		}
		default:
			return head;
		}
	}
}

ValueType parse_expr(expr_p const& expr, bytecodes_t& bytes)
{
	assert(expr && "nullptr 'expr' should be handled by the caller");

	bytes.push_back(expr->to_bytecode());

	switch (expr->type)
	{
	case ExprType::VALUE:
		break;
	case ExprType::UNARY:
		parse_expr(expr->lhs, bytes);
		break;
	case ExprType::BINARY:
		parse_expr(expr->rhs, bytes);
		parse_expr(expr->lhs, bytes);
		break;
	default:
		throw std::runtime_error("parse_expr, missing case for ExprType '" + std::to_string((int)expr->type) + "'");
	}
}

void parse_expr_single(expr_p& head, expr_p const& val)
{
	if (!head)
	{
		head = val;
	}
	else
	{
		expr_p curr(head);
		while (curr->next())
			curr = curr->next();

		curr->next() = val;
	}
}

ExprUnaryType str_to_unary_type(std::string const& str)
{
	if (str == "!")
		return ExprUnaryType::NOT;

	throw std::runtime_error("str_to_unary_type, missing case for '" + str + "'");
}

ExprBinaryType str_to_binary_type(std::string const& str)
{
	if (str == "+")
		return ExprBinaryType::ADD;
	if (str == "-")
		return ExprBinaryType::SUB;
	if (str == "*")
		return ExprBinaryType::MULT;
	if (str == "/")
		return ExprBinaryType::DIV;

	throw std::runtime_error("str_to_binary_type, missing case for '" + str + "'");
}

void type_check::var_defined(Lexer const& lexer, Scope const& scope, std::string const& var_name)
{
	if (scope.vars.find(var_name) != std::end(scope.vars))
	{
		NIGHT_CREATE_MINOR("variable '" + var_name + "' is already defined");
	}
}

void type_check::var_undefined(Lexer const& lexer, Scope const& scope, std::string const& var_name)
{
	if (scope.vars.find(var_name) == std::end(scope.vars))
	{
		NIGHT_CREATE_MINOR("variable '" + var_name + "' is undefined");
	}
}

void type_check::var_assign_type(Lexer const& lexer, Scope& scope, std::string const& var_name, BytecodeType assign_type)
{
	switch (scope.vars[var_name])
	{
	case ValueType::INT:
		break;
	case ValueType::CHAR:
			NIGHT_CREATE_MINOR("variable '" + var_name + "' has type 'char', which is not compatable with operator '");
		break;
	default:
		throw std::runtime_error("typecheck::var_assign_type, unhandled case");
	}
}

void type_check::var_expr_type(Lexer const& lexer, Scope& scope, std::string const& var_name, ValueType expr_type)
{
	if (scope.vars[var_name] != expr_type)
	{
		NIGHT_CREATE_MINOR(std::string("expression of type '") + "expr_type" + "' is incompatable with variable '" + var_name + "' of type '" + "var_type");
	}
}