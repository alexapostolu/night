#pragma once

#include "night.h"
#include "utils.h"
#include "error.h"
#include "token.h"

#include <iostream>
#include <string>
#include <vector>

// "mAcRoS aRe BaD, dOn'T uSe ThEm, reeeeeeee" - haha macro go brrrrrrrrrrrr
#define EVAL_NUM_EXPR(OP) Expression expr1 = EvaluateExpression(node->left, variables, functions);     \
                          Expression expr2 = EvaluateExpression(node->right, variables, functions);    \
                                                                                                       \
                          expr1.data = std::to_string(std::stoi(expr1.data) OP std::stoi(expr2.data)); \
                          return expr1;

// interprets expressions
void Interpreter(const std::vector<Statement>& statements, Expression* returnValue = nullptr);

// evaluates an expression
Expression EvaluateExpression(const Expression* node, std::vector<NightVariable>& variables, std::vector<NightFunction>& functions)
{
    assert(node != nullptr && "node should not be NULL");

    // if left and right node are NULL, then node must be a value
    if (node->left == nullptr && node->right == nullptr)
    {
        if (node->type == ValueType::VARIABLE)
        {
            const NightVariable* variableValue = GetContainer(variables, node->data);
            assert(variableValue != nullptr && "variableValue is not defined");

            return variableValue->value;
        }
        if (node->type == ValueType::CALL)
        {
            if (node->data == "input")
            {   
                std::string uinput;
                getline(std::cin, uinput);
                
                // type check this!!!
                return Expression{ ValueType::STRING, uinput };
            }

            const NightFunction* function = GetContainer(functions, node->data);
            assert(function != nullptr && "function is not defined");

            // create variables from parameters
            assert(function->parameters.size() == node->extras.size() && "function parameters and function call don't match");
            for (std::size_t a = 0; a < function->parameters.size(); ++a)
            {
                variables.push_back(NightVariable{
                    function->parameters[a],
                    EvaluateExpression(&node->extras[a], variables, functions)
                });
            }

            // interpret function body and extract return value
            Expression* returnValue = nullptr;
            Interpreter(function->body, returnValue);

            assert(returnValue != nullptr && "function used in expression doesn't have a return statement");

            // remove variables since they are now out of scope
            variables.erase(
                variables.begin() + variables.size() - function->parameters.size(),
                variables.end()
            );

            return *returnValue;
        }

        return *node;
    }

    if (node->data == "+")
    {
        Expression value1 = EvaluateExpression(node->left, variables, functions);
        Expression value2 = EvaluateExpression(node->right, variables, functions);

        value1.data = value1.type == ValueType::STRING
            ? value1.data + value2.data
            : std::to_string(std::stoi(value1.data) + std::stoi(value2.data));

        return value1;
    }
    if (node->data == "-")
    {
        EVAL_NUM_EXPR(-);
    }
    if (node->data == "*")
    {
        EVAL_NUM_EXPR(*);
    }
    if (node->data == "/")
    {
        EVAL_NUM_EXPR(/);
    }
    if (node->data == "%")
    {
        Expression value1 = EvaluateExpression(node->left, variables, functions);
        const Expression value2 = EvaluateExpression(node->right, variables, functions);

        value1.data = std::to_string(std::stoi(value1.data) % std::stoi(value2.data));
        return value1;
    }
    if (node->data == ">")
    {
        EVAL_NUM_EXPR(>);
    }
    if (node->data == "<")
    {
        EVAL_NUM_EXPR(<);
    }
    if (node->data == ">=")
    {
        EVAL_NUM_EXPR(>=);
    }
    if (node->data == "<=")
    {
        EVAL_NUM_EXPR(<=);
    }
    if (node->data == "!")
    {
        Expression expression = EvaluateExpression(node->left, variables, functions);
        expression.data = expression.data == "true" ? "false" : "true";

        return expression;
    }
    if (node->data == "||")
    {
        Expression expr1 = EvaluateExpression(node->left, variables, functions);
        Expression expr2 = EvaluateExpression(node->right, variables, functions);

        expr1.data = expr1.data == "true" || expr2.data == "true" ? "true" : "false";
        return expr1;
    }
    if (node->data == "&&")
    {
        Expression expr1 = EvaluateExpression(node->left, variables, functions);
        Expression expr2 = EvaluateExpression(node->right, variables, functions);

        expr1.data = expr1.data == "true" && expr2.data == "true" ? "true" : "false";
        return expr1;
    }
    if (node->data == "[]")
    {
        Expression array = EvaluateExpression(node->left, variables, functions);
        return array.extras[std::stoi(EvaluateExpression(&node->extras[0], variables, functions).data)];
    }
    if (node->data == "..")
    {
        Expression start = EvaluateExpression(node->right, variables, functions);
        Expression end = EvaluateExpression(node->left, variables, functions);

        Expression array{ ValueType::NUM_ARR };
        for (int a = std::stoi(start.data); a <= std::stoi(end.data); ++a)
            array.extras.push_back(Expression{ ValueType::NUM, std::to_string(a) });

        return array;
    }
    if (node->data == "<-")
    {
        Expression array = EvaluateExpression(node->left, variables, functions);
        Expression index = EvaluateExpression(node->right, variables, functions);
        
        if (index.extras.empty())
        {
            array.extras.push_back(index);
            return array;
        }
        else
        {
            array.extras.insert(
                array.extras.begin() + std::stoi(index.data),
                EvaluateExpression(&index.extras[0], variables, functions)
            );

            return array;
        }
    }

    assert_rtn(false && "operator missing", Expression());
}

