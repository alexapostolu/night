#pragma once

#include <fstream>
#include <string>

#include "lib/array.h"
#include "lib/error.h"
#include "lib/file.h"

#include "containers/token.h"

#include "./lexer.h"
#include "parser/parser.h"
#include "parser/pre.h"
#include "interpreter/interpreter.h"

void ExtractFile(const char* fileName)
{
    std::ifstream codeFile = Read(fileName);

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

    int openCurly = 0;
    bool flip = false;
    for (int a = 0; a < tokens.length(); ++a)
    {
        code.back().push_back(tokens[a]);

        if (tokens[a].type == TokenType::IMPORT || flip)
        {
          if (!flip)
            continue;
          flip = !flip;
          std::stringstream tik;
          tik << "./dusk_pks/" << tokens[a];
          codeFile = Read(tik.str().c_str());
          while (getline(codeFile, fileLine))
          {
              night::array<Token> temp = Lexer(fileLine.c_str());
              for (int a = 0; a < temp.length(); ++a)
                  tokens.push_back(temp[a]);
          }
        }

        if (tokens[a].type == TokenType::OPEN_CURLY)
            openCurly++;
        else if (tokens[a].type == TokenType::CLOSE_CURLY)
            openCurly--;

        if ((tokens[a].type == TokenType::SEMICOLON || tokens[a].type == TokenType::CLOSE_CURLY) &&
            openCurly == 0)
        {
            Parser(code.back());

            if (a < tokens.length() - 1)
                code.push_back(night::array<Token>());
        }
    }

    Interpreter(code);
    PrintOutput();
}