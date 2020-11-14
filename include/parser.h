#pragma once

#include "utils.h"
#include "token.h"
#include "error.h"

#include <iostream>
#include <vector>
#include <string>

std::vector<Value> TokensToValues(const std::vector<Token>& tokens, VariableType* exprType = nullptr);

// separates tokens based on commas
std::vector<std::vector<Value> > SeparateTokens(const std::vector<Token>& tokens, std::size_t& index)
{
	std::vector<std::vector<Value> > values;
	for (int start = index, openBracketCount = 0, openSquareCount = 0; index < tokens.size(); ++index)
	{
		if (tokens[index].type == TokenType::OPEN_BRACKET)
			openBracketCount++;
		else if (tokens[index].type == TokenType::CLOSE_BRACKET)
			openBracketCount--;
		else if (tokens[index].type == TokenType::OPEN_SQUARE)
			openSquareCount++;
		else if (tokens[index].type == TokenType::CLOSE_SQUARE)
			openSquareCount--;

		if (openSquareCount == 0 && ((tokens[index].type == TokenType::COMMA && openBracketCount == 0) ||
			(tokens[index].type == TokenType::CLOSE_BRACKET && openBracketCount == -1)))
		{
			values.push_back(TokensToValues(
				std::vector<Token>(tokens.begin() + start, tokens.begin() + index)
			));

			start = index + 1;

			if (tokens[index].type == TokenType::CLOSE_BRACKET)
				break;

			continue;
		}
	}

	return values;
}

// type checks an array of values
ValueType CheckValuesType(const std::vector<Value>& values) // int& arrayDepth
{
	assert(!values.empty() && "values shouldn't be empty");

	// check types when constructing the AST
	// and for 2d arrays, check in TokenToValues()

	return ValueType::NUM_ARR;
}

