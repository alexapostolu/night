#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include "variable.h"
#include "token.h"
#include "error.h"
#include "lexer.h"
#include "parser.h"

int main()
{
    std::string code = "char answer = 'c';";
    std::vector<Token> tokens;

    for (std::size_t a = 0; a < code.length(); ++a)
    {
        Lexer(tokens, code[a]);
    }

    Parser(tokens);

    std::cout << code << "\n\n";

    for (std::size_t a = 0; a < tokens.size(); ++a)
    {
        std::cout << tokens[a].type << ": (" << tokens[a].value << ")\n";
    }

    std::cout << '\n';

    for (std::size_t a = 0; a < variables.size(); ++a)
    {
        std::cout << variables[a].type << ' ' << variables[a].name << ' ' <<
            variables[a].value << '\n';
    }

    std::cout << '\n';

    for (std::size_t a = 0; a < errors.size(); ++a)
    {
        std::cout << errors[a] << '\n';
    }
}
