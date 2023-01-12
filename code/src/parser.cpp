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
	// testing!!
	Token tok;
	do {
		tok = lexer.eat();
		std::cout << (int)tok.type << ": " << tok.str << " :\n";
	} while (tok.type != TokenType::END_OF_FILE);

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
			throw night::error::get().create_fatal_error(
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
			throw night::error::get().create_fatal_error("unknown syntax");
		}

		bytecodes.insert(std::end(bytecodes), std::begin(bytecode), std::end(bytecode));
	}
}

bytecodes_t parse_var(Lexer& lexer, Scope& scope)
{
	bytecodes_t codes;

	std::string var_name = lexer.curr().str;
	CreateVariableType var_type;
	std::variant<char, int> var_val;

	switch (lexer.eat().type)
	{
	case TokenType::CHAR_TYPE:
		var_type = CreateVariableType::CHAR;
		var_val = 0;
		break;
	case TokenType::INT_TYPE:
		var_type = CreateVariableType::INT;
		var_val = 0;
		break;

	case TokenType::END_OF_FILE:
		throw night::error::get().create_fatal_error("");
	default:
		throw night::error::get().create_fatal_error("");
	}

	switch (lexer.eat().type)
	{
	case TokenType::SEMICOLON:
		break;
	case TokenType::ASSIGN:
	{
		auto expr = parse_expr(lexer, scope);

		break;
	}

	case TokenType::END_OF_FILE:
		throw night::error::get().create_fatal_error("");
	default:
		throw night::error::get().create_fatal_error("");
	}

	codes.push_back(std::make_shared<CreateVariable>(var_type, var_name));
	codes.push_back(std::make_shared<PushConstant>(var_val));

	return codes;
}

bytecodes_t parse_if(Lexer& lexer, Scope& scope)
{

}

expr_p parse_expr(Lexer& lexer, Scope& scope, bool bracket)
{
	expr_p head(nullptr);
	bool open_bracket = false;

	while (true)
	{
		switch (lexer.eat().type)
		{
		case TokenType::CHAR_LIT:
		{
			auto val = std::make_shared<ExprValue>(
				ExprValueType::CONSTANT,
				ExprConstant{ ExprConstantType::CHAR, (char)lexer.eat().str[0] });

			if (!head)
			{
				head = val;
			}
			else
			{
				expr_p curr(head);
				while (curr->next())
					curr = *curr->next();

				*curr->next() = val;
			}

			break;
		}
		case TokenType::INT_LIT:
		{
			auto val = std::make_shared<ExprValue>(
				ExprValueType::CONSTANT,
				ExprConstant{ ExprConstantType::INT, (int)std::stoi(lexer.eat().str) });

			if (!head)
			{
				head = val;
			}
			else
			{
				expr_p curr(head);
				while (curr->next())
					curr = *curr->next();

				*curr->next() = val;
			}

			break;
		}
		case TokenType::UNARY_OP:
		{
			auto val = std::make_shared<ExprUnary>(
				str_to_unary_type(lexer.eat().str), nullptr);

			if (!head)
			{
				head = val;
			}
			else
			{
				expr_p curr(head);
				while (curr->next())
					curr = *curr->next();

				*curr->next() = val;
			}

			break;
		}
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
				while (curr->next()->next() && (int)tok_type > (int)curr->prec())
					curr = *curr->next();

				*curr->next() = std::make_shared<ExprBinary>(tok_type, curr->next(), nullptr);
			}

			break;
		}
		case TokenType::OPEN_BRACKET:
		{
			auto val = std::make_shared<ExprValue>(
				ExprValueType::EXPRESSION,
				parse_expr(lexer, scope, true));

			if (!head)
			{
				head = val;
			}
			else
			{
				expr_p curr(head);
				while (curr->next())
					curr = *curr->next();

				*curr->next() = val;
			}

			break;
		}
		case TokenType::CLOSE_BRACKET:
		{
			if (!bracket)
			{
				throw night::fatal_error("your mom fat");
			}

			break;
		}
		default:
			break;
		}
	}

	return head;
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