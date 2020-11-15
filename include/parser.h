#pragma once

#include "token.h"
#include "utils.h"

#include <vector>
#include <string>

// Expression "constructor"
Expression* new_expression(const std::string& file, int line, const Value& value, Expression* left, Expression* right);

// turns an array of tokens to an array of extras
std::vector<Value> TokensToValues(const std::vector<Token>& tokens, std::vector<Variable>& variables, std::vector<FunctionDef>& functions);

// checks for function definitions in a scope; if found, throw error
void CheckFunctionDefinition(const std::vector<Statement>& statements, const std::string& file, int line, const std::string& errorMsg);

VariableType TypeCheckExpression(const std::string& file, int line, const Expression* node,
	std::vector<Variable>& variables, std::vector<FunctionDef>& functions);

// returns the operator precedence
int GetOperatorPrecedence(const ValueType& type, const std::string& value);

// returns values within a set of brackets
std::vector<Value> GetBracketExpression(const std::string& file, int line, const std::vector<Value>& values, std::size_t& index);

// get next group of values
Expression* GetNextGroup(const std::string& file, int line, const std::vector<Value>& values, std::size_t& index);

// turns an array of values into an AST
Expression* ParseValues(const std::string& file, int line, const std::vector<Value>& values,
	std::vector<Variable>& variables, std::vector<FunctionDef>& functions);

// parses an array of tokens into an array of statements
void Parser(std::vector<Statement>& statements, const std::vector<Token>& tokens, bool inFunction = false);