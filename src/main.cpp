#include "../include/lexer.hpp"
#include "../include/parser.hpp"
#include "../include/interpreter.hpp"
#include "../include/utils.hpp"
#include "../include/error.hpp"
#include "../include/token.hpp"

#include <fstream>
#include <iostream>
#include <exception>
#include <string>
#include <vector>

std::vector<Token> OpenFile(const std::string& file)
{
	std::ifstream sourceFile(file);
	if (!sourceFile.is_open())
		throw Error("file '" + file + "' could not be opened");

	std::vector<Token> tokens;
	std::string fileLine;
	for (int line = 1; getline(sourceFile, fileLine); ++line)
	{
		std::vector<Token> fileTokens = Lexer(file, line, fileLine);
		if (fileTokens.size() >= 1 && fileTokens[0].type == TokenType::IMPORT)
		{
			if (fileTokens.size() == 1 || fileTokens[1].type != TokenType::STR_VAL)
				throw Error(file, line, "expected file name (string) after '" + fileTokens[0].data + "' statement");
			if (fileTokens.size() > 2)
				throw Error(file, line, fileTokens[0].data + " statement must be on it's own line");

			const std::vector<Token> importTokens = OpenFile(
				fileTokens[0].data == "import"
					? "../pkgs/" + fileTokens[1].data + ".night"
					: fileTokens[1].data + ".night"
			);

			fileTokens.erase(fileTokens.begin(), fileTokens.begin() + 2);
			fileTokens.insert(fileTokens.begin(), importTokens.begin(), importTokens.end());
		}

		tokens.insert(tokens.end(), fileTokens.begin(), fileTokens.end());
	}

	sourceFile.close();

	return tokens;
}

void BackEnd(const std::string& file)
{
	const std::vector<std::vector<Token> > code = SplitCode(OpenFile(file));
	std::vector<Statement> statements;
	for (const std::vector<Token>& tokens : code)
	{
		assert(!tokens.empty() && "tokens shouldn't be empty");
		Parser(statements, tokens);
	}

	Interpreter(statements);
}

void Frontend(const int argc)
{
	if (argc != 2)
		throw Error("invalid command line arguments; only pass in the file name as an argument");
}

int main(int argc, char* argv[])
{
	try {
		Frontend(argc);
		BackEnd(argv[1]);
	}
	catch (const Error& e) {
		std::clog << e.what() << '\n';
		return 1;
	}
	catch (const std::exception& e) {
		std::clog << "shit\n" << e.what() << '\n';
		return 1;
	}
}