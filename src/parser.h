#pragma once

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
std::vector<Value> TokensToExpression(const std::vector<Token>& tokenExpr, VariableType* exprType = nullptr)
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
                        std::vector<Value> tokenParam = TokensToExpression(paramTokenExpr);
                        functionCall.values.push_back(tokenParam[0].value);

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
            // array init
        }
        else
        {
            switch (tokenExpr[a].type)
            {
            case TokenType::BOOL_VAL:
                expression.push_back(Value{ ValueType::BOOL, tokenExpr[a].value });
            case TokenType::NUM_VAL:
                expression.push_back(Value{ ValueType::NUM, tokenExpr[a].value });
            case TokenType::STRING_VAL:
                expression.push_back(Value{ ValueType::STRING, tokenExpr[a].value });
            case TokenType::OPERATOR:
                expression.push_back(Value{ ValueType::OPERATOR, tokenExpr[a].value });
            default:
                throw Error(tokenExpr[a].file, tokenExpr[a].line, "unexpected token '" + tokenExpr[a].value + "' in expression");
            }
        }
    }

    // check expression
    // exprType = something

    return expression;
}

// splits an array of tokens into different statements
std::vector<std::vector<Token> > SplitCode(const std::vector<Token>& tokens)
{
    std::vector<std::vector<Token> > code;
    for (int a = -1, openCurlyCount = 0; a < tokens.size(); ++a)
    {
        if (a == -1 || (tokens[a].type == TokenType::EOL && openCurlyCount == 0))
        {
            code.push_back(std::vector<Token>());
            continue;
        }
        else if (tokens[a].type == TokenType::OPEN_CURLY)
        {
            openCurlyCount++;
        }
        else if (tokens[a].type == TokenType::CLOSE_CURLY)
        {
            openCurlyCount--;
        }
        
        code.back().push_back(tokens[a]);
    }

    return code;
}

// checks for function definitions in a scope; if found, throw error
void CheckFunctionDefinition(const Scope& scope, const std::string& file, int line, const std::string& errorMsg)
{
    for (const Statement* statement : scope.statements)
    {
        if (statement->type == StatementType::FUNCTION_DEF)
            throw Error(file, line, errorMsg);
    }
}

// used by 'ParseExpression()'; returns operator precedence
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
            if (token.values[0] == op)
                return a;
        }
    }

    assert(true, "operator missing from vector");
}

// used by 'ParseExpression()'; returns expression within a set of brackets
std::vector<Value> GetBracketExpression(const std::string& file, int line, const std::vector<Value>& expression, std::size_t& index)
{
    std::size_t start = index;
    AdvanceCloseBracketIndex(file, line, expression, ValueType::OPEN_BRACKET, ValueType::CLOSE_BRACKET, index);

    return std::vector<Value>(expression.begin() + start, expression.begin() + index);
}

// turns an array of values into an AST
Expression* ParseExpression(const std::string& file, int line, const std::vector<Value>& expression)
{
    if (expression.empty())
        throw Error(file, line, "expression cannot be empty");

    std::size_t a = 1;

    Expression* root = nullptr;
    if (expression[0].type == ValueType::OPEN_BRACKET)
    {
        std::vector<Value> bracketExpr = GetBracketExpression(file, line, expression, a);
        root = ParseExpression(file, line, bracketExpr);
        a++;
    }
    else
    {
        root = new Expression{ expression[0], nullptr, nullptr };
    }

    Expression* curr = root;
    for (; a < expression.size(); ++a)
    {
        if (expression[a].type == ValueType::CLOSE_BRACKET)
            throw Error(file, line, "missing opening bracket");

        while ((curr->left != nullptr || curr->right != nullptr) &&
            GetOperatorPrecedence(expression[a]) < GetOperatorPrecedence(curr->value))
            curr = curr->right;

        std::vector<Value> bracketExpr;
        if (expression[a + 1].type == ValueType::OPEN_BRACKET)
        {
            a += 2;
            bracketExpr = GetBracketExpression(file, line, expression, a);
        }

        Expression* node = new Expression{
            expression[a],
            curr,
            bracketExpr.empty()
                ? new Expression{ expression[a + 1], nullptr, nullptr }
                : ParseExpression(file, line, bracketExpr)
        };

        curr->right = node;
        curr = root;
    }

    return root;
}

