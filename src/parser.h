#pragma once

#include "utils.h"
#include "token.h"
#include "error.h"

#include <iostream>
#include <vector>
#include <string>

// index starts right after opening bracket; advances index to closing bracket
template <typename Unit, typename UnitType>
void AdvanceCloseBracketIndex(const std::string& file, int line, const std::vector<Unit>& units,
    const UnitType& openBracket, const UnitType& closeBracket, std::size_t& index)
{
    for (int openBracketCount = 0; index < units.size(); ++index)
    {
        if (units[index].type == closeBracket && openBracketCount == 0)
            return;

        if (units[index].type == openBracket)
            openBracketCount++;
        else if (units[index].type == closeBracket)
            openBracketCount--;
    }

    throw Error(file, line, "missing closing bracket");
}

// turns an array of tokens to an array of values
std::vector<Value> TokensToValues(const std::vector<Token>& tokenExpr, VariableType* exprType = nullptr)
{
    assert(tokenExpr.size() == 0, "tokenExpr shouldn't be empty");

    std::vector<Value> expression;
    for (std::size_t a = 0; a < tokenExpr.size(); ++a)
    {
        if (tokenExpr[a].type == TokenType::VARIABLE)
        {
            if (a < tokenExpr.size() - 1 && tokenExpr[a + 1].type == TokenType::OPEN_BRACKET)
            {
                Value functionCall{ ValueType::CALL };
                std::vector<Token> paramTokenExpr;

                a += 2;
                for (int openBracketCount = 0; a < tokenExpr.size(); ++a)
                {
                    if (tokenExpr[a].type == TokenType::OPEN_BRACKET)
                        openBracketCount++;
                    else if (tokenExpr[a].type == TokenType::CLOSE_BRACKET)
                        openBracketCount--;
                    
                    if ((tokenExpr[a].type == TokenType::COMMA && openBracketCount == 0) ||
                        (tokenExpr[a].type == TokenType::CLOSE_BRACKET && openBracketCount == -1))
                    {
                        std::vector<Value> tokenParam = TokensToValues(paramTokenExpr);
                        functionCall.values.push_back(tokenParam[0]);

                        if (tokenExpr[a].type == TokenType::CLOSE_BRACKET)
                            break;

                        continue;
                    }

                    paramTokenExpr.push_back(tokenExpr[a]);
                }

                if (a == tokenExpr.size())
                    throw Error(tokenExpr[0].file, tokenExpr[0].line, "missing closing bracket for function call");
            }
            else if (a < tokenExpr.size() - 1 && tokenExpr[a + 1].type == TokenType::OPEN_SQUARE)
            {
                // array element
                // if tokenExpr[a] == NUM: array init
            }
            else
            {
                expression.push_back(Value{ ValueType::VARIABLE, tokenExpr[a].value });
            }
        }
        else if (tokenExpr[a].type == TokenType::OPEN_SQUARE)
        {
            // array
        }
        else if (a < tokenExpr.size() - 1 && tokenExpr[a].type == TokenType::NUM_VAL && tokenExpr[a + 1].type == TokenType::OPEN_SQUARE)
        {
            Value array;
            array.value = std::to_string(std::stoi(tokenExpr[a].value));

            a++;

            std::vector<Token> elementTokens;
            for (int openBracketCount = 0, openSquareCount = 0; a < tokenExpr.size(); ++a)
            {
                if (tokenExpr[a].type == TokenType::OPEN_BRACKET)
                    openBracketCount++;
                else if (tokenExpr[a].type == TokenType::CLOSE_BRACKET)
                    openBracketCount--;
                else if (tokenExpr[a].type == TokenType::OPEN_SQUARE)
                    openSquareCount++;
                else if (tokenExpr[a].type == TokenType::CLOSE_SQUARE)
                    openSquareCount--;

                if (tokenExpr[a].type == TokenType::CLOSE_SQUARE && openSquareCount == -1)
                {
                    break;
                }

                if (tokenExpr[a].type == TokenType::COMMA && openBracketCount == 0 && openSquareCount == 0)
                {
                    array.values.push_back(TokensToValues(tokenExpr)[0]);
                    continue;
                }

                elementTokens.push_back(tokenExpr[a]);
            }
        }
        else
        {
            switch (tokenExpr[a].type)
            {
            case TokenType::BOOL_VAL:
                expression.push_back(Value{ ValueType::BOOL, tokenExpr[a].value });
                break;
            case TokenType::NUM_VAL:
                expression.push_back(Value{ ValueType::NUM, tokenExpr[a].value });
                break;
            case TokenType::STRING_VAL:
                expression.push_back(Value{ ValueType::STRING, tokenExpr[a].value });
                break;
            case TokenType::OPERATOR:
                expression.push_back(Value{ ValueType::OPERATOR, tokenExpr[a].value });
                break;
            default:
                throw Error(tokenExpr[a].file, tokenExpr[a].line, "unexpected token '" + tokenExpr[a].value + "' in expression");
            }
        }
    }

    // check expression
    // exprType = something

    return expression;
}

