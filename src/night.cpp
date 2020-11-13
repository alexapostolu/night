#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include "utils.h"
#include "error.h"
#include "token.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <variant>

std::vector<Token> OpenFile(const std::string& file)
{
    std::ifstream sourceFile(file);
    if (!sourceFile.is_open())
        throw Error("file '" + file + "' could not be opened");

    std::vector<Token> tokens;
    std::string fileLine;
    for (int line = 0; getline(sourceFile, fileLine); ++line)
    {
        std::vector<Token> fileTokens = Lexer(file, line, fileLine);
        if (fileTokens.size() >= 1 && fileTokens[0].type == TokenType::IMPORT)
        {
            if (fileTokens.size() == 1 || fileTokens[1].type != TokenType::STRING_VAL)
                throw Error(file, line, "expected file name after '" + fileTokens[0].value + "' statement");
            if (fileTokens.size() > 2)
                throw Error(file, line, fileTokens[0].value + " statement must be on it's own line");

            std::vector<Token> importTokens = OpenFile(
                fileTokens[0].value == "import"
                ? "../pkgs/" + fileTokens[1].value + ".night"
                : fileTokens[1].value + ".night"
            );

            fileTokens.erase(fileTokens.begin());
            fileTokens.erase(fileTokens.begin());

            fileTokens.insert(fileTokens.begin(), importTokens.begin(), importTokens.end());
        }

        tokens.insert(tokens.end(), fileTokens.begin(), fileTokens.end());
    }

    sourceFile.close();

    return tokens;
}

void EntryPoint(const std::string& file)
{
    std::vector<std::vector<Token> > code = SplitCode(OpenFile(file));
    std::vector<Statement> statements;
    for (const std::vector<Token>& tokens : code)
        Parser(statements, tokens);

    Interpreter(statements);
}

int main(int argc, char* argv[])
{
    try {
        if (argc > 2)
            throw Error("invalid command line arguments");
        
        EntryPoint(argc == 1 ? "C:\\dev\\solutions\\Night\\Night v4\\Night\\source.night" : argv[1]);
    }
    catch (const Error& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}