// parses an array of tokens into an array of statements
void Parser(std::vector<Statement*>& statements, const std::vector<Token>& tokens)
{
    assert(tokens.size() == 0, "tokens array shouldn't be empty");

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
        std::vector<Value> expression = TokensToExpression(std::vector<Token>(tokens.begin() + 2, tokens.end()), &exprType);

        Statement* statement = new Statement{ StatementType::VARIABLE };
        statement->as.variable = Variable{
            tokens[1].value,
            ParseExpression(file, line, expression)
        };

        statements.push_back(statement);
    }
    else if (tokens.size() >= 2 && tokens[0].type == TokenType::VARIABLE && tokens[1].type == TokenType::ASSIGNMENT)
    {
        if (tokens.size() == 2)
            throw Error(file, line, "expected expression after assignment operator");

        std::vector<Value> expression = TokensToExpression(std::vector<Token>(tokens.begin() + 2, tokens.end()));

        Statement* statement = new Statement{ StatementType::ASSIGNMENT };
        statement->as.assignment = Assignment{
            tokens[0].value,
            ParseExpression(file, line, expression)
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
        std::vector<Value> condition = TokensToExpression(
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

        Statement* statement = new Statement{ StatementType::CONDITIONAL };
        statement->as.conditional = Conditional{
            ParseExpression(file, line, condition),
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

        if (statements.size() == 0 || statements.back()->type != StatementType::CONDITIONAL ||
            statements.back()->as.conditional.condition == nullptr)
            throw Error(file, line, "expected if or else if statement before else if statement");

        // else if statement condition

        std::size_t closeBracketIndex = 3;
        AdvanceCloseBracketIndex(file, line, tokens, TokenType::OPEN_BRACKET, TokenType::CLOSE_BRACKET, closeBracketIndex);

        VariableType exprType;
        std::vector<Value> condition = TokensToExpression(
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

        statements.back()->as.conditional.chains.push_back(
            Conditional{ ParseExpression(file, line, condition), scope }
        );
    }
    else if (tokens.size() >= 1 && tokens[0].type == TokenType::ELSE)
    {
        if (tokens.size() == 2 || tokens[2].type != TokenType::OPEN_CURLY)
            throw Error(file, line, "expected open brace after 'else' keyword");

        if (statements.size() == 0 || statements.back()->type != StatementType::CONDITIONAL ||
            statements.back()->as.conditional.condition == nullptr)
            throw Error(file, line, "expected if or else if statement before else statement");

        // else statement body

        Scope scope;
        std::vector<std::vector<Token> > code = SplitCode(std::vector<Token>(tokens.begin() + 2, tokens.end() - 1));
        for (const std::vector<Token>& tokens : code)
            Parser(scope.statements, tokens);

        CheckFunctionDefinition(scope, file, line, "else statements cannot contain functions");

        statements.back()->as.conditional.chains.push_back(Conditional{ nullptr, scope });
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

        Statement* statement = new Statement{ StatementType::FUNCTION_DEF };
        statement->as.functionDef.name = tokens[1].value;

        // function parameters

        int closeBracketIndex = 3;
        for (; closeBracketIndex < tokens.size(); closeBracketIndex += 2)
        {
            if (tokens[closeBracketIndex].type != TokenType::VARIABLE)
                throw Error(file, line, "expected variable names as function parameters");

            statement->as.functionDef.parameters.push_back(tokens[closeBracketIndex].value);

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
            statement->as.functionDef.body.statements,
            std::vector<Token>(tokens.begin() + closeBracketIndex + 2, tokens.end() - 1)
        );

        CheckFunctionDefinition(statement->as.functionDef.body, file, line, "functions cannot contain functions");

        statements.push_back(statement);
    }
    else if (tokens.size() >= 2 && tokens[0].type == TokenType::VARIABLE && tokens[1].type == TokenType::OPEN_BRACKET)
    {
        if (tokens.size() == 2 || tokens.size() == 3 || tokens[2].type != TokenType::CLOSE_BRACKET)
            throw Error(file, line, "missing closing bracket");

        // function parameters

        Statement* statement = new Statement{ StatementType::FUNCTION_CALL };
        statement->as.functionCall.name = tokens[0].value;

        std::vector<Token> tokenParam;
        for (int a = 2, openBracketIndex = 0; a < tokens.size(); ++a)
        {
            if (tokens[a].type == TokenType::OPEN_BRACKET)
                openBracketIndex++;
            else if (tokens[a].type == TokenType::CLOSE_BRACKET)
                openBracketIndex--;

            if (tokens[a].type == TokenType::CLOSE_BRACKET && openBracketIndex == -1)
            {
                if (a < tokens.size() - 1)
                    throw Error(file, line, "unexpected tokens after function call; each statement must be on it's own line");

                std::vector<Value> paramExpr = TokensToExpression(tokenParam);
                statement->as.functionCall.parameters.push_back(ParseExpression(file, line, paramExpr));

                tokenParam.clear();
                continue;
            }
            
            if (tokens[a].type == TokenType::COMMA && openBracketIndex == 0)
            {
                std::vector<Value> paramExpr = TokensToExpression(tokenParam);
                statement->as.functionCall.parameters.push_back(ParseExpression(file, line, paramExpr));

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

        Statement* statement = new Statement{ StatementType::WHILE_LOOP };

        // while loop condition

        std::size_t closeBracketIndex = 2;
        AdvanceCloseBracketIndex(file, line, tokens, TokenType::OPEN_BRACKET, TokenType::CLOSE_BRACKET, closeBracketIndex);

        if (closeBracketIndex == tokens.size() - 1 || tokens[closeBracketIndex].type != TokenType::OPEN_CURLY)
            throw Error(file, line, "expected open curly bracket for while loop body");

        VariableType conditionType;
        std::vector<Value> condition = TokensToExpression(
            std::vector<Token>(tokens.begin() + 2, tokens.begin() + closeBracketIndex),
            &conditionType
        );

        if (conditionType != VariableType::BOOL)
            throw Error(file, line, "while loop condition can't be a " + VarTypeToStr(conditionType) + "; it must be a boolean value");

        // while loop body

        statement->as.whileLoop.condition = ParseExpression(file, line, condition);
        Parser(
            statement->as.whileLoop.body.statements,
            std::vector<Token>(tokens.begin() + closeBracketIndex + 1, tokens.end() - 1)
        );

        CheckFunctionDefinition(statement->as.whileLoop.body, file, line, "while loops cannot contain functions");

        statements.push_back(statement);
    }
    else if (tokens.size() >= 1 && tokens[0].type == TokenType::FOR)
    {
        //

    }
    else if (tokens.size() >= 2 && tokens[0].type == TokenType::VARIABLE && tokens[1].type == TokenType::OPEN_SQUARE)
    {
        //

    }
    else
    {
        throw Error(file, line, "invalid syntax");
    }
}