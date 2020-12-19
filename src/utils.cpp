#include "../include/utils.hpp"
#include "../include/parser.hpp"
#include "../include/token.hpp"
#include "../include/error.hpp"

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

namespace night {

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
	case VariableType::STR:
		return "string";
	case VariableType::STR_ARR:
		return "string array";
	case VariableType::EMPTY_ARR:
		return "array";
	default:
		assert_rtn(false && "variable type is missing", std::string());
	}
}

} // namespace night

std::vector<VariableType> TypeCheckExpression(
	const std::string& file,
	const int          line,

	const std::shared_ptr<Expression>& node,

	const std::vector<CheckVariable>& variables,
	const std::vector<CheckFunction>& functions,
	const std::vector<CheckClass>& classes,

	const std::vector<CheckVariable>& parameters
);
std::shared_ptr<Expression> ParseTokenExpression(const std::vector<Token>& tokens, const std::size_t start,
	const std::size_t end, const std::vector<CheckVariable>& variables, const std::vector<CheckFunction>& functions,
	const std::vector<CheckClass>& classes, const std::vector<CheckVariable>& parameters, std::vector<VariableType>* types)
{
	const std::vector<Value> values = TokensToValues(
		std::vector<Token>(tokens.begin() + start, tokens.begin() + end)
	);

	const std::shared_ptr<Expression> expression = ValuesToExpression(
		tokens[0].file, tokens[0].line, values
	);

	const std::vector<VariableType> exprTypes = TypeCheckExpression(
		tokens[0].file, tokens[0].line, expression, variables, functions, classes, parameters
	);
	
	if (types != nullptr)
		*types = exprTypes;

	return expression;
}

std::shared_ptr<Expression> ExtractCondition(const std::vector<Token>& tokens, std::size_t& closeBracketIndex,
	const std::vector<CheckVariable>& variables, const std::vector<CheckFunction>& functions,
	const std::vector<CheckClass>& classes, const std::vector<CheckVariable>& parameters, const std::string& stmt)
{
	const std::size_t start = closeBracketIndex;

	AdvanceToCloseBracket(tokens, TokenType::OPEN_BRACKET, TokenType::CLOSE_BRACKET, closeBracketIndex);
	if (closeBracketIndex >= tokens.size())
		throw Error(tokens[0].file, tokens[0].line, "missing closing bracket for " + stmt);

	std::vector<VariableType> types;
	const std::shared_ptr<Expression> conditionExpr = ParseTokenExpression(
		tokens, start, closeBracketIndex, variables, functions, classes, parameters, &types
	);

	if (!night::find_type(types, VariableType::BOOL))
		throw Error(tokens[0].file, tokens[0].line, stmt + " condition must evaluate to a boolean value");

	return conditionExpr;
}

std::vector<Statement> ExtractBody(const std::vector<Token>& tokens, const std::size_t closeBracketIndex,
	std::vector<CheckVariable>& variables, const std::string& stmt)
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