// turns an array of tokens to an array of extras
std::vector<Value> TokensToValues(const std::vector<Token>& tokens, VariableType* exprType)
{
	assert(!tokens.empty() && "tokens shouldn't be empty");

	const std::string file = tokens[0].file;
	const int line = tokens[0].line;

	// convert tokens to values

	std::vector<Value> values;
	for (std::size_t a = 0; a < tokens.size(); ++a)
	{
		if (tokens[a].type == TokenType::VARIABLE)
		{
			if (a < tokens.size() - 1 && tokens[a + 1].type == TokenType::OPEN_BRACKET)
			{
				Value functionCall{ ValueType::CALL };

				a += 2;
				functionCall.extras = SeparateTokens(tokens, a);

				if (a >= tokens.size())
					throw Error(tokens[a].file, tokens[a].line, "missing closing bracket for function call");

				values.push_back(functionCall);
			}
			else if (a < tokens.size() - 1 && tokens[a + 1].type == TokenType::OPEN_SQUARE)
			{
				values.push_back(Value{ ValueType::VARIABLE, tokens[a].value });

				a += 2;
				std::size_t start = a;

				AdvanceCloseBracketIndex(file, line, tokens, TokenType::OPEN_SQUARE, TokenType::CLOSE_SQUARE, a);

				if (a >= tokens.size())
					throw Error(file, line, "missing closing square bracket for subscript operator");

				Value subscript{ ValueType::OPERATOR, "[]" };
				subscript.extras.push_back(TokensToValues(
					std::vector<Token>(tokens.begin() + start, tokens.begin() + a)
				));

				values.push_back(subscript);
			}
			else
			{
				values.push_back(Value{ ValueType::VARIABLE, tokens[a].value });
			}
		}
		else if (tokens[a].type == TokenType::OPEN_SQUARE || (a < tokens.size() - 1 &&
			tokens[a].type == TokenType::NUM_VAL && tokens[a + 1].type == TokenType::OPEN_SQUARE))
		{
			Value array;

			if (tokens[a].type == TokenType::OPEN_SQUARE)
				a++;
			else
				a += 2;

			for (int start = a, openBracketCount = 0, openSquareCount = 0; a < tokens.size(); ++a)
			{
				if (tokens[a].type == TokenType::OPEN_BRACKET)
					openBracketCount++;
				else if (tokens[a].type == TokenType::CLOSE_BRACKET)
					openBracketCount--;
				else if (tokens[a].type == TokenType::OPEN_SQUARE)
					openSquareCount++;
				else if (tokens[a].type == TokenType::CLOSE_SQUARE)
					openSquareCount--;

				if (openBracketCount == 0 && ((tokens[a].type == TokenType::COMMA && openBracketCount == 0) ||
					(tokens[a].type == TokenType::CLOSE_SQUARE && openSquareCount == -1)))
				{
					array.extras.push_back(TokensToValues(
						std::vector<Token>(tokens.begin() + start, tokens.begin() + a)
					));

					// type check array elements
					if (array.extras.size() == 1) {
						array.type = CheckValuesType(array.extras[0]);
					}
					else if (array.type != CheckValuesType(array.extras.back())) {
						throw Error(file, line, "elements in an array must be of the same type");
					}

					start = a + 1;

					if (tokens[a].type == TokenType::CLOSE_SQUARE)
						break;

					continue;
				}
			}

			if (a >= tokens.size())
				throw Error(file, line, "missing closing square for array");

			if (tokens[a].type == TokenType::NUM_VAL)
			{
				std::size_t arraySize = std::stol(tokens[a].value);
				if (array.extras.size() > arraySize)
					throw Error(file, line, "array of size '" + std::to_string(arraySize) + "' cannot hold '" + std::to_string(array.extras.size()) + "' elements");

				for (std::size_t b = array.extras.size(); b <= arraySize; ++b)
				{
					// default values for 2D arrays?
					array.extras.push_back(std::vector<Value>{
						Value{ array.type, DefaultValue(array.type) }
					});
				}
			}

			values.push_back(array);
		}
		else
		{
			switch (tokens[a].type)
			{
			case TokenType::BOOL_VAL:
				values.push_back(Value{ ValueType::BOOL, tokens[a].value });
				break;
			case TokenType::NUM_VAL:
				values.push_back(Value{ ValueType::NUM, tokens[a].value });
				break;
			case TokenType::STRING_VAL:
				values.push_back(Value{ ValueType::STRING, tokens[a].value });
				break;
			case TokenType::OPERATOR:
				values.push_back(Value{ ValueType::OPERATOR, tokens[a].value });
				break;
			case TokenType::OPEN_BRACKET:
				values.push_back(Value{ ValueType::OPEN_BRACKET, tokens[a].value });
				break;
			case TokenType::CLOSE_BRACKET:
				values.push_back(Value{ ValueType::CLOSE_BRACKET, tokens[a].value });
				break;
			default:
				throw Error(file, line, "unexpected token '" + tokens[a].value + "' in expression");
			}
		}
	}

	// type check values

	const ValueType valuesType = CheckValuesType(values);
	if (exprType != nullptr)
	{
		switch (valuesType)
		{
		case ValueType::BOOL:
			*exprType = VariableType::BOOL;
			break;
		case ValueType::BOOL_ARR:
			*exprType = VariableType::BOOL_ARR;
			break;
		case ValueType::NUM:
			*exprType = VariableType::NUM;
			break;
		case ValueType::NUM_ARR:
			*exprType = VariableType::NUM_ARR;
			break;
		case ValueType::STRING:
			*exprType = VariableType::STRING;
			break;
		case ValueType::STRING_ARR:
			*exprType = VariableType::STRING_ARR;
			break;
		default:
			assert(false && "missing type");
		}
	}

	return values;
}

// checks for function definitions in a scope; if found, throw error
void CheckFunctionDefinition(const std::vector<Statement>& statements, const std::string& file, int line, const std::string& errorMsg)
{
	for (const Statement& statement : statements)
	{
		if (statement.type == StatementType::FUNCTION_DEF)
			throw Error(file, line, errorMsg);
	}
}

