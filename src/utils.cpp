#include "../include/utils.h"
#include "../include/parser.h"
#include "../include/token.h"
#include "../include/error.h"

#include <memory>
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

namespace night
{

std::string ttos(const ValueType& type)
{
    switch (type)
    {
    case ValueType::BOOL:
        return "bool";
    case ValueType::BOOL_ARR:
        return "bool array";
    case ValueType::NUM:
        return "number";
    case ValueType::NUM_ARR:
        return "number array";
    case ValueType::STRING:
        return "string";
    case ValueType::STRING_ARR:
        return "string array";
    case ValueType::EMPTY_ARRAY:
        return "array";
    default:
        assert_rtn(false && "value type missing", std::string());
    }
}

std::string ttos(const VariableType& type)
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
    case VariableType::EMPTY_ARR:
        return "array";
    default:
        assert_rtn(false && "variable type is missing", std::string());
    }
}

} // namespace night

std::shared_ptr<Expression> ExtractExpression(const std::vector<Token>& tokens, const std::size_t start, const std::size_t end,
    const std::vector<Variable>& variables, const std::vector<FunctionDef>& functions, VariableType* type)
{
    const std::vector<Value> values = TokensToValues(
        std::vector<Token>(tokens.begin() + start, tokens.begin() + end),
        variables, functions
    );

    const std::shared_ptr<Expression> expression = ParseValues(tokens[0].file, tokens[0].line, values, variables, functions);
    const ValueType exprType = TypeCheckExpression(tokens[0].file, tokens[0].line, expression, variables, functions).type;
    
    if (type != nullptr)
    {
        switch (exprType)
        {
        case ValueType::BOOL:
            *type = VariableType::BOOL;
            break;
        case ValueType::BOOL_ARR:
            *type = VariableType::BOOL_ARR;
            break;
        case ValueType::NUM:
            *type = VariableType::NUM;
            break;
        case ValueType::NUM_ARR:
            *type = VariableType::NUM_ARR;
            break;
        case ValueType::STRING:
            *type = VariableType::STRING;
            break;
        case ValueType::STRING_ARR:
            *type = VariableType::STRING_ARR;
            break;
        case ValueType::EMPTY_ARRAY:
            *type = VariableType::EMPTY_ARR;
            break;
        default:
            assert(false && "value type missing");
        }
    }

    return expression;
}

std::shared_ptr<Expression> ExtractCondition(const std::vector<Token>& tokens, std::size_t& closeBracketIndex,
    const std::vector<Variable>& variables, const std::vector<FunctionDef>& functions, const std::string& stmt)
{
    const std::size_t start = closeBracketIndex;

    AdvanceCloseBracketIndex(tokens, TokenType::OPEN_BRACKET, TokenType::CLOSE_BRACKET, closeBracketIndex);
    if (closeBracketIndex >= tokens.size())
        throw Error(tokens[0].file, tokens[0].line, "missing closing bracket for " + stmt);

    VariableType type;
    const std::shared_ptr<Expression> conditionExpr = ExtractExpression(tokens, start, closeBracketIndex, variables, functions, &type);

    if (type != VariableType::BOOL)
        throw Error(tokens[0].file, tokens[0].line, stmt + " condition evaluates to a '" + night::ttos(type) + "'; conditions must evaluate to a boolean");

    return conditionExpr;
}

std::vector<Statement> ExtractBody(const std::vector<Token>& tokens, const std::size_t closeBracketIndex,
    std::vector<Variable>& variables, const std::string& stmt, const bool inFunction)
{
    const std::vector<std::vector<Token> > splitTokens = tokens[closeBracketIndex + 1].type == TokenType::OPEN_CURLY
        ? SplitCode(std::vector<Token>(tokens.begin() + closeBracketIndex + 2, tokens.end() - 1))
        : SplitCode(std::vector<Token>(tokens.begin() + closeBracketIndex + 1, tokens.end()));

    const std::size_t variableSize = variables.size();

    std::vector<Statement> body;
    for (const std::vector<Token>& toks : splitTokens)
    {
        Parser(body, toks, inFunction);

        if (body.back().type == StatementType::FUNCTION_DEF)
            throw Error(toks[0].file, toks[0].line, "function definition found in " + stmt + "; " + stmt + "s cannot contain function definitions");
    }

    variables.erase(variables.begin() + variableSize, variables.end());

    return body;
}