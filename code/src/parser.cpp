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
	bytecodes_t bytecodes;

	while (true)
	{
		bytecodes_t bytecode;
		auto tok = lexer.eat();

		switch (tok.type)
		{
		case TokenType::VARIABLE:
			bytecode = parse_var(lexer, scope);
			break;

		case TokenType::IF:
			bytecode = parse_if(lexer, scope);
			break;

		case TokenType::ELIF:
		case TokenType::ELSE:
			throw NIGHT_CREATE_FATAL(
				lexer.curr().str + " statement does not precede an if or elif statement");

		case TokenType::FOR:
			bytecode = parse_for(lexer, scope);
			break;

		case TokenType::WHILE:
			bytecode = parse_while(lexer, scope);
			break;

		case TokenType::RETURN:
			bytecode = parse_rtn(lexer, scope);
			break;

		case TokenType::END_OF_FILE:
			return bytecodes;

		default:
			throw NIGHT_CREATE_FATAL("unknown syntax");
		}

		bytecodes.insert(std::end(bytecodes), std::begin(bytecode), std::end(bytecode));
	}

	std::cout << "parsing tokens\n";
	for (auto code : bytecodes)
	{
		std::cout << code->to_str() << '\n';
	}
}

bytecodes_t parse_var(Lexer& lexer, Scope& scope)
{
	bytecodes_t codes;

	std::string var_name = lexer.curr().str;
	ValueType var_type;
	std::variant<char, int> var_val;

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
		throw NIGHT_CREATE_FATAL("end of file reached, expected variable type");
	default:
		throw NIGHT_CREATE_FATAL("'" + lexer.curr().str + "' found, expected variable type");
	}

	switch (lexer.eat().type)
	{
	case TokenType::SEMICOLON:
		codes.push_back(std::make_shared<CreateConstant>(var_type, var_val));
		break;
	case TokenType::ASSIGN:
	{
		auto expr = parse_toks(lexer, scope);
		parse_expr(expr, codes);

		break;
	}

	case TokenType::END_OF_FILE:
		throw NIGHT_CREATE_FATAL("end of file reached, expected semicolon or assignment");
	default:
		throw NIGHT_CREATE_FATAL("'" + lexer.curr().str + "' found, expected semicolon or assignment");
	}

	codes.push_back(std::make_shared<CreateVariable>(var_type, var_name));
	codes.push_back(std::make_shared<StoreConstant>(var_name));

	return codes;
}

bytecodes_t parse_if(Lexer& lexer, Scope& scope)
{
	return {};
}

bytecodes_t parse_for(Lexer& lexer, Scope& scope)
{
	return {};
}

bytecodes_t parse_while(Lexer& lexer, Scope& scope)
{
	return {};
}

bytecodes_t parse_rtn(Lexer& lexer, Scope& scope)
{
	return {};
}

expr_p parse_toks(Lexer& lexer, Scope& scope, bool bracket)
{
	expr_p head(nullptr);
	expr_p guard(nullptr);
	bool open_bracket = false;

	while (true)
	{
		switch (lexer.eat().type)
		{
		case TokenType::CHAR_LIT:
			parse_expr_single(head, std::make_shared<ExprValue>(
				ValueType::CHAR, (char)lexer.curr().str[0]));

			break;
		case TokenType::INT_LIT:
			parse_expr_single(head, std::make_shared<ExprValue>(
				ValueType::INT, std::stoi(lexer.curr().str)));

			break;
		case TokenType::UNARY_OP:
			parse_expr_single(head, std::make_shared<ExprUnary>(
				str_to_unary_type(lexer.curr().str), nullptr));

			break;
		case TokenType::BINARY_OP:
		{
			expr_p curr(nullptr);

			auto tok_type = str_to_binary_type(lexer.curr().str);

			assert(head);
			if (!head->next())
			{
				head = std::make_shared<ExprBinary>(tok_type, head, nullptr);
			}
			else
			{
				assert(curr->next());
				while (curr->next()->next() && (int)tok_type > (int)curr->prec() && curr->next() != guard)
					curr = curr->next();

				curr->next() = std::make_shared<ExprBinary>(tok_type, curr->next(), nullptr);
			}

			break;
		}
		case TokenType::OPEN_BRACKET:
		{
			expr_p val = parse_toks(lexer, scope, true);

			if (!head)
			{
				head = val;
				guard = head;
			}
			else
			{
				expr_p curr(head);
				while (curr->next())
					curr = curr->next();

				curr->next() = val;
				guard = curr;
			}

			break;
		}
		case TokenType::CLOSE_BRACKET:
		{
			if (!bracket)
			{
				throw NIGHT_CREATE_FATAL("missing bracket");
			}

			break;
		}
		default:
			return head;
		}
	}
}

void parse_expr(expr_p const& expr, bytecodes_t& bytes)
{
	bytes.push_back(expr->to_bytecode());

	switch (expr->type)
	{
	case ExprType::VALUE:
		break;
	case ExprType::UNARY:
		parse_expr(expr->lhs, bytes);
		break;
	case ExprType::BINARY:
		parse_expr(expr->lhs, bytes);
		parse_expr(expr->rhs, bytes);
		break;
	default:
		throw "fuck";
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