#pragma once

#include "lib/string.h"
#include "lib/array.h"
#include "lib/error.h"

#include "containers/token.h"

void Expression(const night::array<Token>& expr);

bool is_element(const night::array<Token>& tokens, int index);
bool is_call(const night::array<Token>& tokens, int index);
bool is_value(const night::array<Token>& tokens, int index);
bool is_unary(const night::array<Token>& tokens, int index);
bool is_binary(const night::array<Token>& tokens, int index);
bool is_open_bracket(const night::array<Token>& tokens, int index);
bool is_close_bracket(const night::array<Token>& tokens, int index);

void check_unary(const night::array<Token>& tokens, int index);
void check_binary(const night::array<Token>& tokens, int index);
void check_open_bracket(const night::array<Token>& tokens, int index);
void check_close_bracket(const night::array<Token>& tokens, int index);


bool is_element(const night::array<Token>& tokens, int index)
{
	if (index > tokens.length() - 2 || tokens[index + 1].type != TokenType::OPEN_SQUARE)
		return false;

	night::array<Token> expression;
	for (int a = index + 2; a < tokens.length(); ++a)
	{
		if (expression[a].type == TokenType::CLOSE_SQUARE)
		{
			Expression(tokens.access(index + 2, a - 1));
			return true;
		}
	}

	throw Error(night::_invalid_expression_, tokens, tokens.length() - 1, tokens.length() - 1, "missing closing square bracket");
}

bool is_call(const night::array<Token>& tokens, int index)
{
	if (index > tokens.length() - 2)
		return false;

	if (tokens[index + 1].type != TokenType::OPEN_BRACKET)
		return false;

	night::array<Token> parameters;
	for (int a = index + 2; a < tokens.length(); ++a)
	{
		if (parameters[a].type == TokenType::CLOSE_BRACKET)
		{
			parameters = tokens.access(index + 2, a - 1);
			//check_parameters(parameters);

			return true;
		}
	}

	throw Error(night::_invalid_expression_, tokens, tokens.length() - 1, tokens.length() - 1, "missing closing bracket");
}

bool is_value(const night::array<Token>& tokens, int index)
{
	if (tokens[index].type >= TokenType::BIT_VALUE && tokens[index].type <= TokenType::STR_VALUE)
		return true;

	if (tokens[index].type != TokenType::VARIABLE)
		return false;

	if (is_element(tokens, index))
		return true;

	if (is_call(tokens, index))
		return true;

	return false;
}

bool is_unary(const night::array<Token>& tokens, int index)
{
	if (tokens[index].type == TokenType::NOT)
		return false;

	return true;
}

bool is_binary(const night::array<Token>& tokens, int index)
{
	if (tokens[index].type >= TokenType::PLUS && tokens[index].type <= TokenType::MOD)
		return true;

	if (tokens[index].type >= TokenType::EQUALS && tokens[index].type <= TokenType::NOT_EQUALS)
		return true;

	if (tokens[index].type >= TokenType::OR && tokens[index].type <= TokenType::AND)
		return true;

	return false;
}

bool is_open_bracket(const night::array<Token>& tokens, int index)
{
	return tokens[index].type == TokenType::OPEN_BRACKET;
}

bool is_close_bracket(const night::array<Token>& tokens, int index)
{
	return tokens[index].type == TokenType::CLOSE_BRACKET;
}


void check_unary(const night::array<Token>& tokens, int index)
{
	if (!is_value(tokens, index + 1) && is_open_bracket(tokens, index + 1))
		throw Error(night::_invalid_expression_, tokens, index, index + 1, "expected a value or a close bracket");
}

void check_binary(const night::array<Token>& tokens, int index)
{
	if (!is_value(tokens, index + 1) && !is_unary(tokens, index + 1) && !is_open_bracket(tokens, index + 1))
		throw Error(night::_invalid_expression_, tokens, index, index + 1, "expected a value, a unary operator, or an open bracket");
}

void check_open_bracket(const night::array<Token>& tokens, int index)
{
	if (!is_value(tokens, index + 1) && !is_unary(tokens, index + 1) && !is_open_bracket(tokens, index + 1))
		throw Error(night::_invalid_expression_, tokens, index, index + 1, "expected a value, a unary operator, or an open bracket");
}

void check_close_bracket(const night::array<Token>& tokens, int index)
{
	if (!is_binary(tokens, index + 1) && !is_close_bracket(tokens, index + 1))
		throw Error(night::_invalid_expression_, tokens, index, index + 1, "expected a binary operator or a close bracket");
}

