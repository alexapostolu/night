#pragma once

#include <fstream>
#include <string>

#include "lib/array.h"
#include "lib/error.h"

#include "containers/token.h"

#include "./lexer.h"
#include "parser/parser.h"
#include "interpreter/interpreter.h"

void ExtractFile(const char* fileName)
{
    std::ifstream codeFile(fileName);
    if (!codeFile.is_open())
        throw "source file could not be opened";

    night::array<Token> tokens;
    std::string fileLine;
    while (getline(codeFile, fileLine))
    {
        night::array<Token> temp = Lexer(fileLine.c_str());
        for (int a = 0; a < temp.length(); ++a)
            tokens.push_back(temp[a]);
    }

    codeFile.close();

    night::array<night::array<Token> > code;
    code.push_back(night::array<Token>());

    int openCurly = 0;//, openSquare = 0, openBracket = 0;
    for (int a = 0; a < tokens.length(); ++a)
    {
        code.back().push_back(tokens[a]);

        if (tokens[a].type == TokenType::OPEN_CURLY)
            openCurly++;
        else if (tokens[a].type == TokenType::CLOSE_CURLY)
            openCurly--;
        /*
        else if (tokens[a].type == TokenType::OPEN_SQUARE)
            openSquare++;
        else if (tokens[a].type == TokenType::CLOSE_SQUARE)
            openSquare--;
        else if (tokens[a].type == TokenType::OPEN_CURLY)
            openCurly++;
        else if (tokens[a].type == TokenType::CLOSE_CURLY)
            openBracket--;
        */

        if ((tokens[a].type == TokenType::SEMICOLON || tokens[a].type == TokenType::CLOSE_CURLY) &&
            openCurly == 0)
        {
            Parser(code.back());

            if (a < tokens.length() - 1)
                code.push_back(night::array<Token>());
        }
    }

    /*
    if (openBracket > 0)
        throw Error(night::_invalid_grammar_, code.back(), code.back().length() - 1, code.back().length() - 1, "closing bracket is missing");
    else if (openBracket < 0)
        throw Error(night::_invalid_grammar_, code.back(), code.back().length() - 1, code.back().length() - 1, "opening bracket is missing");
    else if (openSquare > 0)
        throw Error(night::_invalid_grammar_, code.back(), code.back().length() - 1, code.back().length() - 1, "closing square bracket is missing");
    else if (openSquare < 0)
        throw Error(night::_invalid_grammar_, code.back(), code.back().length() - 1, code.back().length() - 1, "opening square bracket is missing");
    else if (openCurly > 0)
        throw Error(night::_invalid_grammar_, code.back(), code.back().length() - 1, code.back().length() - 1, "closing curly bracket is missing");
    else if (openCurly < 0)
        throw Error(night::_invalid_grammar_, code.back(), code.back().length() - 1, code.back().length() - 1, "opening curly bracket is missing");
    else if (code.back().back().type != TokenType::CLOSE_CURLY &&
        code.back().back().type != TokenType::SEMICOLON)
        throw Error(night::_invalid_grammar_, code.back(), code.back().length() - 1, code.back().length() - 1, "semicolon is missing");
    */

    Interpreter(code);
    PrintOutput();
}