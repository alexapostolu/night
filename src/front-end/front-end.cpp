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

void FrontEnd(const int argc, const char* argv[])
{
	if (argc != 2)
		throw FrontEndError("invalid command line arguments; only pass in the file name as an argument");

	const std::vector<std::vector<Token> > code = SplitCode(OpenFile(argv[1]));
	std::shared_ptr<Scope> global_scope = std::make_shared<Scope>(Scope{ nullptr });
	for (const std::vector<Token>& tokens : code)
	{
		assert(!tokens.empty() && "tokens shouldn't be empty");
		Parser parse(global_scope, tokens);
	}

	NightScope night_global{ nullptr };
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
				throw BackError(file, line, "expected file name after '" + file_tokens[0].data + "' statement");
			if (file_tokens.size() > 2)
				throw BackError(file, line, file_tokens[0].data + " statement must be on it's own line");

			const std::vector<Token> import_tokens = OpenFile(
				(file_tokens[0].data == "import" ? "../../../pkgs/" : "") +
				(file_tokens[1].data + ".night")
			);

			file_tokens.erase(file_tokens.begin(), file_tokens.begin() + 2);
			file_tokens.insert(file_tokens.end(), import_tokens.begin(), import_tokens.end());
		}

		tokens.insert(tokens.end(), file_tokens.begin(), file_tokens.end());
	}

	return tokens;
}