#include "../include/front-end/front-end.hpp"
#include "../include/lexer.hpp"
#include "../include/parser.hpp"
#include "../include/interpreter.hpp"
#include "../include/utils.hpp"
#include "../include/error.hpp"
#include "../include/token.hpp"

#include <fstream>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

std::vector<Token> OpenFile(const std::string& file)
{
	std::ifstream sourceFile(file);
	if (!sourceFile.is_open())
		throw FrontError("file '" + file + "' could not be opened");

	std::vector<Token> tokens;
	std::string fileLine;
	for (int line = 1; getline(sourceFile, fileLine); ++line)
	{
		std::vector<Token> fileTokens = Lexer(file, line, fileLine);
		if (fileTokens.size() >= 1 && fileTokens[0].type == TokenType::IMPORT)
		{
			if (fileTokens.size() == 1 || fileTokens[1].type != TokenType::STR)
				throw BackError(file, line, "expected file name (string) after '" + fileTokens[0].data + "' statement");
			if (fileTokens.size() > 2)
				throw BackError(file, line, fileTokens[0].data + " statement must be on it's own line");

			const std::vector<Token> importTokens = OpenFile(
				fileTokens[0].data == "import"
					? "../pkgs/" + fileTokens[1].data + ".night"
					: fileTokens[1].data + ".night"
			);

			fileTokens.erase(std::begin(fileTokens), std::begin(fileTokens) + 2);

			fileTokens.insert(std::end(fileTokens),
				std::begin(importTokens), std::end(importTokens));
		}

		tokens.insert(std::end(tokens),
			std::begin(fileTokens), std::end(fileTokens));
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
		Parser parse(statements, tokens);
	}

	Interpreter interpret(statements);
}

int main(int argc, char* argv[])
{
	try {
		FrontEnd(argc);
		BackEnd(argv[1]);

		return 0;
	}
	catch (const Error& e) {
		std::cout << e.what() << '\n';
	}
	catch (const std::exception& e) {
		std::cout << "Uh oh! We've come across an unexpected error:\n\n\t" << e.what() << "\n\n"
				  << "Please submit an issue on the GitHub page:\ngithub.com/dynamicsquid/night\n";
	}

	return 1;
}