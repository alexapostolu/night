#pragma once

#include "token.h"

#include <vector>

// splits an array of tokens into different statements
std::vector<std::vector<Token> > SplitCode(const std::vector<Token>& tokens)
{
    std::vector<std::vector<Token> > code(1);
    for (std::size_t a = 0, openCurlyCount = 0; a < tokens.size(); ++a)
    {
        if (tokens[a].type == TokenType::EOL && openCurlyCount == 0)
        {
            if (code.back()[0].type != TokenType::IF)
                code.push_back(std::vector<Token>());

            continue;
        }

        if (tokens[a].type == TokenType::OPEN_CURLY)
            openCurlyCount++;
        else if (tokens[a].type == TokenType::CLOSE_CURLY)
            openCurlyCount--;

        code.back().push_back(tokens[a]);
    }

    if (code.size() == 1 || code[0].empty())
        code.clear();

    return code;
}