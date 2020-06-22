#include <iostream>
#include <regex>
#include <string>
#include <vector>

// enum TokenType
// do that later since I still print out the tokens
// so I need them as a string, but in the future
// change it to a enum cause it looks better

std::vector<std::string> errors;

struct Variable
{
    std::string type;
    std::string name;
    std::string value;
};

std::vector<Variable> variables;

struct Token
{
    std::string type;
    std::string value;
};

void Lexer(std::vector<Token>& tokens, char input)
{
    static std::string token = "";

    if (input != ';' && input != ' ')
        token += input;

    if (input == ' ' || input == ';')
    {
        if (token == "bool" || token == "char" || token == "int")
        {
            tokens.push_back(Token{ "DATA_TYPE", token });
        }
        else if (token == "=" || token == "+" || token == "-" || token == "*" || token == "/")
        {
            tokens.push_back(Token{ "OPERATOR", token });
        }
        else if ((std::regex_match(token, std::regex("[0-9]+"))) ||
                 (token.length() == 3 && token[0] == '\'' && token[2] == '\'') ||
                 (token == "true" || token == "false"))
        {
            tokens.push_back(Token{ "VALUE", token });
        }
        else if (token != "")
        {
            tokens.push_back(Token{ "VARIABLE", token });
        }

        if (input == ';')
        {
            tokens.push_back(Token{ "SYMBOL", std::string(1, input) });
        }

        token = "";
    }
}

void Parser(const std::vector<Token>& tokens)
{
    if (tokens[0].type == "DATA_TYPE")
    {
        if (tokens[1].type == "VARIABLE")
        {
            if (tokens[2].value == "=")
            {
                // calculate actual value first, for example:
                // int var = 5 + 3;
                // calculate '5 + 3' before moving on and trying to assign this a value
                for (std::size_t a = 0; tokens[a].value != ";"; ++a)
                {
                    if (a > tokens.size())
                    {
                        errors.push_back("Error - semicolon not found");
                        return;
                    }

                    if (tokens[a].value == "*")
                    {

                    }
                }

                if ((std::regex_match(tokens[3].value,
                        std::regex("[0-9]+")) && tokens[0].value == "int") ||
                    (tokens[3].value.length() == 3 && tokens[3].value[0] == '\'' &&
                        tokens[3].value[2] == '\'' && tokens[0].value == "char") ||
                    ((tokens[3].value == "true" || tokens[3].value == "false") &&
                        tokens[0].value == "bool"))
                {
                    variables.push_back(Variable{tokens[0].value,
                        tokens[1].value, tokens[3].value});
                }
                else
                {
                    errors.push_back("Error - wrong value");
                }
            }
            else
            {
                errors.push_back("Error - expected assignment operator");
            }
        }
        else
        {
            errors.push_back("Error - expected variable");
        }
    }
    else
    {
        // throw error!
    }
}

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
