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

bytecodes_t parse_stmts(Lexer& lexer, Scope& upper_scope)
{
	Scope scope{ upper_scope.vars };

	bool curly_bracket = false;

	bytecodes_t bytecodes;

	while (true)
	{
		switch (lexer.eat().type)
		{
		case TokenType::CLOSE_CURLY:
		{
			if (!curly_bracket) {
				throw NIGHT_CREATE_FATAL("found close curly bracket, missing opening curly bracket");
			}

			return bytecodes;
		}
		case TokenType::OPEN_CURLY:
		{
			curly_bracket = true;
			lexer.eat();

			auto bytecode = parse_stmt(lexer, scope);
			if (bytecode.empty())
			{
				// empty indicates empty curlys
				return bytecodes;
			}

			bytecodes.insert(std::end(bytecodes), std::begin(bytecode), std::end(bytecode));

			break;
		}
		case TokenType::END_OF_FILE:
		{
			if (curly_bracket) {
				throw NIGHT_CREATE_FATAL("found end of file, missing opening curly bracket");
			}

			return bytecodes;
		}
		default:
		{
			auto bytecode = parse_stmt(lexer, scope);
			bytecodes.insert(std::end(bytecodes), std::begin(bytecode), std::end(bytecode));

			break;
		}
		}
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
	bytecodes_t codes;

	assert(lexer.curr().type == TokenType::VARIABLE && "variable type should be handled by the caller");
	std::string var_name = lexer.curr().str;

	ValueType var_type;
	std::variant<char, int> var_val;

	lexer.eat();

	if (lexer.curr().type == TokenType::ASSIGN)
	{
		parse_var_assign(lexer, scope, codes, var_name);

		if (lexer.curr().type != TokenType::SEMICOLON)
		{

		}
	}
	else if (lexer.curr().type == TokenType::SEMICOLON)
	{
		codes.push_back(std::make_shared<Constant>(var_type, var_val));
	}

	if (lexer.eat().type != TokenType::ASSIGN)
	{
		// type and default value
		switch (lexer.curr().type)
		{
		case TokenType::CHAR_TYPE:
			var_type = ValueType::CHAR;
			var_val = '\0';
			break;
		case TokenType::INT_TYPE:
			var_type = ValueType::INT;
			var_val = 0;
			break;
		default:
			throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected variable type or assignment");
		}
	}

	if (lexer.eat().type == TokenType::SEMICOLON)
	{
		codes.push_back(std::make_shared<Constant>(var_type, var_val));
	}
	else
	{
		parse_var_assign(lexer, scope, codes, var_name);
	}

	codes.push_back(std::make_shared<CreateVariable>(var_type, var_name));
	codes.push_back(std::make_shared<StoreConstant>(var_name));

	return codes;
}

bytecodes_t parse_if(Lexer& lexer, Scope& scope, bool is_elif)
{
	bytecodes_t codes;

	lexer.expect(TokenType::OPEN_BRACKET);

	auto expr = parse_expr_toks(lexer, scope);
	auto type = parse_expr(expr, codes);

	codes.push_back(std::make_shared<Bytecode>(
		 is_elif ? BytecodeType::ELIF : BytecodeType::IF));

	lexer.expect(TokenType::CLOSE_BRACKET);

	auto stmt_bytes = parse_stmts(lexer, scope);
	codes.insert(std::end(codes), std::begin(stmt_bytes), std::end(stmt_bytes));

	codes.push_back(std::make_shared<Bytecode>(BytecodeType::END_IF));

	return codes;
}

bytecodes_t parse_else(Lexer& lexer, Scope& scope)
{
	bytecodes_t codes;
	codes.push_back(std::make_shared<Bytecode>(BytecodeType::ELSE));

	auto stmt_codes = parse_stmts(lexer, scope);
	codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));

	codes.push_back(std::make_shared<Bytecode>(BytecodeType::END_IF));

	return codes;
}

