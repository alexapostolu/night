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

void OpenFile(const std::string& file)
{
    std::ifstream sourceFile(file);
    if (!sourceFile.is_open())
        throw Error("source file could not be opened");

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
                throw Error(file, line, "import statement must be on it's own line");

            std::ifstream importFile((fileTokens[0].value == "import" ? "../dusk_pkgs/" : "") + fileTokens[1].value);
            if (!importFile.is_open())
                throw Error(file, line, "file '" + fileTokens[1].value + "' cannot be opened");

            std::vector<Token> importTokens;
            for (int lineNumber2 = 0; getline(importFile, fileLine); ++lineNumber2)
            {
                std::vector<Token> importTemp = Lexer(fileTokens[1].value, lineNumber2, fileLine);
                importTokens.insert(importTokens.end(), importTemp.begin(), importTemp.end());
            }

            importFile.close();

            fileTokens.erase(fileTokens.begin());
            fileTokens.erase(fileTokens.begin());

            fileTokens.insert(fileTokens.begin(), importTokens.begin(), importTokens.end());
        }

        tokens.insert(tokens.end(), fileTokens.begin(), fileTokens.end());
    }

    sourceFile.close();

    std::vector<Statement> statements;
    std::vector<std::vector<Token> > code = SplitCode(tokens);
    for (const std::vector<Token>& tokens : code)
        Parser(statements, tokens);

    Interpreter(statements);
}

int main(int argc, char* argv[])
{
    try {
        if (argc > 2)
            throw Error("invalid command line arguments");
        
        OpenFile(argc == 1 ? "C:\\dev\\solutions\\Night\\Night v4\\Night\\source.night" : argv[1]);
    }
    catch (const Error& e) {
        std::cerr << e.what() << '\n';
    }
}

/*

class AST
{
private:
    struct Node;

public:
    static Token EvaluateExpression(const std::vector<Token>& expression)
    {
        Node* head = CreateTree(expression);
        TravelTree(head);

        Token result = head->token;
        delete head;

        return result;
    }

public:
    static Node* CreateTree(const std::vector<Token>& expression)
    {
        if (expression.size() == 1)
            return new Node{ expression[0], nullptr, nullptr };

        Node* head = nullptr;
        for (std::size_t a = 0; a < expression.size(); ++a)
        {
            if (expression[a].type == TokenType::PLUS)
            {
                if (expression[a + 1].type == TokenType::OPEN_BRACKET)
                {
                    head = new Node{
                        expression[a],
                        head == nullptr ? new Node{ expression[a - 1], nullptr, nullptr } : head,
                        CreateTree(GetBracketExpression(expression, a))
                    };

                    continue;
                }

                head = new Node{
                    expression[a],
                    head == nullptr ? new Node{ expression[a - 1], nullptr, nullptr } : head,
                    new Node{ expression[a + 1], nullptr, nullptr }
                };
            }
            else if (expression[a].value == "*")
            {
                if (head == nullptr)
                {
                    if (expression[a + 1].value == "(")
                    {
                        head = new Node{
                            expression[a],
                            new Node{ expression[a - 1], nullptr, nullptr },
                            CreateTree(GetBracketExpression(expression, a))
                        };

                        continue;
                    }

                    head = new Node{
                        expression[a],
                        new Node{ expression[a - 1], nullptr, nullptr },
                        new Node{ expression[a + 1], nullptr, nullptr }
                    };

                    continue;
                }

                if (expression[a + 1].value == "(")
                {
                    head->right = new Node{
                        expression[a],
                        head->right,
                        CreateTree(GetBracketExpression(expression, a))
                    };

                    continue;
                }

                head->right = new Node{
                    expression[a],
                    head->right,
                    new Node{ expression[a + 1], nullptr, nullptr }
                };
            }
        }

        return head;
    }

    static void TravelTree(Node* node)
    {
        if (node->left == nullptr || node->right == nullptr)
            return;

        TravelTree(node->left);
        TravelTree(node->right);

        if (node->left->token.type == "val" && node->right->token.type == "val")
        {
            if (node->token.value == "+")
                node->token = Token{ "val", std::to_string(std::stoi(node->left->token.value) + std::stoi(node->right->token.value)) };
            else if (node->token.value == "*")
                node->token = Token{ "val", std::to_string(std::stoi(node->left->token.value) * std::stoi(node->right->token.value)) };

            delete node->left;
            delete node->right;
        }
    }

private:
    static std::vector<Token> GetBracketExpression(const std::vector<Token>& expression, std::size_t& index)
    {
        index += 2;

        std::vector<Token> bracket_expression;
        for (int count = 0; index < expression.size(); ++index)
        {
            if (expression[index].value == "(")
                count++;
            else if (expression[index].value == ")")
                count--;

            if (expression[index].value == ")" && count == -1)
                return bracket_expression;

            bracket_expression.push_back(expression[index]);
        }
    }

private:
    struct Node
    {
        Token token;
        Node* left;
        Node* right;
    };

    Node* head;
};

*/

/*


std::vector<Value> GetBracketExpression(const std::vector<Value>& expression, int& index)
{
    index += 2;

    std::vector<Value> bracket_expression;
    for (int count = 0; index < expression.size(); ++index)
    {
        if (expression[index].type == ValueType::OPEN_BRACKET)
            count++;
        else if (expression[index].type == ValueType::CLOSE_BRACKET)
            count--;

        if (expression[index].type == ValueType::CLOSE_BRACKET && count == -1)
            return bracket_expression;

        bracket_expression.push_back(expression[index]);
    }
}

Expression* ParseExpression(const std::vector<Value>& expression)
{
    if (expression.size() == 1)
        return new Expression{ expression[0], nullptr, nullptr };

    Expression* head = nullptr;
    for (int a = 0; a < expression.size(); ++a)
    {
        if (GetOperatorPrecedence(expression[a]) == 1)
        {
            if (head == nullptr)
            {
                if (expression[a + 1].type == ValueType::OPEN_BRACKET)
                {
                    head = new Expression{
                        expression[a],
                        new Expression{ expression[a - 1], nullptr, nullptr },
                        ParseExpression(GetBracketExpression(expression, a))
                    };

                    continue;
                }

                head = new Expression{
                    expression[a],
                    new Expression{ expression[a - 1], nullptr, nullptr },
                    new Expression{ expression[a + 1], nullptr, nullptr }
                };

                continue;
            }

            if (expression[a + 1].type == ValueType::OPEN_BRACKET)
            {
                head->right = new Expression{
                    expression[a],
                    head->right,
                    ParseExpression(GetBracketExpression(expression, a))
                };

                continue;
            }

            head->right = new Expression{
                expression[a],
                head->right,
                new Expression{ expression[a + 1], nullptr, nullptr }
            };
        }
        else if (GetOperatorPrecedence(expression[a]) == 2)
        {
            if (expression[a + 1].type == ValueType::OPEN_BRACKET)
            {
                head = new Expression{
                    expression[a],
                    head == nullptr ? new Expression{ expression[a - 1], nullptr, nullptr } : head,
                    ParseExpression(GetBracketExpression(expression, a))
                };

                continue;
            }

            head = new Expression{
                expression[a],
                head == nullptr ? new Expression{ expression[a - 1], nullptr, nullptr } : head,
                new Expression{ expression[a + 1], nullptr, nullptr }
            };
        }
    }

    return head;
}

*/