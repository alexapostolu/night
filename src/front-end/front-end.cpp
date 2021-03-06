#include "../../include/front-end/front-end.hpp"
#include "../../include/back-end/interpreter.hpp"
#include "../../include/back-end/parser.hpp"
#include "../../include/back-end/lexer.hpp"
#include "../../include/back-end/token.hpp"
#include "../../include/back-end/utils.hpp"
#include "../../include/error.hpp"

#include <fstream>
#include <string>
#include <vector>

void FrontEnd(int argc, char** argv)
{
	if (argc != 2)
		throw FrontEndError("invalid command line arguments; only pass in the file name as an argument");

	if (std::string(argv[1]) == "--version")
	{
		std::cout << "night v1.0.0";
		return;
	}

	const std::vector<std::vector<Token> > code = SplitCode(OpenFile(argv[1]));
	const std::shared_ptr<Scope> global_scope = std::make_shared<Scope>(nullptr);

	for (const std::vector<Token>& tokens : code)
	{
		assert(!tokens.empty());
		Parser(global_scope, tokens);
	}

	std::shared_ptr<NightScope> night_global = std::make_shared<NightScope>(nullptr);
	Interpreter(night_global, global_scope->statements);
}

std::vector<Token> OpenFile(const std::string& file)
{
	std::ifstream source_file(file);
	if (!source_file.is_open())
		throw FrontEndError("file '" + file + "' could not be opened");

	std::vector<Token> tokens;
	std::string file_line;
	for (int line = 1; getline(source_file, file_line); ++line)
	{
		std::vector<Token> file_tokens = Lexer(file, line, file_line);
		if (file_tokens.size() >= 1 && file_tokens[0].type == TokenType::IMPORT)
		{
			if (file_tokens.size() == 1 || file_tokens[1].type != TokenType::STR)
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "expected file name after '" + file_tokens[0].data + "' statement");
			if (file_tokens.size() > 3) // EOF
				throw CompileError(__FILE__, __LINE__, CompileError::invalid_syntax, file, line, "'" + file_tokens[0].data + "' must be on its own line");

			if (file_tokens[0].data == "import" && file_tokens[1].data == "python")
				throw FrontEndError("module 'python' could not be imported; only good languages are allowed here");

			const std::vector<Token> import_tokens = OpenFile(
				(file_tokens[0].data == "import" ? "../../../pkgs/" : "") +
				(file_tokens[1].data + ".night")
			);

			file_tokens.erase(file_tokens.begin(), file_tokens.begin() + 3); // EOF
			file_tokens.insert(file_tokens.begin(), import_tokens.begin(), import_tokens.end());
		}

		tokens.insert(tokens.end(), file_tokens.begin(), file_tokens.end());
	}

	return tokens;
}