// returns the operator precedence
int GetOperatorPrecedence(const ValueType& type, const std::string& value)
{
	static const std::vector<std::vector<std::string> > operators{
		{ "[]" },
		{ "!" },
		{ "*", "/", "%" },
		{ "+", "-" },
		{ ".." },
		{ ">", "<", ">=", "<=" },
		{ "==", "!=" },
		{ "||", "&&" },
		{ "<-" }
	};

	assert(type == ValueType::OPERATOR && "value must be operator to have operator precedence");

	for (std::size_t a = 0; a < operators.size(); ++a)
	{
		for (const std::string& op : operators[a])
		{
			if (value == op)
				return a;
		}
	}

	assert_rtn(false && "operator missing", 0);
}

// returns values within a set of brackets
std::vector<Value> GetBracketExpression(const std::string& file, int line, const std::vector<Value>& values, std::size_t& index)
{
	std::size_t start = index;
	AdvanceCloseBracketIndex(file, line, values, ValueType::OPEN_BRACKET, ValueType::CLOSE_BRACKET, index);

	if (index >= values.size())
		throw Error(file, line, "missing closing bracket in expression");

	return std::vector<Value>(values.begin() + start, values.begin() + index);
}

Expression* ParseValues(const std::string& file, int line, const std::vector<Value>& values);

// Expression "constructor"
Expression* new_expression(const std::string& file, int line, const Value& value, Expression* left, Expression* right)
{
	Expression* expression = new Expression{
		value.type,
		value.data, 
		std::vector<Expression>(),
		left,
		right
	};

	for (const std::vector<Value>& values : value.extras)
		expression->extras.push_back(*ParseValues(file, line, values));

	return expression;
}

// turns an array of values into an AST
Expression* ParseValues(const std::string& file, int line, const std::vector<Value>& values)
{
	assert(!values.empty() && "values shouldn't be empty");

	std::size_t a = 1;

	// special case of brackets at the start of expression

	Expression* root = nullptr;
	if (values[0].type == ValueType::OPEN_BRACKET)
	{
		std::vector<Value> bracketExpr = GetBracketExpression(file, line, values, a);
		root = ParseValues(file, line, bracketExpr);
		a++;
	}
	else
	{
		root = new_expression(file, line, values[0], nullptr, nullptr);
		a = 0;
	}

	// parse expression

	for (; a < values.size(); ++a)
	{
		if (values[a].type != ValueType::OPERATOR)
			continue;

		// travel tree to find a nice spot to settle down

		Expression* curr = root;
		Expression* prev = nullptr;
		while ((curr->left != nullptr || curr->right != nullptr) &&
			GetOperatorPrecedence(curr->type, curr->data) > GetOperatorPrecedence(values[a].type, values[a].data))
		{
			prev = curr;
			curr = curr->right;
		}

		// create right hand value node

		std::size_t opIndex = a;

		Expression* nextValue = nullptr;
		if (a < values.size() - 1 && values[a + 1].type == ValueType::OPEN_BRACKET)
		{
			a += 2;

			const std::vector<Value> bracketValues = GetBracketExpression(file, line, values, a);
			nextValue = ParseValues(file, line, bracketValues);
		}
		else if (values[a].data != "[]")
		{
			nextValue = new_expression(file, line, values[opIndex + 1], nullptr, nullptr);
		}

		// create operator node

		Expression* opNode = values[a].data == "[]"
			? new_expression(file, line, values[opIndex], nullptr, curr)
			: new_expression(file, line, values[opIndex], curr, nextValue);

		if (curr == root)
		{
			root = opNode;
		}
		else
		{
			assert(prev != nullptr && "prev shouldn't be NULL");
			prev->right = opNode;
		}

		// handle unary operator

		if (opIndex < values.size() - 1 && values[opIndex + 1].type == ValueType::OPERATOR && values[opIndex + 1].data == "!")
		{
			assert(root != nullptr && "root shouldn't be NULL; that should be a specific case");

			curr = root;
			while (curr->left != nullptr && curr->right != nullptr)
				curr = curr->right;


			Expression* nextValue = nullptr;
			if (a < values.size() - 2 && values[opIndex + 2].type == ValueType::OPEN_BRACKET)
			{
				a += 3;

				const std::vector<Value> bracketValues = GetBracketExpression(file, line, values, a);
				nextValue = ParseValues(file, line, bracketValues);
			}
			else
			{
				nextValue = new_expression(file, line, values[opIndex + 2], nullptr, nullptr);
			}

			curr->right = nextValue;
		}

		if (values[a].data != "[]")
			a++;
	}

	return root;
}

