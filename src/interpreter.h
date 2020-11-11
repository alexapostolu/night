#pragma once

#include "error.h"
#include "token.h"

#include <vector>

struct NightVariable
{
    std::string name;
    Value value;
};

struct NightFunction
{
    std::string name;
    std::vector<std::string> parameters;
    Scope body;
};

// evaluates an expression
Value EvaluateExpression(Expression* node)
{
    assert(node == nullptr, "node should not be NULL");

    if (node->left == nullptr && node->right == nullptr)
        return node->value;

    if (node->value.value == "+")
    {
        Value val1 = EvaluateExpression(node->left);
        Value val2 = EvaluateExpression(node->right);

        
    }

    assert(true, "operator missing");
}

// finds variable or function in array and returns it's address
template <typename T>
T* GetContainer(std::vector<T>& container, const std::string& token)
{
    for (T& data : container)
    {
        if (token == data.name)
            return &data;
    }

    return nullptr;
}

// [[ deprecated ]]
void UpdateVariables(std::vector<NightVariable>& variables, const std::vector<NightVariable>& scopeVariables)
{
    assert(scopeVariables.size() > variables.size(), "scopeVariables size should be bigger or equal to variables size");
    for (std::size_t a = 0; a < variables.size(); ++a)
    {
        if (variables[a].value != scopeVariables[a].value)
            variables[a].value = scopeVariables[a].value;
    }
}

// interprets statements
void Interpreter(const std::vector<Statement>& statements)//, std::vector<NightVariable>& variables)
{
    static std::vector<NightFunction> functions;
    static std::vector<NightVariable> variables;

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
                EvaluateExpression(std::get<Variable>(statement.stmt).value)
            });

            break;
        }
        case StatementType::ASSIGNMENT: {
            NightVariable* variable = GetContainer(variables, std::get<Assignment>(statement.stmt).name);
            if (variable == nullptr)
                throw Error("variable '" + std::get<Assignment>(statement.stmt).name + "' is not defined");

            variable->value = EvaluateExpression(std::get<Assignment>(statement.stmt).value);

            break;
        }
        case StatementType::CONDITIONAL: {
            if (EvaluateExpression(std::get<Conditional>(statement.stmt).condition).value == "true")
            {
                // std::vector<NightVariable> scopeVariables = variables;
                Interpreter(std::get<Conditional>(statement.stmt).body.statements); // , scopeVariables);

                // UpdateVariables(variables, scopeVariables);

                break;
            }

            for (const Conditional& conditional : std::get<Conditional>(statement.stmt).chains)
            {
                if (conditional.condition == nullptr || EvaluateExpression(conditional.condition).value == "true")
                {
                    Interpreter(conditional.body.statements);
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
            NightFunction* function = GetContainer(functions, std::get<FunctionCall>(statement.stmt).name);
            if (function == nullptr)
                throw Error("function '" + std::get<FunctionCall>(statement.stmt).name + "' is undefined");

            Interpreter(function->body.statements);

            break;
        }
        case StatementType::WHILE_LOOP: {
            while (EvaluateExpression(std::get<WhileLoop>(statement.stmt).condition).value == "true")
                Interpreter(std::get<WhileLoop>(statement.stmt).body.statements);

            break;
        }
        case StatementType::FOR_LOOP: {
            

            break;
        }
        case StatementType::ELEMENT: {
            NightVariable* variable = GetContainer(variables, std::get<Element>(statement.stmt).name);
            if (variable == nullptr)
                throw Error("variable '" + std::get<Element>(statement.stmt).name + "' is not defined");

            if (variable->value.type != ValueType::STRING && variable->value.type != ValueType::BOOL_ARR &&
                variable->value.type != ValueType::NUM_ARR && variable->value.type != ValueType::STRING_ARR)
                throw Error("variable '" + std::get<Element>(statement.stmt).name + "' must contain a string or an array");

            int index = std::stoi(EvaluateExpression(std::get<Element>(statement.stmt).index).value);
            if (index >= variable->value.values.size())
                throw Error("index '" + std::to_string(index) + "' is out of bounds for array of size '" +
                    std::to_string(variable->value.values.size()) + "'");

            variable->value.values[index] = EvaluateExpression(std::get<Element>(statement.stmt).assign);

            break;
        }
        default:
            assert(true, "statement missing");
        }
    }

    variables.erase(variables.begin() + variablesSize + 1, variables.end());
}