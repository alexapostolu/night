#include "parser.hpp"
#include "lexer.hpp"
#include "scope.hpp"
#include "bytecode.hpp"
#include "expression.hpp"
#include "error.hpp"

#include <iostream>

bytecodes_t parse_stmts(Lexer& lexer, Scope& scope)
{
	// testing!!
	Token tok;
	do {
		tok = lexer.eat();
		std::cout << (int)tok.type << ": " << tok.val << " :\n";
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
			bytecode = parse_stmt_if(lexer, scope);
			break;

		case TokenType::ELIF:
		case TokenType::ELSE:
			throw night::error::get().create_fatal_error(
				lexer.curr().val + " statement does not precede an if or elif statement");

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

	std::string var_name = lexer.curr().val;
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
		codes.push_back(std::make_shared<CreateVariable>(var_type, var_name));
		codes.push_back(std::make_shared<PushConstant>(var_val));
		break;
	case TokenType::ASSIGN: {
		expr_p head(nullptr);

		while (true)
		{
			switch (lexer.eat().type)
			{
			case TokenType::CHAR_LIT: {
				expr_p curr(nullptr);

				if (!head)
				{
					curr = head;
				}
				else
				{
					while (curr->next())
						curr = curr->next();
				}

				curr->next() = std::make_shared<Expr>(lexer.eat().val[0]);

				break;
			}
			case TokenType::INT_LIT: {
				expr_p curr(nullptr);

				if (!head)
				{
					curr = head;
				}
				else
				{
					while (curr->next())
						curr = curr->next();
				}

				curr->next() = std::make_shared<Expr>(std::stoi(lexer.eat().val));

				break;
			}
			case TokenType::UNARY_OP: {
				expr_p curr(nullptr);

				if (!head)
				{
					curr = head;
				}
				else
				{
					while (curr->next())
						curr = curr->next();
				}

				curr = std::make_shared<ExprUnary>(lexer.eat().val, nullptr);

				break;
			}
			case TokenType::BINARY_OP: {
				expr_p curr(nullptr);

				if (!head->next())
				{
					head = std::make_shared<ExprBinary>(lexer.curr().str, head, nullptr);
				}
				else
				{
					while (curr->next()->next())
						curr = curr->next();

					curr->next() = std::make_shared<ExprBinary>(lexer.curr().str, curr->next(), nullptr);
				}

				break;
			}
			default:
				break;
			}
		}

		break;
	}

	case TokenType::END_OF_FILE:
		throw night::error::get().create_fatal_error("");
	default:
		throw night::error::get().create_fatal_error("");
	}

	return codes;
}

bytecodes_t parse_stmt_if(Lexer& lexer, Scope& scope)
{

}