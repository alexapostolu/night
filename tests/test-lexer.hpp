#pragma once

#include "../night/include/back-end/lexer.hpp"

#include <iostream>
#include <string>

int TestLexer()
{
    using std::cout;
    using std::string;

    cout << "testing lexer... ";

    Lexer lexer("./test-lexer-code.night", true);

    string toks[] = {
        "set",
        "var",
        ":",
        "int",
        "=",
        "2",
        "+", 
        "(",
        "3",
        ")"
    };

    for (int a = 0; a < sizeof(toks) / sizeof(toks[0]); ++a)
    {
        string tok = lexer.eat(false).data;
        if (tok != toks[a])
        {
            cout << "\n";
            cout << "error when scanning tokens!\n";
            cout << "expected token '" + toks[a] + "' but found token '" + tok + "' instead\n";

            return 1;
        }
    }

    cout << "success!\n";
    return 0;
}