// checks for function definitions in a scope; if found, throw error
void CheckFunctionDefinition(const Scope& scope, const std::string& file, int line, const std::string& errorMsg)
{
    for (const Statement& statement : scope.statements)
    {
        if (statement.type == StatementType::FUNCTION_DEF)
            throw Error(file, line, errorMsg);
    }
}

// used by 'ParseValues()'; returns operator precedence
int GetOperatorPrecedence(const Value& token)
{
    static std::vector<std::vector<std::string> > operators{
        { "!" },
        { "*", "/", "%" },
        { "+", "-" },
        { ">", "<", ">=", "<=" },
        { "==", "!=" }
    };

    for (std::size_t a = 0; a < operators.size(); ++a)
    {
        for (const std::string& op : operators[a])
        {
            if (token.values[0].value == op)
                return a;
        }
    }

    assert(true, "operator missing from vector");
}

// used by 'ParseValues()'; returns expression within a set of brackets
std::vector<Value> GetBracketExpression(const std::string& file, int line, const std::vector<Value>& expression, std::size_t& index)
{
    std::size_t start = index;
    AdvanceCloseBracketIndex(file, line, expression, ValueType::OPEN_BRACKET, ValueType::CLOSE_BRACKET, index);

    return std::vector<Value>(expression.begin() + start, expression.begin() + index);
}

// turns an array of values into an AST
Expression* ParseValues(const std::string& file, int line, const std::vector<Value>& values)
{
    assert(values.empty(), "expression should not be empty");

    std::size_t a = 1;

    // special case - brackets at start of expression

    Expression* root = nullptr;
    if (values[0].type == ValueType::OPEN_BRACKET)
    {
        std::vector<Value> bracketExpr = GetBracketExpression(file, line, values, a);
        root = ParseValues(file, line, bracketExpr);
        a++;
    }
    else
    {
        root = new Expression{ values[0], nullptr, nullptr };
    }

    // parse expression

    Expression* curr = root;
    for (; a < values.size(); ++a)
    {
        if (values[a].type != ValueType::OPERATOR)
            continue;

        int save_a = a;

        // evaluating brackets

        Expression* bracketExpression = nullptr;
        if (a < values.size() - 1 && values[a + 1].type == ValueType::OPEN_BRACKET)
        {
            a += 2;

            std::vector<Value> bracketValues = GetBracketExpression(file, line, values, a);
            bracketExpression = ParseValues(file, line, bracketValues);
        }

        // traveling tree

        while ((curr->left != nullptr || curr->right != nullptr) &&
            GetOperatorPrecedence(values[a]) < GetOperatorPrecedence(curr->value))
            curr = curr->right;

        // creating node

        Expression* node = new Expression{
            values[save_a],
            curr,
            bracketExpression == nullptr
                ? new Expression{ values[a + 1], nullptr, nullptr }
                : bracketExpression
        };

        if (curr == root)
        {
            curr = node;
            root = node;
        }
        else
        {
            curr->right = node;
            curr = root;
        }
    }

    return root;
}

