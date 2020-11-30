#pragma once

#include "token.h"

#include <memory>
#include <string>
#include <vector>

// finds item in array and returns its address
// returns NULL if not found
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

// index starts right after opening bracket
// advances index to closing bracket
template <typename Unit, typename UnitType>
void AdvanceCloseBracketIndex(const std::vector<Unit>& units,
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

namespace night {
   
// converts a value or variable type to a string
std::string ttos(const ValueType& type);
std::string ttos(const VariableType& type);

bool find(const std::vector<VariableType>& types, const VariableType& findType);

} // namespace night

// splits a 1D array of tokens into a 2d array based on individual statements
std::vector<std::vector<Token> > SplitCode(
    const std::vector<Token>& tokens
);

// extracts expression from tokens; returns a type checked expression
std::shared_ptr<Expression> ExtractExpression(
    const std::vector<Token>& tokens,

    const std::size_t start,
    const std::size_t end,

    const std::vector<CheckVariable>& variables,
    const std::vector<CheckFunction>& functions,

    VariableType* type = nullptr
);

// extracts condition from tokens; returns a type checked expression
std::shared_ptr<Expression> ExtractCondition(
    const std::vector<Token>& tokens,
    std::size_t& closeBracketIndex,

    const std::vector<CheckVariable>& variables,
    const std::vector<CheckFunction>& functions,

    const std::string& stmt,
);

// extracts body from tokens; returns a parsed statement vector
std::vector<Statement> ExtractBody(
    const std::vector<Token>& tokens,
    const std::size_t closeBracketIndex,

    std::vector<CheckVariable>& variables, // can't be const since variables need to be removed after scope finished

    const std::string& stmt,

    const bool inFunction = false
);