#include "parser.hpp"
#include "lexer.hpp"
#include "scope.hpp"
#include "bytecode.hpp"
#include "expression.hpp"
#include "value.hpp"
#include "error.hpp"

#include <iostream>
#include <variant>
#include <string>
#include <assert.h>
#include <unordered_map>
#include <limits>

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
			if (lexer.curr().type == TokenType::END_OF_FILE)
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
	assert(lexer.curr().type != TokenType::CLOSE_CURLY && "should be handled by caller");
	assert(lexer.curr().type != TokenType::END_OF_FILE && "should be handled by caller");

	bytecodes_t codes;

	switch (lexer.curr().type)
	{
	case TokenType::VARIABLE:
		generate_codes_var(codes, lexer, scope);
		if_stmt = false;
		break;
	case TokenType::IF:		  generate_codes_if(codes, lexer, scope, false); if_stmt = true break;
	case TokenType::ELIF:
		if (!if_stmt)
			NIGHT_CREATE_MINOR("missing if statement before elif");
		if_stmt = true;
		generate_codes_if(codes, lexer, scope, true);  break;
	case TokenType::ELSE:	  return parse_else(lexer, scope);
		if (!if_stmt)
			NIGHT_CREATE_MINOR("missing if statement before elif");
	case TokenType::FOR:	  return parse_for(lexer, scope);
	case TokenType::WHILE:	  return parse_while(lexer, scope);
	case TokenType::DEF:	  return parse_func(lexer, scope);
	case TokenType::RETURN:	  return parse_rtn(lexer, scope);

	default: throw NIGHT_CREATE_FATAL("unknown syntax '" + lexer.curr().str + "'");
	}

	return codes;
}

void generate_codes_var(bytecodes_t& codes, Lexer& lexer, Scope& scope)
{
	assert(lexer.curr().type == TokenType::VARIABLE);

	std::string var_name = lexer.curr().str;

	lexer.eat();

	if (lexer.curr().is_type())
		generate_codes_var_init(codes, lexer, scope, var_name);
	else if (lexer.curr().type == TokenType::ASSIGN)
		generate_codes_var_assign(codes, lexer, scope, var_name, true);
	else
		throw NIGHT_CREATE_FATAL("expected variable type or assignment after variable name '" + var_name + "'");
}

void generate_codes_if(bytecodes_t& codes, Lexer& lexer, Scope& scope, bool is_elif)
{
	assert(lexer.curr().type == TokenType::IF);

	// parse condition

	lexer.expect(TokenType::OPEN_BRACKET);

	auto cond_expr = parse_toks_expr(lexer, scope,
		"expected expression after opening bracket in if statement");

	auto cond_type = cond_expr->type_check(scope);

	if (cond_type.has_value() && *cond_type == ValueType::OBJECT)
		NIGHT_CREATE_MINOR("condition of type '" + val_type_to_str(*cond_type) + "', expected type 'bool', 'char', or 'int'");

	generate_codes_expr(codes, cond_expr);

	lexer.expect(TokenType::CLOSE_BRACKET);

	// parse statements

	bool curly_enclosed = false;
	auto stmt_codes = parse_stmts(lexer, scope, &curly_enclosed);

	if (!curly_enclosed && stmt_codes.empty())
		NIGHT_CREATE_MINOR("if statement missing body");

	codes.push_back((bytecode_t)(is_elif ? BytecodeType::ELIF : BytecodeType::IF));
	codes.push_back((bytecode_t)stmt_codes.size());
	codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));
}

void generate_codes_else(bytecodes_t& codes, Lexer& lexer, Scope& scope)
{
	assert(lexer.curr().type == TokenType::ELSE);

	auto stmt_codes = parse_stmts(lexer, scope);

	codes.push_back((bytecode_t)BytecodeType::ELSE);
	codes.push_back((bytecode_t)stmt_codes.size());
	codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));
}

void generate_codes_for(bytecodes_t& codes, Lexer& lexer, Scope& scope)
{
	assert(lexer.curr().type == TokenType::FOR);

	lexer.expect(TokenType::OPEN_BRACKET);

	bytecodes_t codes;

	// variable initialization

	std::string var_name = lexer.eat().str;
	type_check::var_undefined(lexer, scope, var_name);

	if (!lexer.eat().is_type())
		throw NIGHT_CREATE_FATAL("expected variable type after variable name in for loop initialization");

	generate_codes_var_init(codes, lexer, scope, var_name);
	
	// condition

	lexer.eat();

	auto cond_expr = parse_toks_expr(lexer, scope,
		"expected condition after variable initialization in for loop");

	if (lexer.curr().type != TokenType::SEMICOLON)
		throw NIGHT_CREATE_FATAL("expected semicolon after condition in for loop");

	auto cond_type = cond_expr->type_check(scope);

	if (cond_type.has_value() && *cond_type == ValueType::OBJECT)
		NIGHT_CREATE_MINOR("for loop condition is type '" + val_type_to_str(*cond_type) + "', expected type 'bool', 'char', or 'int'");

	generate_codes_expr(codes, cond_expr);

	// increment

	auto inc_name = lexer.expect(TokenType::VARIABLE);
	type_check::var_defined(lexer, scope, var_name);

	lexer.expect(TokenType::ASSIGN);

	generate_codes_var_assign(codes, lexer, scope, var_name, false);

	if (lexer.curr().type != TokenType::CLOSE_BRACKET)
		throw NIGHT_CREATE_FATAL("expected closing bracket after for loop increment");

	// statements

	auto stmt_codes = parse_stmts(lexer, scope);

	codes.push_back((bytecode_t)BytecodeType::FOR);
	codes.push_back((bytecode_t)stmt_codes.size());
	codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));
}

