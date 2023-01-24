#include "parser.hpp"
#include "lexer.hpp"
#include "scope.hpp"
#include "bytecode.hpp"
#include "expression.hpp"
#include "error.hpp"

#include <iostream>
#include <string>
#include <assert.h>
#include <unordered_map>

bytecodes_t parse_stmts(Lexer& lexer, Scope& scope)
{
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
			bytecodes.insert(std::end(bytecodes), std::begin(bytecode), std::end(bytecode));

			break;
		}
		case TokenType::END_OF_FILE:
		{
			if (curly_bracket) {
				throw NIGHT_CREATE_FATAL("found close end of file, missing opening curly bracket");
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
	bytecodes_t bytecodes;

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
		bytecodes = parse_for(lexer, scope);
		break;

	case TokenType::WHILE:
		bytecodes = parse_while(lexer, scope);
		break;

	case TokenType::RETURN:
		return parse_rtn(lexer, scope);

	case TokenType::END_OF_FILE:
		throw std::runtime_error("EOF handled in parse_stmts(), not here");

	default:
		throw NIGHT_CREATE_FATAL("unknown syntax");
	}
}

bytecodes_t parse_var(Lexer& lexer, Scope& scope)
{
	bytecodes_t codes;

	std::string var_name = lexer.curr().str;

	ValueType var_type;
	std::variant<char, int> var_val;

	// type and default value
	switch (lexer.eat().type)
	{
	case TokenType::CHAR_TYPE:
		var_type = ValueType::CHAR;
		var_val = '\0';
		break;
	case TokenType::INT_TYPE:
		var_type = ValueType::INT;
		var_val = 0;
		break;

	case TokenType::END_OF_FILE:
		throw NIGHT_CREATE_FATAL("reached end of file, expected variable type");
	default:
		throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected variable type");
	}

	// assignment
	switch (lexer.eat().type)
	{
	case TokenType::SEMICOLON:
	{
		codes.push_back(std::make_shared<CreateConstant>(var_type, var_val));
		break;
	}
	case TokenType::ASSIGN:
	{
		bytecodes_t bytes;
		auto expr = parse_expr_toks(lexer, scope);
		auto expr_type = parse_expr(expr, bytes);
		codes.insert(std::end(codes), std::rbegin(bytes), std::rend(bytes));

		if (var_type != expr_type)
		{
			NIGHT_CREATE_MINOR("variable '" + var_name + "' assigned expression of type '" + val_type_to_str(expr_type) +
				"', but has type '" + val_type_to_str(var_type) + "'");
		}

		break;
	}

	case TokenType::END_OF_FILE:
		throw NIGHT_CREATE_FATAL("reached end of file, expected semicolon or assignment");
	default:
		throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected semicolon or assignment");
	}

	codes.push_back(std::make_shared<CreateVariable>(var_type, var_name));
	codes.push_back(std::make_shared<StoreConstant>(var_name));

	return codes;
}

bytecodes_t parse_if(Lexer& lexer, Scope& scope, bool is_elif)
{
	if (lexer.eat().type != TokenType::OPEN_BRACKET)
	{
		throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected open bracket");
	}

	bytecodes_t codes;

	auto expr = parse_expr_toks(lexer, scope);
	auto type = parse_expr(expr, codes);

	codes.push_back(std::make_shared<Bytecode>(
		 is_elif ? BytecodeType::ELIF : BytecodeType::IF));

	if (lexer.curr().type != TokenType::CLOSE_BRACKET)
	{
		throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected closing bracket");
	}

	Scope stmt_scope{ scope.vars };

	auto stmt_bytes = parse_stmts(lexer, stmt_scope);
	codes.insert(std::end(codes), std::begin(stmt_bytes), std::end(stmt_bytes));

	codes.push_back(std::make_shared<Bytecode>(BytecodeType::END_IF));

	return codes;
}

bytecodes_t parse_else(Lexer& lexer, Scope& scope)
{
	Scope else_scope{ scope.vars };
	return parse_stmts(lexer, else_scope);
}

bytecodes_t parse_for(Lexer& lexer, Scope& scope)
{
	if (lexer.eat().type != TokenType::OPEN_BRACKET) {
		throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected open bracket");
	}

	bytecodes_t codes;

	auto bytes = parse_var(lexer, scope);
	codes.insert(std::end(codes), std::begin(bytes), std::end(bytes));

	if (lexer.curr().type != TokenType::SEMICOLON) {
		throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "', expected semicolon");
	}
}

bytecodes_t parse_while(Lexer& lexer, Scope& scope)
{
	return {};
}

bytecodes_t parse_rtn(Lexer& lexer, Scope& scope)
{
	return {};
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
				while (curr->next()->next() && prec(tok_type) > curr->prec())
					curr = curr->next();

				if (curr == head && curr->prec() > prec(tok_type))
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
			if (!bracket)
			{
				throw NIGHT_CREATE_FATAL("missing bracket");
			}

			return head;
		}
		default:
			return head;
		}
	}
}

ValueType parse_expr(expr_p const& expr, bytecodes_t& bytes)
{
	assert(expr);
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

ExprUnaryType str_to_unary_type(std::string_view str)
{
	if (str == "!")
		return ExprUnaryType::NOT;

	throw std::runtime_error("tf");
}

ExprBinaryType str_to_binary_type(std::string_view str)
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