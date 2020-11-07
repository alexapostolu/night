#pragma once

#include "error.h"
#include "token.h"

#include <vector>

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

    assert(true, "missing operator");
}

template <typename T>
T* GetContainer(std::vector<T>& container, const std::string& token)
{
    for (Variable& data : container)
    {
        if (token == data.name)
            return data;
    }

    return nullptr;
}

struct NightVariable
{
    VariableType type;
    std::string name;
    Value value;
};

void Interpreter(std::vector<Statement>& statements)
{
    static std::vector<NightVariable> variables;

    for (std::size_t a = 0; a < statements.size(); ++a)
    {
        switch (statements[a].type)
        {
        case StatementType::VARIABLE:
            break;
        case StatementType::CONDITIONAL:
            break;
        }
    }
}

/*

Token EvaluateExpression(Expression* node)
{
    if (node->left == nullptr || node->right == nullptr)
        return;

    EvaluateExpression(node->left);
    EvaluateExpression(node->right);

    if (node->left->token.type == "val" && node->right->token.type == "val")
    {
        if (node->token.value == "+")
        {
            node->token = Token{ "val", std::to_string(std::stoi(node->left->token.value) + std::stoi(node->right->token.value)) };
        }
        else if (node->token.value == "*")
        {
            node->token = Token{ "val", std::to_string(std::stoi(node->left->token.value) * std::stoi(node->right->token.value)) };
        }

        delete node->left;
        delete node->right;
    }
}

*/