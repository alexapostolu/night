#pragma once

#include "token.h"
#include "error.h"

#include <memory>
#include <string>
#include <vector>

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

// const overload
template <typename T>
const T* GetContainer(const std::vector<T>& container, const std::string& token)
{
    for (const T& data : container)
    {
        if (token == data.name)
            return &data;
    }

    return nullptr;
}

// index starts right after opening bracket; advances index to closing bracket
template <typename Unit, typename UnitType>
void AdvanceCloseBracketIndex(const std::string& file, const int line, const std::vector<Unit>& units,
    const UnitType& openBracket, const UnitType& closeBracket, std::size_t& index)
{
    for (int openBracketCount = 0; index < units.size(); ++index)
    {
        if (units[index].type == openBracket)
        {
            openBracketCount++;
        }
        else if (units[index].type == closeBracket)
        {
            if (openBracketCount == 0)
                return;

            openBracketCount--;
        }
    }
}

// splits an array of tokens into different statements
std::vector<std::vector<Token> > SplitCode(
    const std::vector<Token>& tokens
);

// returns default value for a given type
std::string DefaultValue(
    const ValueType& type
);

// converts between enum types
std::string  VarTypeToStr       (const VariableType& type);
VariableType ValueTypeToVarType (const ValueType&    type);
ValueType    VarTypeToValueType (const VariableType& type);
VariableType VarTypeToArrType   (const VariableType& type);

// extracts expression from tokens; returns a type checked expression
std::shared_ptr<Expression> ExtractExpression(
    const std::vector<Token>& tokens,

    const std::size_t start,
    const std::size_t end,

    const std::vector<Variable>&    variables,
    const std::vector<FunctionDef>& functions,

    VariableType* type = nullptr
);

// extracts condition from tokens; returns a type checked expression
std::shared_ptr<Expression> ExtractCondition(
    const std::vector<Token>& tokens,
    std::size_t& closeBracketIndex,

    const std::vector<Variable>&    variables,
    const std::vector<FunctionDef>& functions,

    const std::string& stmt
);

// extracts body from tokens; returns a parsed statement vector
std::vector<Statement> ExtractBody(
    const std::vector<Token>& tokens,
    std::size_t closeBracketIndex,

    std::vector<Variable>& variables, // can't be const since variables need to be removed after scope finished

    const std::string& stmt,

    bool inFunction = false
);