void Interpreter(const std::vector<Statement>& statements, Expression* returnValue)
{
    static std::vector<NightVariable> variables;
    static std::vector<NightFunction> functions;

    const std::size_t variablesSize = variables.size();

    for (const Statement& statement : statements)
    {
        switch (statement.type)
        {
        case StatementType::VARIABLE: {
            if (GetContainer(variables, std::get<Variable>(statement.stmt).name) != nullptr)
                throw Error("variable '" + std::get<Variable>(statement.stmt).name + "' is already defined");
            if (GetContainer(functions, std::get<Variable>(statement.stmt).name) != nullptr)
                throw Error("variable '" + std::get<Variable>(statement.stmt).name + "' cannot have the same name as a function");

            variables.push_back(NightVariable{
                std::get<Variable>(statement.stmt).name,
                EvaluateExpression(std::get<Variable>(statement.stmt).value, variables, functions)
            });

            break;
        }
        case StatementType::ASSIGNMENT: {
            NightVariable* variable = GetContainer(variables, std::get<Assignment>(statement.stmt).name);
            if (variable == nullptr)
                throw Error("variable '" + std::get<Assignment>(statement.stmt).name + "' is not defined");

            variable->value = EvaluateExpression(std::get<Assignment>(statement.stmt).value, variables, functions);

            break;
        }
        case StatementType::CONDITIONAL: {
            if (EvaluateExpression(std::get<Conditional>(statement.stmt).condition, variables, functions).data == "true")
            {
                Interpreter(std::get<Conditional>(statement.stmt).body);
                break;
            }

            for (const Conditional& conditional : std::get<Conditional>(statement.stmt).chains)
            {
                if (conditional.condition == nullptr || EvaluateExpression(conditional.condition, variables, functions).data == "true")
                {
                    Interpreter(conditional.body);
                    break;
                }
            }

            break;
        }
        case StatementType::FUNCTION_DEF: {
            if (GetContainer(variables, std::get<FunctionDef>(statement.stmt).name) != nullptr)
                throw Error("function '" + std::get<FunctionDef>(statement.stmt).name + "' cannot have the same name as a variable");
            if (GetContainer(functions, std::get<FunctionDef>(statement.stmt).name) != nullptr)
                throw Error("function '" + std::get<FunctionDef>(statement.stmt).name + "' is already defined");

            functions.push_back(NightFunction{
                std::get<FunctionDef>(statement.stmt).name,
                std::get<FunctionDef>(statement.stmt).parameters,
                std::get<FunctionDef>(statement.stmt).body
            });
            
            break;
        }
        case StatementType::FUNCTION_CALL: {
            if (std::get<FunctionCall>(statement.stmt).name == "print")
            {
                for (const Expression* parameter : std::get<FunctionCall>(statement.stmt).parameters)
                {
                    Expression expression = EvaluateExpression(parameter, variables, functions);
                    NightPrint(expression);
                }

                break;
            }

            NightFunction* function = GetContainer(functions, std::get<FunctionCall>(statement.stmt).name);
            if (function == nullptr)
                throw Error("function '" + std::get<FunctionCall>(statement.stmt).name + "' is undefined");

            Interpreter(function->body);

            break;
        }
        case StatementType::RETURN: {
            assert(returnValue != nullptr && "returnValue should not be NULL");

            *returnValue = EvaluateExpression(std::get<Return>(statement.stmt).expression, variables, functions);

            variables.erase(variables.begin() + variablesSize, variables.end());
            return;
        }
        case StatementType::WHILE_LOOP: {
            while (EvaluateExpression(std::get<WhileLoop>(statement.stmt).condition, variables, functions).data == "true")
                Interpreter(std::get<WhileLoop>(statement.stmt).body);

            break;
        }
        case StatementType::FOR_LOOP: {
            Expression range = EvaluateExpression(std::get<ForLoop>(statement.stmt).range, variables, functions);
            assert(!range.extras.empty() && "range should not be empty");

            variables.push_back(NightVariable{ std::get<ForLoop>(statement.stmt).index });
            NightVariable* index = &variables.back();

            for (Expression rangeValue : range.extras)
            {
                index->value = rangeValue;
                Interpreter(std::get<ForLoop>(statement.stmt).body);
            }

            break;
        }
        case StatementType::ELEMENT: {
            NightVariable* variable = GetContainer(variables, std::get<Element>(statement.stmt).name);
            if (variable == nullptr)
                throw Error("variable '" + std::get<Element>(statement.stmt).name + "' is not defined");

            if (variable->value.type != ValueType::STRING && variable->value.type != ValueType::BOOL_ARR &&
                variable->value.type != ValueType::NUM_ARR && variable->value.type != ValueType::STRING_ARR)
                throw Error("variable '" + std::get<Element>(statement.stmt).name + "' must contain a string or an array");

            std::size_t index = std::stoi(EvaluateExpression(std::get<Element>(statement.stmt).index, variables, functions).data);
            if (index >= variable->value.extras.size())
                throw Error("index '" + std::to_string(index) + "' is out of bounds for array of size '" +
                    std::to_string(variable->value.extras.size()) + "'");

            variable->value.extras[index] = EvaluateExpression(std::get<Element>(statement.stmt).assign, variables, functions);

            break;
        }
        default: {
            assert(false && "statement missing");
        }
        }
    }

    variables.erase(variables.begin() + variablesSize, variables.end());
}