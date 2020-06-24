#pragma once

#include <regex>
#include <string>
#include <vector>

#include "Token.h"
#include "Variable.h"

std::string MathParser(const std::vector<Token>& tokens, const std::vector<Variable>& variables);

void VariableDeclaration(const std::vector<Token>& tokens, std::vector<Variable>& variables)
{
	variables.push_back(Variable{ tokens[0].token, tokens[1].token,
		tokens[0].type == TokenTypes::BOOL ? "false" : "0" });
}

void VariableInitialization(const std::vector<Token>& tokens, std::vector<Variable>& variables)
{
    variables.push_back(Variable{ tokens[0].token, tokens[1].token,
        MathParser(tokens, variables) });
}


void PerformSmartMathStuff(std::vector<std::string>& expression, int& a, char operation)
{
    int value;
    if (operation == '+')
        value = stoi(expression[a - 1]) + stoi(expression[a + 1]);
    else if (operation == '-')
        value = stoi(expression[a - 1]) - stoi(expression[a + 1]);
    else if (operation == '*')
        value = stoi(expression[a - 1]) * stoi(expression[a + 1]);
    else
        value = stoi(expression[a - 1]) / stoi(expression[a + 1]);

    expression[a - 1] = std::to_string(value);

    expression.erase(expression.begin() + a);
    expression.erase(expression.begin() + a);

    a -= 2;
}

std::string MathParser(const std::vector<Token>& tokens, const std::vector<Variable>& variables)
{
    std::vector<std::string> expression(tokens.size() - 4);
    for (std::size_t a = 3; a < tokens.size() - 1; ++a)
    {
        if (tokens[a].type == TokenTypes::INT_VALUE)
        {
            expression[a - 3] = tokens[a].token;
        }
        else if (tokens[a].type == TokenTypes::VARIABLE)
        {
            for (std::size_t b = 0; b < variables.size(); ++b)
            {
                if (variables[b].type == "int")
                {
                    expression[a - 3] = variables[b].value;
                    break;
                }
            }
        }
        else if ((int)tokens[a].type >= 8 && (int)tokens[a].type <= 13)
        {
            expression[a - 3] = tokens[a].token;
        }
        else
        {
            std::cout << "Error - Cannot convert type '" << tokens[a].token << "' into 'int'";
            exit(0);
        }
    }

    int openBracket, closeBracket;
    for (int a = 0; a < expression.size(); ++a)
    {
        if (expression[a] == "(")
        {
            openBracket = a;
        }
        else if (expression[a] == ")")
        {
            closeBracket = a;

            for (int b = openBracket + 1; b < closeBracket - 1; ++b)
            {
                if (expression[b] == "*")
                {
                    PerformSmartMathStuff(expression, b, '*');
                    closeBracket -= 2;
                }
                else if (expression[b] == "/")
                {
                    PerformSmartMathStuff(expression, b, '/');
                    closeBracket -= 2;
                }
            }

            for (int b = openBracket + 1; b < closeBracket - 1; ++b)
            {
                if (expression[b] == "+")
                {
                    PerformSmartMathStuff(expression, b, '+');
                    closeBracket -= 2;
                }
                else if (expression[b] == "-")
                {
                    PerformSmartMathStuff(expression, b, '-');
                    closeBracket -= 2;
                }
            }

            expression.erase(expression.begin() + openBracket);
            expression.erase(expression.begin() + closeBracket - 1);

            a = -1;
        }
    }

    for (int a = 0; a < expression.size(); ++a)
    {
        if (expression[a] == "*")
            PerformSmartMathStuff(expression, a, '*');
        else if (expression[a] == "/")
            PerformSmartMathStuff(expression, a, '/');
    }

    for (int a = 0; a < expression.size(); ++a)
    {
        if (expression[a] == "+")
            PerformSmartMathStuff(expression, a, '+');
        else if (expression[a] == "-")
            PerformSmartMathStuff(expression, a, '-');
    }

    return expression[0];
}
