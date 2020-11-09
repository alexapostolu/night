#pragma once

#include "token.h"

#include <vector>

// splits an array of tokens into different statements
std::vector<std::vector<Token> > SplitCode(const std::vector<Token>& tokens)
{
    for (const Token& token : tokens)
        std::clog << token.value << ' ';
    std::clog << '\n';

    std::vector<std::vector<Token> > code;
    for (std::size_t a = 0, openCurlyCount = 0; a < tokens.size(); ++a)
    {
        if (a == 0)
            code.push_back(std::vector<Token>());

        if (tokens[a].type == TokenType::EOL && openCurlyCount == 0)
        {
            if (a < tokens.size() - 1)
                code.push_back(std::vector<Token>());

            continue;
        }

        if (tokens[a].type == TokenType::OPEN_CURLY)
            openCurlyCount++;
        else if (tokens[a].type == TokenType::CLOSE_CURLY)
            openCurlyCount--;

        code.back().push_back(tokens[a]);
    }

    return code;
}


// variable type to string
std::string VarTypeToStr(const VariableType& type)
{
    switch (type)
    {
    case VariableType::BOOL:
        return "boolean";
    case VariableType::BOOL_ARR:
        return "boolean array";
    case VariableType::NUM:
        return "number";
    case VariableType::NUM_ARR:
        return "number array";
    case VariableType::STRING:
        return "string";
    case VariableType::STRING_ARR:
        return "string array";
    default:
        assert(true, "variable type is missing");
    }
}