// parses an array of tokens into an array of statements
void Parser(std::vector<Statement>& statements, const std::vector<Token>& tokens)
{
    assert(tokens.size() == 0, "tokens array should not be empty");

    const std::string file = tokens[0].file;
    const int line = tokens[0].line;

    if (tokens.size() >= 1 && tokens[0].type == TokenType::SET)
    {
        if (tokens.size() == 1 || tokens[1].type != TokenType::VARIABLE)
            throw Error(file, line, "expected variable name after 'set' keyword");
        if (tokens.size() == 2 || tokens[2].type != TokenType::ASSIGNMENT)
            throw Error(file, line, "expected assignment operator after variable name");
        if (tokens.size() == 3)
            throw Error(file, line, "expected expression after assignment operator");

        VariableType exprType;
        std::vector<Value> expression = TokensToValues(std::vector<Token>(tokens.begin() + 2, tokens.end()), &exprType);

        Statement statement{ StatementType::VARIABLE };
        statement.stmt = Variable{
            tokens[1].value,
            ParseValues(file, line, expression)
        };

        statements.push_back(statement);
    }
    else if (tokens.size() >= 2 && tokens[0].type == TokenType::VARIABLE && tokens[1].type == TokenType::ASSIGNMENT)
    {
        if (tokens.size() == 2)
            throw Error(file, line, "expected expression after assignment operator");

        std::vector<Value> expression = TokensToValues(std::vector<Token>(tokens.begin() + 2, tokens.end()));

        Statement statement{ StatementType::ASSIGNMENT };
        statement.stmt = Assignment{
            tokens[0].value,
            ParseValues(file, line, expression)
        };

        statements.push_back(statement);
    }
    else if (tokens.size() >= 1 && tokens[0].type == TokenType::IF)
    {
        if (tokens.size() == 1 || tokens[1].type != TokenType::OPEN_BRACKET)
            throw Error(file, line, "expected open bracket after 'if' keyword");
        if (tokens.size() == 2)
            throw Error(file, line, "expected close bracket in if condition");
        if (tokens.size() == 3 && tokens[2].type == TokenType::CLOSE_BRACKET)
            throw Error(file, line, "expected expression in between brackets");

        // if statement condition

        std::size_t closeBracketIndex = 2;
        AdvanceCloseBracketIndex(file, line, tokens, TokenType::OPEN_BRACKET, TokenType::CLOSE_BRACKET, closeBracketIndex);
        
        VariableType exprType;
        std::vector<Value> condition = TokensToValues(
            std::vector<Token>(tokens.begin() + 2, tokens.begin() + closeBracketIndex),
            &exprType
        );

        if (exprType != VariableType::BOOL)
            throw Error(file, line, "if statement condition can't be a " + VarTypeToStr(exprType) + "; it must be a boolean value");

        // if statement body

        Scope scope;
        std::vector<std::vector<Token> > code = SplitCode(std::vector<Token>(tokens.begin() + closeBracketIndex + 2, tokens.end() - 1));
        for (const std::vector<Token>& tokens : code)
            Parser(scope.statements, tokens);

        CheckFunctionDefinition(scope, file, line, "if statements cannot contain function definitions");

        Statement statement{ StatementType::CONDITIONAL };
        statement.stmt = Conditional{
            ParseValues(file, line, condition),
            scope
        };

        statements.push_back(statement);
    }
    else if (tokens.size() >= 2 && tokens[0].type == TokenType::ELSE && tokens[1].type == TokenType::IF)
    {
        if (tokens.size() == 2 || tokens[2].type != TokenType::OPEN_BRACKET)
            throw Error(file, line, "expected open bracket after 'if' keyword");
        if (tokens.size() == 3)
            throw Error(file, line, "expected close bracket in if condition");
        if (tokens.size() == 4 && tokens[3].type == TokenType::CLOSE_BRACKET)
            throw Error(file, line, "expected expression in between brackets");

        if (statements.size() == 0 || statements.back().type != StatementType::CONDITIONAL ||
            std::get<Conditional>(statements.back().stmt).condition == nullptr)
            throw Error(file, line, "expected if or else if statement before else if statement");

        // else if statement condition

        std::size_t closeBracketIndex = 3;
        AdvanceCloseBracketIndex(file, line, tokens, TokenType::OPEN_BRACKET, TokenType::CLOSE_BRACKET, closeBracketIndex);

        VariableType exprType;
        std::vector<Value> condition = TokensToValues(
            std::vector<Token>(tokens.begin() + 3, tokens.begin() + closeBracketIndex),
            &exprType
        );

        if (exprType != VariableType::BOOL)
            throw Error(file, line, "else if statement condition can't be a " + VarTypeToStr(exprType) + "; it must be a boolean value");

        // else if statement body

        Scope scope;
        std::vector<std::vector<Token> > code = SplitCode(std::vector<Token>(tokens.begin() + closeBracketIndex + 2, tokens.end() - 1));
        for (const std::vector<Token>& tokens : code)
            Parser(scope.statements, tokens);

        CheckFunctionDefinition(scope, file, line, "else if statements cannot contain functions");

        std::get<Conditional>(statements.back().stmt).chains.push_back(Conditional{ ParseValues(file, line, condition), scope });
    }
    else if (tokens.size() >= 1 && tokens[0].type == TokenType::ELSE)
    {
        if (tokens.size() == 2 || tokens[2].type != TokenType::OPEN_CURLY)
            throw Error(file, line, "expected open brace after 'else' keyword");

        if (statements.size() == 0 || statements.back().type != StatementType::CONDITIONAL ||
            std::get<Conditional>(statements.back().stmt).condition == nullptr)
            throw Error(file, line, "expected if or else if statement before else statement");

        // else statement body

        Scope scope;
        std::vector<std::vector<Token> > code = SplitCode(std::vector<Token>(tokens.begin() + 2, tokens.end() - 1));
        for (const std::vector<Token>& tokens : code)
            Parser(scope.statements, tokens);

        CheckFunctionDefinition(scope, file, line, "else statements cannot contain functions");
        
        std::get<Conditional>(statements.back().stmt).chains.push_back(Conditional{ nullptr, scope });
    }
    else if (tokens.size() >= 1 && tokens[0].type == TokenType::DEF)
    {
        if (tokens.size() == 1 || tokens[1].type != TokenType::VARIABLE)
            throw Error(file, line, "expected function name after 'def' keyword");
        if (tokens.size() == 2 || tokens[2].type != TokenType::OPEN_BRACKET)
            throw Error(file, line, "expected open bracket after function name");
        if (tokens.size() == 3)
            throw Error(file, line, "expected open bracket after function name");
        if (tokens.size() == 4 && tokens[3].type != TokenType::CLOSE_BRACKET)
            throw Error(file, line, "expected closing bracket for function parameters");
        if (tokens.size() == 4 || (tokens.size() == 5 || tokens[4].type != TokenType::OPEN_CURLY))
            throw Error(file, line, "expected open curly bracket for function body");

        Statement statement{ StatementType::FUNCTION_DEF };
        std::get<FunctionDef>(statement.stmt).name = tokens[1].value;

        // function parameters

        std::size_t closeBracketIndex = 3;
        for (; closeBracketIndex < tokens.size(); closeBracketIndex += 2)
        {
            if (tokens[closeBracketIndex].type != TokenType::VARIABLE)
                throw Error(file, line, "expected variable names as function parameters");

            std::get<FunctionDef>(statement.stmt).parameters.push_back(tokens[closeBracketIndex].value);

            if (tokens[closeBracketIndex + 1].type == TokenType::CLOSE_BRACKET)
            {
                closeBracketIndex++;
                break;
            }

            if (tokens[closeBracketIndex + 1].type != TokenType::COMMA)
                throw Error(file, line, "expected comma or closing bracket after function parameter");
        }

        // function body

        Parser(
            std::get<FunctionDef>(statement.stmt).body.statements,
            std::vector<Token>(tokens.begin() + closeBracketIndex + 2, tokens.end() - 1)
        );

        CheckFunctionDefinition(std::get<FunctionDef>(statement.stmt).body, file, line, "functions cannot contain functions");

        statements.push_back(statement);
    }
    else if (tokens.size() >= 2 && tokens[0].type == TokenType::VARIABLE && tokens[1].type == TokenType::OPEN_BRACKET)
    {
        if (tokens.size() == 2 || tokens.size() == 3 || tokens[2].type != TokenType::CLOSE_BRACKET)
            throw Error(file, line, "missing closing bracket");

        // function parameters

        Statement statement{ StatementType::FUNCTION_CALL };
        std::get<FunctionCall>(statement.stmt).name = tokens[0].value;

        std::vector<Token> tokenParam;
        for (std::size_t a = 2, openBracketIndex = 0; a < tokens.size(); ++a)
        {
            if (tokens[a].type == TokenType::OPEN_BRACKET)
                openBracketIndex++;
            else if (tokens[a].type == TokenType::CLOSE_BRACKET)
                openBracketIndex--;

            if (tokens[a].type == TokenType::CLOSE_BRACKET && openBracketIndex == -1)
            {
                if (a < tokens.size() - 1)
                    throw Error(file, line, "unexpected tokens after function call; each statement must be on it's own line");

                std::vector<Value> paramExpr = TokensToValues(tokenParam);
                std::get<FunctionCall>(statement.stmt).parameters.push_back(ParseValues(file, line, paramExpr));

                tokenParam.clear();
                continue;
            }
            
            if (tokens[a].type == TokenType::COMMA && openBracketIndex == 0)
            {
                std::vector<Value> paramExpr = TokensToValues(tokenParam);
                std::get<FunctionCall>(statement.stmt).parameters.push_back(ParseValues(file, line, paramExpr));

                tokenParam.clear();
                continue;
            }

            tokenParam.push_back(tokens[a]);
        }

        statements.push_back(statement);
    }
    else if (tokens.size() >= 1 && tokens[0].type == TokenType::WHILE)
    {
        if (tokens.size() == 1 || tokens[1].type != TokenType::OPEN_BRACKET)
            throw Error(file, line, "expected open bracket after 'while' keyword");
        if (tokens.size() == 2)
            throw Error(file, line, "expected condition in while loop");
        if (tokens.size() == 3 && tokens[2].type != TokenType::CLOSE_BRACKET)
            throw Error(file, line, "expected closing bracket in condition");
        if (tokens.size() == 3 && tokens[2].type == TokenType::CLOSE_BRACKET)
            throw Error(file, line, "expected condition in between brackets");

        Statement statement{ StatementType::WHILE_LOOP };

        // while loop condition

        std::size_t closeBracketIndex = 2;
        AdvanceCloseBracketIndex(file, line, tokens, TokenType::OPEN_BRACKET, TokenType::CLOSE_BRACKET, closeBracketIndex);

        if (closeBracketIndex == tokens.size() - 1 || tokens[closeBracketIndex].type != TokenType::OPEN_CURLY)
            throw Error(file, line, "expected open curly bracket for while loop body");

        VariableType conditionType;
        std::vector<Value> condition = TokensToValues(
            std::vector<Token>(tokens.begin() + 2, tokens.begin() + closeBracketIndex),
            &conditionType
        );

        if (conditionType != VariableType::BOOL)
            throw Error(file, line, "while loop condition can't be a " + VarTypeToStr(conditionType) + "; it must be a boolean");

        // while loop body

        std::get<WhileLoop>(statement.stmt).condition = ParseValues(file, line, condition);
        Parser(
            std::get<WhileLoop>(statement.stmt).body.statements,
            std::vector<Token>(tokens.begin() + closeBracketIndex + 1, tokens.end() - 1)
        );

        CheckFunctionDefinition(std::get<WhileLoop>(statement.stmt).body, file, line, "while loops cannot contain functions");

        statements.push_back(statement);
    }
    else if (tokens.size() >= 1 && tokens[0].type == TokenType::FOR)
    {
        if (tokens.size() == 1 || tokens[1].type != TokenType::OPEN_BRACKET)
            throw Error(file, line, "expected open bracket after 'for' keyword");
        if (tokens.size() == 2 || tokens[2].type != TokenType::VARIABLE)
            throw Error(file, line, "expected iterator name after open bracket");
        if (tokens.size() == 3 || tokens[3].type != TokenType::COLON)
            throw Error(file, line, "expected colon after iterator name");
        if (tokens.size() == 4)
            throw Error(file, line, "expected array after colon");

        // iterator and range

        std::size_t closeBracketIndex = 0;
        AdvanceCloseBracketIndex(file, line, tokens, TokenType::OPEN_BRACKET, TokenType::CLOSE_BRACKET, closeBracketIndex);

        VariableType rangeType;
        std::vector<Value> rangeValue = TokensToValues(
            std::vector<Token>(tokens.begin() + 4, tokens.begin() + closeBracketIndex),
            &rangeType
        );

        if (rangeType != VariableType::BOOL_ARR && rangeType != VariableType::NUM_ARR && rangeType != VariableType::STRING_ARR)
            throw Error(file, line, "for loop range cannot be a '" + VarTypeToStr(rangeType) + "'; it can only be an array");

        // scope

        Scope scope;
        Parser(scope.statements, std::vector<Token>(tokens.begin() + closeBracketIndex + 1, tokens.end() - 1));

        Statement statement{ StatementType::FOR_LOOP };
        statement.stmt = ForLoop{
            tokens[1].value,
            ParseValues(file, line, rangeValue),
            scope
        };

        statements.push_back(statement);
    }
    else if (tokens.size() >= 2 && tokens[0].type == TokenType::VARIABLE && tokens[1].type == TokenType::OPEN_SQUARE)
    {
        if (tokens.size() == 2)
            throw Error(file, line, "expected index after open square");

        Statement statement{ StatementType::ELEMENT };

        std::size_t assignmentIndex = 2;
        for (; assignmentIndex < tokens.size() && tokens[assignmentIndex].type != TokenType::ASSIGNMENT; ++assignmentIndex);

        if (tokens[assignmentIndex - 1].type != TokenType::CLOSE_SQUARE)
            throw Error(file, line, "missing closing square");
        if (assignmentIndex == tokens.size())
            throw Error(file, line, "missing assignment operator");

        VariableType indexType;
        std::vector<Value> indexValues = TokensToValues(std::vector<Token>(tokens.begin() + 2, tokens.begin() + assignmentIndex - 1), &indexType);

        if (indexType != VariableType::NUM)
            throw Error(file, line, "array index can't be a '" + VarTypeToStr(indexType) + "'; it must be a number");

        std::vector<Value> assignValue = TokensToValues(std::vector<Token>(tokens.begin() + assignmentIndex + 1, tokens.end()));

        statement.stmt = Element{
            tokens[0].value,
            ParseValues(file, line, indexValues),
            ParseValues(file, line, assignValue)
        };

        statements.push_back(statement);
    }
    else
    {
        throw Error(file, line, "invalid syntax");
    }
}