#include "../include/utils.h"
#include "../include/parser.h"
#include "../include/token.h"
#include "../include/error.h"

#include <string>
#include <vector>

std::vector<std::vector<Token> > SplitCode(const std::vector<Token>& tokens)
{
    std::vector<std::vector<Token> > code;
    for (std::size_t a = 0, openCurlyCount = 0; a < tokens.size(); ++a)
    {
        if (a == 0)
            code.push_back(std::vector<Token>());

        if (tokens[a].type == TokenType::EOL && openCurlyCount == 0)
        {
            if (a < tokens.size() - 1)
                code.push_back(std::vector<Token>());

            continue;
        }

        if (tokens[a].type == TokenType::OPEN_CURLY)
            openCurlyCount++;
        else if (tokens[a].type == TokenType::CLOSE_CURLY)
            openCurlyCount--;

        code.back().push_back(tokens[a]);
    }

    return code;
}

std::string VarTypeToStr(const VariableType& type)
{
    switch (type)
    {
    case VariableType::BOOL:
        return "boolean";
    case VariableType::BOOL_ARR:
        return "boolean array";
    case VariableType::NUM:
        return "number";
    case VariableType::NUM_ARR:
        return "number array";
    case VariableType::STRING:
        return "string";
    case VariableType::STRING_ARR:
        return "string array";
    case VariableType::COORD:
        return "coordinate";
    default:
        assert_rtn(false && "variable type is missing", "");
    }
}

std::string DefaultValue(const ValueType& type)
{
    switch (type)
    {
    case ValueType::BOOL:
        return "false";
    case ValueType::NUM:
        return "0";
    case ValueType::STRING:
        return "";
    default:
        assert_rtn(false && "missing value", "");
    }
}

VariableType ValueTypeToVarType(const ValueType& type)
{
    switch (type)
    {
    case ValueType::BOOL:
        return VariableType::BOOL;
    case ValueType::BOOL_ARR:
        return VariableType::BOOL_ARR;
    case ValueType::NUM:
        return VariableType::NUM;
    case ValueType::NUM_ARR:
        return VariableType::NUM_ARR;
    case ValueType::STRING:
        return VariableType::STRING;
    case ValueType::STRING_ARR:
        return VariableType::STRING_ARR;
    default:
        std::clog << (int)type << '\n';
        assert_rtn(false && "value type missing", VariableType());
    }
}

ValueType VarTypeToValueType(const VariableType& type)
{
    switch (type)
    {
    case VariableType::BOOL:
        return ValueType::BOOL;
    case VariableType::BOOL_ARR:
        return ValueType::BOOL_ARR;
    case VariableType::NUM:
        return ValueType::NUM;
    case VariableType::NUM_ARR:
        return ValueType::NUM_ARR;
    case VariableType::STRING:
        return ValueType::STRING;
    case VariableType::STRING_ARR:
        return ValueType::STRING_ARR;
    default:
        assert_rtn(false && "value type missing", ValueType());
    }
}

VariableType VarTypeToArrType(const VariableType& type)
{
    switch (type)
    {
    case VariableType::BOOL:
        return VariableType::BOOL_ARR;
    case VariableType::NUM:
        return VariableType::NUM_ARR;
    case VariableType::STRING:
        return VariableType::STRING_ARR;
    case VariableType::BOOL_ARR:
        return VariableType::BOOL_ARR;
    case VariableType::NUM_ARR:
        return VariableType::NUM_ARR;
    case VariableType::STRING_ARR:
        return VariableType::STRING_ARR;
    default:
        assert_rtn(false && "value type missing", VariableType());
    }
}

std::shared_ptr<Expression> ExtractExpression(const std::vector<Token>& tokens, const std::size_t start, const std::size_t end,
    std::vector<Variable>& variables, std::vector<FunctionDef>& functions, const VariableType* expectedType)
{
    const std::vector<Value> values = TokensToValues(
        std::vector<Token>(tokens.begin() + start, tokens.begin() + end),
        variables, functions
    );

    const std::shared_ptr<Expression> expression = ParseValues(tokens[0].file, tokens[0].line, values, variables, functions);
    const VariableType type = TypeCheckExpression(tokens[0].file, tokens[0].line, expression, variables, functions);

    return expectedType != nullptr && type != *expectedType ? nullptr : expression;
}

std::shared_ptr<Expression> ExtractCondition(const std::vector<Token>& tokens, std::size_t& closeBracketIndex,
    std::vector<Variable>& variables, std::vector<FunctionDef>& functions, const std::string& stmt)
{
    const std::size_t start = closeBracketIndex;

    AdvanceCloseBracketIndex(tokens[0].file, tokens[0].line, tokens, TokenType::OPEN_BRACKET, TokenType::CLOSE_BRACKET, closeBracketIndex);
    if (closeBracketIndex >= tokens.size())
        throw Error(tokens[0].file, tokens[0].line, "missing closing bracket for " + stmt);

    const VariableType type = VariableType::BOOL;
    std::shared_ptr<Expression> conditionExpr = ExtractExpression(tokens, start, closeBracketIndex, variables, functions, &type);

    if (conditionExpr == nullptr)
        throw Error(tokens[0].file, tokens[0].line, stmt + " condition evaluates to a '" + VarTypeToStr(type) + "'; conditions must evaluate to a boolean");

    return conditionExpr;
}

std::vector<Statement> ExtractBody(const std::vector<Token>& tokens, std::size_t closeBracketIndex,
    std::vector<Variable>& variables, const std::string& stmt)
{
    const std::vector<std::vector<Token> > splitTokens = tokens[closeBracketIndex + 1].type == TokenType::OPEN_CURLY
        ? SplitCode(std::vector<Token>(tokens.begin() + closeBracketIndex + 2, tokens.end() - 1))
        : SplitCode(std::vector<Token>(tokens.begin() + closeBracketIndex + 1, tokens.end()));

    const std::size_t variableSize = variables.size();

    std::vector<Statement> body;
    for (const std::vector<Token>& toks : splitTokens)
    {
        Parser(body, toks);

        if (body.back().type == StatementType::FUNCTION_DEF)
            throw Error(toks[0].file, toks[0].line, "function definition found in " + stmt + "; " + stmt + "s cannot contain function definitions");
    }

    variables.erase(variables.begin() + variableSize, variables.end());

    return body;
}