bytecodes_t parse_for(Lexer& lexer, Scope& scope)
{
	assert(lexer.curr().type == TokenType::FOR);

	lexer.expect(TokenType::OPEN_BRACKET);

	bytecodes_t codes;

	std::string var_name = lexer.eat().str;
	auto bytes = parse_var(lexer, scope);
	codes.insert(std::end(codes), std::begin(bytes), std::end(bytes));

	lexer.expect(TokenType::SEMICOLON);

	auto expr = parse_expr_toks(lexer, scope);
	auto expr_type = parse_expr(expr, codes);

	lexer.expect(TokenType::SEMICOLON);

	// increment

	lexer.expect(TokenType::VARIABLE);
	lexer.expect(TokenType::ASSIGN);

	parse_var_assign(lexer, scope, codes, var_name);

	lexer.expect(TokenType::CLOSE_BRACKET);

	auto stmt_codes = parse_stmts(lexer, scope);
	codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));


	type_check::var_defined(scope, var_name);

	if (expr_type != ValueType::BOOL)
	{
		NIGHT_CREATE_MINOR("found '" + val_type_to_str(expr_type) + "' expression, expected boolean expression");
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

	auto stmt_codes = parse_stmts(lexer, scope);
	codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));

	if (cond_type != ValueType::BOOL)
	{
		NIGHT_CREATE_MINOR("found '" + val_type_to_str(cond_type) + "' expression, expected boolean expression");
	}

	return codes;
}

bytecodes_t parse_rtn(Lexer& lexer, Scope& scope)
{
	bytecodes_t codes;

	auto expr = parse_expr_toks(lexer, scope);
	auto type = parse_expr(expr, codes);

	codes.push_back(std::make_shared<Bytecode>(BytecodeType::RETURN));

	return codes;
}

void parse_var_assign(Lexer& lexer, Scope& scope, bytecodes_t& codes, std::string const& var_name)
{
	assert(lexer.curr().type == TokenType::ASSIGN);
	
	BytecodeType assign_type;
	if (lexer.curr().str == "=")
	{
		assign_type = BytecodeType::ASSIGN;
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
		throw std::runtime_error("unhandled case in parse_var_assign");
	}

	codes.push_back(std::make_shared<Variable>(var_name));

	auto expr = parse_expr_toks(lexer, scope);
	if (!expr)
	{
		throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected expression after assignment");
	}

	auto expr_type = parse_expr(expr, codes);

	codes.push_back(std::make_shared<Bytecode>(assign_type));


	type_check::var_undefined(scope, var_name);
	type_check::var_assign_type(scope, var_name, assign_type);
	type_check::var_expr_type(scope, var_name, expr_type);
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
		throw std::runtime_error("unhandled case");
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

	throw std::runtime_error("tf");
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

	throw std::runtime_error("bruh");
}

void type_check::var_defined(Scope const& scope, std::string const& var_name)
{
	if (scope.vars.find(var_name) != std::end(scope.vars))
	{
		NIGHT_CREATE_MINOR("variable '" + var_name + "' is already defined");
	}
}

void type_check::var_undefined(Scope const& scope, std::string const& var_name)
{
	if (scope.vars.find(var_name) == std::end(scope.vars))
	{
		NIGHT_CREATE_MINOR("variable '" + var_name + "' is undefined");
	}
}

void type_check::var_assign_type(Scope& scope, std::string const& var_name, BytecodeType assign_type)
{
	switch (scope.vars[var_name])
	{
	case ValueType::INT:
		break;
	case ValueType::CHAR:
			NIGHT_CREATE_MINOR("variable '" + var_name + "' has type 'char', which is not compatable with operator '");
		break;
	default:
		throw NIGHT_INTERNAL_ERROR("unhandled case");
	}
}

void type_check::var_expr_type(Scope& scope, std::string const& var_name, ValueType expr_type)
{
	if (scope.vars[var_name] != expr_type)
	{
		NIGHT_CREATE_MINOR(std::string("expression of type '") + "expr_type" + "' is incompatable with variable '" + var_name + "' of type '" + "var_type");
	}
}