// parses an array of tokens into an array of statements
void Parser(std::vector<Statement>& statements, const std::vector<Token>& tokens, bool inFunction = false)
{
	assert(!tokens.empty() && "tokens array should not be empty");

	const std::string file = tokens[0].file;
	const int line = tokens[0].line;

	if (tokens.size() >= 1 && tokens[0].type == TokenType::SET)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::VARIABLE)
			throw Error(file, line, "expected variable name after 'set' keyword");
		if (tokens.size() == 2 || tokens[2].type != TokenType::ASSIGNMENT)
			throw Error(file, line, "expected assignment operator after variable name");
		if (tokens.size() == 3)
			throw Error(file, line, "expected expression after assignment operator");

		VariableType exprType;
		const std::vector<Value> expression = TokensToValues(std::vector<Token>(tokens.begin() + 3, tokens.end()), &exprType);

		Statement statement{ StatementType::VARIABLE };
		statement.stmt = Variable{
			tokens[1].value,
			ParseValues(file, line, expression)
		};

		statements.push_back(statement);
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VARIABLE && tokens[1].type == TokenType::ASSIGNMENT)
	{
		if (tokens.size() == 2)
			throw Error(file, line, "expected expression after assignment operator");

		std::vector<Value> expression = TokensToValues(std::vector<Token>(tokens.begin() + 2, tokens.end()));

		Statement statement{ StatementType::ASSIGNMENT };
		statement.stmt = Assignment{
			tokens[0].value,
			ParseValues(file, line, expression)
		};

		statements.push_back(statement);
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::IF)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::OPEN_BRACKET)
			throw Error(file, line, "expected open bracket after 'if' keyword");
		if (tokens.size() == 2)
			throw Error(file, line, "expected close bracket in if condition");
		if (tokens.size() == 3 && tokens[2].type == TokenType::CLOSE_BRACKET)
			throw Error(file, line, "expected expression in between brackets");

		// if statement condition

		std::size_t closeBracketIndex = 2;
		AdvanceCloseBracketIndex(file, line, tokens, TokenType::OPEN_BRACKET, TokenType::CLOSE_BRACKET, closeBracketIndex);
		
		VariableType exprType;
		std::vector<Value> condition = TokensToValues(
			std::vector<Token>(tokens.begin() + 2, tokens.begin() + closeBracketIndex),
			&exprType
		);

		if (exprType != VariableType::BOOL)
			throw Error(file, line, "if statement condition can't be a " + VarTypeToStr(exprType) + "; it must be a boolean value");

		// if statement body

		std::vector<Statement> scope;
		std::vector<std::vector<Token> > code = SplitCode(std::vector<Token>(tokens.begin() + closeBracketIndex + 2, tokens.end() - 1));
		for (const std::vector<Token>& tokens : code)
			Parser(scope, tokens);

		CheckFunctionDefinition(scope, file, line, "if statements cannot contain function definitions");

		Statement statement{ StatementType::CONDITIONAL };
		statement.stmt = Conditional{
			ParseValues(file, line, condition),
			scope
		};

		statements.push_back(statement);
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::ELSE && tokens[1].type == TokenType::IF)
	{
		if (tokens.size() == 2 || tokens[2].type != TokenType::OPEN_BRACKET)
			throw Error(file, line, "expected open bracket after 'if' keyword");
		if (tokens.size() == 3)
			throw Error(file, line, "expected close bracket in if condition");
		if (tokens.size() == 4 && tokens[3].type == TokenType::CLOSE_BRACKET)
			throw Error(file, line, "expected expression in between brackets");
		if (statements.size() == 0 || statements.back().type != StatementType::CONDITIONAL ||
			std::get<Conditional>(statements.back().stmt).condition == nullptr)
			throw Error(file, line, "expected if or else if statement before else if statement");

		// else if statement condition

		std::size_t closeBracketIndex = 3;
		AdvanceCloseBracketIndex(file, line, tokens, TokenType::OPEN_BRACKET, TokenType::CLOSE_BRACKET, closeBracketIndex);

		VariableType exprType;
		std::vector<Value> condition = TokensToValues(
			std::vector<Token>(tokens.begin() + 3, tokens.begin() + closeBracketIndex),
			&exprType
		);

		if (exprType != VariableType::BOOL)
			throw Error(file, line, "else if statement condition can't be a " + VarTypeToStr(exprType) + "; it must be a boolean value");

		// else if statement body

		std::vector<Statement> scope;
		std::vector<std::vector<Token> > code = SplitCode(std::vector<Token>(tokens.begin() + closeBracketIndex + 2, tokens.end() - 1));
		for (const std::vector<Token>& tokens : code)
			Parser(scope, tokens);

		CheckFunctionDefinition(scope, file, line, "else if statements cannot contain functions");

		std::get<Conditional>(statements.back().stmt).chains.push_back(Conditional{ ParseValues(file, line, condition), scope });
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::ELSE)
	{
		if (tokens.size() == 2 || tokens[2].type != TokenType::OPEN_CURLY)
			throw Error(file, line, "expected open brace after 'else' keyword");
		if (statements.size() == 0 || statements.back().type != StatementType::CONDITIONAL ||
			std::get<Conditional>(statements.back().stmt).condition == nullptr)
			throw Error(file, line, "expected if or else if statement before else statement");

		// else statement body

		std::vector<Statement> scope;
		std::vector<std::vector<Token> > code = SplitCode(std::vector<Token>(tokens.begin() + 2, tokens.end() - 1));
		for (const std::vector<Token>& tokens : code)
			Parser(scope, tokens);

		CheckFunctionDefinition(scope, file, line, "else statements cannot contain functions");
		
		std::get<Conditional>(statements.back().stmt).chains.push_back(Conditional{ nullptr, scope });
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::DEF)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::VARIABLE)
			throw Error(file, line, "expected function name after 'def' keyword");
		if (tokens.size() == 2 || tokens[2].type != TokenType::OPEN_BRACKET)
			throw Error(file, line, "expected open bracket after function name");
		if (tokens.size() == 3)
			throw Error(file, line, "expected open bracket after function name");
		if (tokens.size() == 4 && tokens[3].type != TokenType::CLOSE_BRACKET)
			throw Error(file, line, "expected closing bracket for function parameters");
		if (tokens.size() == 4 || (tokens.size() == 5 || tokens[4].type != TokenType::OPEN_CURLY))
			throw Error(file, line, "expected open curly bracket for function body");

		Statement statement{ StatementType::FUNCTION_DEF };
		std::get<FunctionDef>(statement.stmt).name = tokens[1].value;

		// function parameters

		std::size_t closeBracketIndex = 3;
		for (; closeBracketIndex < tokens.size(); closeBracketIndex += 2)
		{
			if (tokens[closeBracketIndex].type != TokenType::VARIABLE)
				throw Error(file, line, "expected variable names as function parameters");

			std::get<FunctionDef>(statement.stmt).parameters.push_back(tokens[closeBracketIndex].value);

			if (tokens[closeBracketIndex + 1].type == TokenType::CLOSE_BRACKET)
			{
				closeBracketIndex++;
				break;
			}

			if (tokens[closeBracketIndex + 1].type != TokenType::COMMA)
				throw Error(file, line, "expected comma or closing bracket after function parameter");
		}

		// function body

		Parser(
			std::get<FunctionDef>(statement.stmt).body,
			std::vector<Token>(tokens.begin() + closeBracketIndex + 2, tokens.end() - 1),
			true
		);

		CheckFunctionDefinition(std::get<FunctionDef>(statement.stmt).body, file, line, "functions cannot contain functions");

		statements.push_back(statement);
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VARIABLE && tokens[1].type == TokenType::OPEN_BRACKET)
	{
		if (tokens.size() == 2 || (tokens.size() == 3 && tokens[2].type != TokenType::CLOSE_BRACKET))
			throw Error(file, line, "missing closing bracket");

		// function parameters

		Statement statement{ StatementType::FUNCTION_CALL };
		statement.stmt = FunctionCall{ tokens[0].value };

		std::vector<Token> tokenParam;
		for (std::size_t a = 2, openBracketIndex = 0; a < tokens.size(); ++a)
		{
			if (tokens[a].type == TokenType::OPEN_BRACKET)
				openBracketIndex++;
			else if (tokens[a].type == TokenType::CLOSE_BRACKET)
				openBracketIndex--;

			if ((tokens[a].type == TokenType::COMMA && openBracketIndex == 0) ||
				(tokens[a].type == TokenType::CLOSE_BRACKET && openBracketIndex == -1))
			{
				if (tokens[a].type == TokenType::CLOSE_BRACKET && a < tokens.size() - 1)
					throw Error(file, line, "unexpected tokens after function call; each statement must be on it's own line");

				const std::vector<Value> paramExpr = TokensToValues(tokenParam);
				std::get<FunctionCall>(statement.stmt).parameters.push_back(ParseValues(file, line, paramExpr));

				tokenParam.clear();
				continue;
			}

			tokenParam.push_back(tokens[a]);
		}

		statements.push_back(statement);
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::RETURN)
	{
		if (!inFunction)
			throw Error(file, line, "return statement can only be in functions");
		if (tokens.size() == 1)
			throw Error(file, line, "expected expression after 'return' keyword");

		std::vector<Value> extras = TokensToValues(std::vector<Token>(tokens.begin() + 1, tokens.end()));

		Statement statement{ StatementType::RETURN };
		statement.stmt = Return{ ParseValues(file, line, extras) };

		statements.push_back(statement);
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::WHILE)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::OPEN_BRACKET)
			throw Error(file, line, "expected open bracket after 'while' keyword");
		if (tokens.size() == 2)
			throw Error(file, line, "expected condition in while loop");
		if (tokens.size() == 3 && tokens[2].type != TokenType::CLOSE_BRACKET)
			throw Error(file, line, "expected closing bracket in condition");
		if (tokens.size() == 3 && tokens[2].type == TokenType::CLOSE_BRACKET)
			throw Error(file, line, "expected condition in between brackets");

		Statement statement{ StatementType::WHILE_LOOP };

		// while loop condition

		std::size_t closeBracketIndex = 2;
		AdvanceCloseBracketIndex(file, line, tokens, TokenType::OPEN_BRACKET, TokenType::CLOSE_BRACKET, closeBracketIndex);

		if (closeBracketIndex == tokens.size() - 1 || tokens[closeBracketIndex].type != TokenType::OPEN_CURLY)
			throw Error(file, line, "expected open curly bracket for while loop body");

		VariableType conditionType;
		std::vector<Value> condition = TokensToValues(
			std::vector<Token>(tokens.begin() + 2, tokens.begin() + closeBracketIndex),
			&conditionType
		);

		if (conditionType != VariableType::BOOL)
			throw Error(file, line, "while loop condition can't be a " + VarTypeToStr(conditionType) + "; it must be a boolean");

		// while loop body

		std::get<WhileLoop>(statement.stmt).condition = ParseValues(file, line, condition);
		Parser(
			std::get<WhileLoop>(statement.stmt).body,
			std::vector<Token>(tokens.begin() + closeBracketIndex + 1, tokens.end() - 1)
		);

		CheckFunctionDefinition(std::get<WhileLoop>(statement.stmt).body, file, line, "while loops cannot contain functions");

		statements.push_back(statement);
	}
	else if (tokens.size() >= 1 && tokens[0].type == TokenType::FOR)
	{
		if (tokens.size() == 1 || tokens[1].type != TokenType::OPEN_BRACKET)
			throw Error(file, line, "expected open bracket after 'for' keyword");
		if (tokens.size() == 2 || tokens[2].type != TokenType::VARIABLE)
			throw Error(file, line, "expected iterator name after open bracket");
		if (tokens.size() == 3 || tokens[3].type != TokenType::COLON)
			throw Error(file, line, "expected colon after iterator name");
		if (tokens.size() == 4)
			throw Error(file, line, "expected array after colon");

		// iterator and range

		std::size_t closeBracketIndex = 0;
		AdvanceCloseBracketIndex(file, line, tokens, TokenType::OPEN_BRACKET, TokenType::CLOSE_BRACKET, closeBracketIndex);

		VariableType rangeType;
		std::vector<Value> rangeValue = TokensToValues(
			std::vector<Token>(tokens.begin() + 4, tokens.begin() + closeBracketIndex),
			&rangeType
		);

		if (rangeType != VariableType::BOOL_ARR && rangeType != VariableType::NUM_ARR && rangeType != VariableType::STRING_ARR)
			throw Error(file, line, "for loop range cannot be a '" + VarTypeToStr(rangeType) + "'; it can only be an array");

		// scope

		std::vector<Statement> statements;
		Parser(statements, std::vector<Token>(tokens.begin() + closeBracketIndex + 1, tokens.end() - 1));

		Statement statement{ StatementType::FOR_LOOP };
		statement.stmt = ForLoop{
			tokens[1].value,
			ParseValues(file, line, rangeValue),
			statements
		};

		statements.push_back(statement);
	}
	else if (tokens.size() >= 2 && tokens[0].type == TokenType::VARIABLE && tokens[1].type == TokenType::OPEN_SQUARE)
	{
		if (tokens.size() == 2)
			throw Error(file, line, "expected index after open square");

		Statement statement{ StatementType::ELEMENT };

		std::size_t assignmentIndex = 2;
		for (; assignmentIndex < tokens.size() && tokens[assignmentIndex].type != TokenType::ASSIGNMENT; ++assignmentIndex);

		if (tokens[assignmentIndex - 1].type != TokenType::CLOSE_SQUARE)
			throw Error(file, line, "missing closing square");
		if (assignmentIndex == tokens.size())
			throw Error(file, line, "missing assignment operator");

		VariableType indexType;
		std::vector<Value> indexValues = TokensToValues(std::vector<Token>(tokens.begin() + 2, tokens.begin() + assignmentIndex - 1), &indexType);

		if (indexType != VariableType::NUM)
			throw Error(file, line, "array index can't be a '" + VarTypeToStr(indexType) + "'; it must be a number");

		std::vector<Value> assignValue = TokensToValues(std::vector<Token>(tokens.begin() + assignmentIndex + 1, tokens.end()));

		statement.stmt = Element{
			tokens[0].value,
			ParseValues(file, line, indexValues),
			ParseValues(file, line, assignValue)
		};

		statements.push_back(statement);
	}
	else
	{
		throw Error(file, line, "invalid syntax");
	}
}