void generate_codes_while(bytecodes_t& codes, Lexer& lexer, Scope& scope)
{
	assert(lexer.curr().type == TokenType::WHILE);

	lexer.expect(TokenType::OPEN_BRACKET);

	auto cond_expr = parse_toks_expr(lexer, scope,
		"expected expression after opening bracket in while loop");

	auto cond_type = cond_expr->type_check(scope);

	if (cond_type.has_value() && *cond_type == ValueType::OBJECT)
		NIGHT_CREATE_MINOR("while loop condition is type '" + val_type_to_str(*cond_type) + "', expected type 'bool', 'char', or 'int'");

	if (lexer.curr().type != TokenType::CLOSE_BRACKET)
		throw NIGHT_CREATE_FATAL("expected closing bracket after while loop condition");

	// statements

	auto stmt_codes = parse_stmts(lexer, scope);

	codes.push_back((bytecode_t)BytecodeType::WHILE);
	codes.push_back((bytecode_t)stmt_codes.size());

	codes.insert(std::end(codes), std::begin(stmt_codes), std::end(stmt_codes));
}

bytecodes_t parse_func(Lexer& lexer, Scope& scope)
{
	assert(lexer.curr().type == TokenType::DEF);

	auto const& func_name = lexer.expect(TokenType::VARIABLE).str;

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

	if (scope.funcs.contains(func_name))
		NIGHT_CREATE_MINOR("function already defined");

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

BytecodeType token_var_type_to_bytecode(std::string const& type)
{
	if (type == "int8")
		return BytecodeType::S_INT1;
	else if (type == "int16")
		return BytecodeType::S_INT2;
	else if (type == "int32")
		return BytecodeType::S_INT4;
	else if (type == "int64")
		return BytecodeType::S_INT8;
	else if (type == "uint8")
		return BytecodeType::U_INT1;
	else if (type == "uint16")
		return BytecodeType::U_INT2;
	else if (type == "uint32")
		return BytecodeType::U_INT4;
	else if (type == "uint64")
		return  BytecodeType::U_INT8;
	else
		night::unhandled_case(type);
}

void number_to_bytecode(std::string const& s_num, bytecodes_t& codes)
{
	assert(s_num.length());

	if (s_num[0] != '-')
	{
		assert(s_num.length() > 1);

		uint64_t uint64 = std::stoull(s_num);

		if (uint64 <= std::numeric_limits<uint8_t>::max())
			codes.push_back((bytecode_t)BytecodeType::U_INT1);
		else if (uint64 <= std::numeric_limits<uint16_t>::max())
			codes.push_back((bytecode_t)BytecodeType::U_INT2);
		else if (uint64 <= std::numeric_limits<uint32_t>::max())
			codes.push_back((bytecode_t)BytecodeType::U_INT4);
		else if (uint64 <= std::numeric_limits<uint64_t>::max())
			codes.push_back((bytecode_t)BytecodeType::U_INT8);
		else {}

		do {
			codes.push_back(uint64 & 0xFF);
		} while (uint64 >>= 8);
	}
	else
	{
		int64_t int64 = std::stoll(s_num);

		if (int64 <= std::numeric_limits<uint8_t>::max())
			codes.push_back((bytecode_t)BytecodeType::S_INT1);
		else if (int64 <= std::numeric_limits<uint16_t>::max())
			codes.push_back((bytecode_t)BytecodeType::S_INT2);
		else if (int64 <= std::numeric_limits<uint32_t>::max())
			codes.push_back((bytecode_t)BytecodeType::S_INT4);
		else if (int64 <= std::numeric_limits<uint64_t>::max())
			codes.push_back((bytecode_t)BytecodeType::S_INT8);
		else {}

		do {
			codes.push_back(int64 & 0xFF);
		} while (int64 >>= 8);
	}
}

void number_to_bytecode(int64_t num, bytecodes_t& codes)
{
	if (num >= 0)
	{
		uint64_t uint64 = num;

		if (uint64 <= std::numeric_limits<uint8_t>::max())
			codes.push_back((bytecode_t)BytecodeType::U_INT1);
		else if (uint64 <= std::numeric_limits<uint16_t>::max())
			codes.push_back((bytecode_t)BytecodeType::U_INT2);
		else if (uint64 <= std::numeric_limits<uint32_t>::max())
			codes.push_back((bytecode_t)BytecodeType::U_INT4);
		else if (uint64 <= std::numeric_limits<uint64_t>::max())
			codes.push_back((bytecode_t)BytecodeType::U_INT8);
		else {}

		do {
			codes.push_back(uint64 & 0xFF);
		} while (uint64 >>= 8);
	}
	else
	{
		int64_t int64 = num;

		if (int64 <= std::numeric_limits<uint8_t>::max())
			codes.push_back((bytecode_t)BytecodeType::S_INT1);
		else if (int64 <= std::numeric_limits<uint16_t>::max())
			codes.push_back((bytecode_t)BytecodeType::S_INT2);
		else if (int64 <= std::numeric_limits<uint32_t>::max())
			codes.push_back((bytecode_t)BytecodeType::S_INT4);
		else if (int64 <= std::numeric_limits<uint64_t>::max())
			codes.push_back((bytecode_t)BytecodeType::S_INT8);
		else {}

		do {
			codes.push_back(int64 & 0xFF);
		} while (int64 >>= 8);
	}
}

void generate_codes_var_init(bytecodes_t& codes, Lexer& lexer, Scope& scope,
	std::string const& var_name)
{
	assert(lexer.curr().is_type());

	auto var_type = token_var_type_to_bytecode(lexer.curr().str);
	auto val_type = bytecode_type_to_val_type(var_type);

	bytecode_t var_index = scope.vars.size() - 1;

	scope.vars[var_name] = { val_type, var_index };
	type_check::var_defined(lexer, scope, var_name);

	lexer.eat();

	if (lexer.curr().type == TokenType::SEMICOLON)
	{
		// case:
		//   var int;

		codes.push_back((bytecode_t)var_type);
		codes.push_back(0);
	}
	else if (lexer.curr().type == TokenType::ASSIGN)
	{
		// case:
		//   var int = [expression];

		auto expr = parse_toks_expr(lexer, scope,
			"found '" + lexer.curr().str + "', expected expression after assignment");

		if (lexer.curr().type != TokenType::SEMICOLON)
			NIGHT_CREATE_FATAL("expected semicolon after variable initialization");

		auto expr_type = expr->type_check(scope);

		if (expr_type.has_value() && expr_type != val_type)
			NIGHT_CREATE_MINOR("expression of type '" + val_type_to_str(val_type) + "' does not match with variable of type '" + bytecode_to_str(var_type) + "'");

		generate_codes_expr(codes, expr);
	}
	else
	{
		throw NIGHT_CREATE_FATAL("expected semicolon or assignment after variable type");
	}

	codes.push_back((bytecode_t)BytecodeType::ASSIGN);
	codes.push_back(var_index);
}

void generate_codes_var_assign(bytecodes_t& codes, Lexer& lexer, Scope& scope,
	std::string const& var_name, bool require_semicolon)
{
	assert(lexer.curr().type == TokenType::ASSIGN);
	std::string const op = lexer.curr().str;
	
	BytecodeType assign_type;

	if (op == "=")		 assign_type = BytecodeType::ASSIGN;
	else if (op == "+=") assign_type = BytecodeType::ADD;
	else if (op == "-=") assign_type = BytecodeType::SUB;
	else if (op == "*=") assign_type = BytecodeType::MULT;
	else if (op == "/=") assign_type = BytecodeType::DIV;
	else throw night::unhandled_case(lexer.curr().str);


	auto expr = parse_toks_expr(lexer, scope,
		"found '" + lexer.curr().str + "', expected expression after assignment");

	auto expr_type = expr->type_check(scope);

	if (expr_type.has_value() && expr_type == ValueType::OBJECT)
		NIGHT_CREATE_MINOR("operator '" + op + "' can not be used on expression of type '" + val_type_to_str(*expr_type) + "'")

	generate_codes_expr(codes, expr);


	codes.push_back((bytecode_t)assign_type);
	codes.push_back(scope.vars[var_name].id);


	if (require_semicolon && lexer.curr().type != TokenType::SEMICOLON)
		throw NIGHT_CREATE_FATAL("found '" + lexer.curr().str + "' expected variable type or assignment");

	type_check::var_undefined(lexer, scope, var_name);
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

expr_p parse_toks_expr(Lexer& lexer, Scope& scope, std::string const& err_msg, bool bracket)
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
			auto val = parse_toks_expr(lexer, scope, err_msg, true);
			val->set_guard();

			parse_expr_single(head, val);

			break;
		}
		case TokenType::CLOSE_BRACKET:
		{
			if (bracket)
				throw NIGHT_CREATE_FATAL("missing opening bracket");
		}
		default:
		{
			if (!err_msg.empty() && !head)
				throw NIGHT_CREATE_FATAL(err_msg);

			return head;
		}
		}
	}
}

void generate_codes_expr(bytecodes_t& codes, expr_p const& expr)
{
	assert(expr && "nullptr 'expr' should be handled by the caller");

	auto expr_codes = expr->to_bytecode();
	codes.insert(std::end(codes), std::begin(expr_codes), std::end(expr_codes));

	switch (expr->type)
	{
	case ExprType::VALUE:
		break;
	case ExprType::UNARY:
		generate_codes_expr(codes, expr->lhs);
		break;
	case ExprType::BINARY:
		generate_codes_expr(codes, expr->rhs);
		generate_codes_expr(codes, expr->lhs);
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