void check_value(const night::array<Token >& tokens, int index)
{
	if (!is_binary(tokens, index + 1) && !is_close_bracket(tokens, index + 1))
		throw Error(night::_invalid_expression_, tokens, index, index + 1, "expected an binary operator or a close bracket");
}


Token eat(const night::array<Token>& code, int& index)
{	
	if (++index == code.length())
		throw Error(night::_invalid_grammar_, code, index - 1, index - 1, "unexpected end of line");

	return code[index];
}

Token peek(const night::array<Token>& code, int index)
{
	int set = index;
	return eat(code, set);
}

void Parser(const night::array<Token>& code)
{
	return;

	int index = -1;

	Token token = eat(code, index);
	if (token.type >= TokenType::BIT_TYPE && token.type <= TokenType::NULL_TYPE)
	{
		token = eat(code, index);
		if (token.type == TokenType::VARIABLE)
		{
			token = eat(code, index);
			if (token.type == TokenType::ASSIGNMENT)
			{
				if (code[0].type == TokenType::NULL_TYPE)
					throw Error(night::_invalid_variable_, code, 0, 0, "variable cannot be of type 'null'; only functions can be of type 'null'");

				if (code.back().type != TokenType::SEMICOLON)
					throw Error(night::_invalid_variable_, code, code.length() - 1, code.length() - 1, "expected a semicolon");

				try {
					Expression(code.access(3, code.length() - 2));
				}
				catch (const Error& e) {
					throw Error(e.errorType, code, e.start + 3, e.end + 3, e.errorMsg);
				}
			}
			else if (token.type == TokenType::OPEN_BRACKET)
			{
				token = eat(code, index);
				int a;
				for (a = 3; token.type != TokenType::CLOSE_BRACKET; a += 3)
				{
					if (token.type < TokenType::BIT_TYPE || token.type > TokenType::STR_TYPE)
						throw Error(night::_invalid_function_, code, a, a, "expected variable type");

					token = eat(code, index);
					if (token.type != TokenType::VARIABLE)
						throw Error(night::_invalid_function_, code, a + 1, a + 1, "expected variable name");

					token = eat(code, index);
					if (token.type != TokenType::CLOSE_BRACKET && token.type != TokenType::COMMA)
						throw Error(night::_invalid_function_, code, a + 1, a + 2, "expected closing bracket or comma");

					if (token.type == TokenType::COMMA)
						token = eat(code, index);
				}

				token = eat(code, index);
				if (token.type != TokenType::OPEN_CURLY)
					throw Error(night::_invalid_function_, code, a + 1, a + 1, "expected open curly bracket");

				if (code.back().type != TokenType::CLOSE_CURLY)
					throw Error(night::_invalid_function_, code, code.length() - 1, code.length() - 1, "expected closing curly bracket");

				Parser(code.access(a + 2, code.length() - 2));
			}
			else if (token.type == TokenType::SEMICOLON)
			{
				if (code[0].type == TokenType::NULL_TYPE)
					throw Error(night::_invalid_variable_, code, 0, 0, "variable cannot be of type 'null'; only functions can be of type 'null'");
			}
			else
			{
				throw Error(night::_invalid_variable_, code, 2, 2, "expected a semicolon");
			}
		}
	}
	else if (token.type == TokenType::IMPORT)
		for (int i = 0; i < 2; i++)
			eat(code, index);
	else {
		// cricket noises
	}
}

void Expression(const night::array<Token>& expr)
{
	if (!is_value(expr, 0) && !is_unary(expr, 0) && !is_open_bracket(expr, 0))
		throw Error(night::_invalid_expression_, expr, 0, 0, "expected a value, unary operator, or a open bracket");
	if (!is_value(expr, expr.length() - 1) && !is_close_bracket(expr, expr.length() - 1))
		throw Error(night::_invalid_expression_, expr, expr.length() - 1, expr.length() - 1, "expected a value, or a close bracket");

	for (int a = 0; a < expr.length() - 1; ++a)
	{
		if (is_unary(expr, a))
			check_unary(expr, a);
		else if (is_binary(expr, a))
			check_binary(expr, a);
		else if (is_open_bracket(expr, a))
			check_open_bracket(expr, a);
		else if (is_close_bracket(expr, a))
			check_close_bracket(expr, a);
		else if (is_value(expr, a))
			check_value(expr, a);
		else
			throw Error(night::_invalid_expression_, expr, a, a, "expressions can only contain values, operators, and brackets");
	}
}