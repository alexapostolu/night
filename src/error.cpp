#include "../include/error.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <regex>
#include <string>

Error::Error(
	const std::string& debug_file, const int debug_line,
	const std::string& _type,
	const std::string& _file, const int _line,
	const std::string& _desc,
	const std::string& _note, const std::string& _link
)
	: type(_type), file(_file), line(_line), desc(_desc)
	, note(_note), link(_link)
{
#ifdef _DEBUG
	std::cout << debug_file << '\n' << debug_line << "\n\n";
#endif

	std::ifstream code(file);
	assert(code.is_open() && "shit"); // I blame the user for this one

	for (int a = 0; a < line; ++a)
		getline(code, code_line);

	for (int a = file.length() - 1; a >= 0; --a)
	{
		if (file[a] == '\\' || file[a] == '/')
		{
			file = file.substr(a + 1);
			break;
		}
	}
}

std::string Error::what() const
{
	std::stringstream output;

	output << b_RED << "[ " << error_type << " ] -> " << u_RED << type << '\n';
	output << WHITE << file << " (" << line << ")\n\n";

	output << WHITE << desc << "\n\n\n";

	output << "  " << b_WHITE << "| " << line << ":   " << WHITE << code_line << "\n\n\n";

	output << WHITE << note << '\n';
	if (link != "")
		output << WHITE << "\nfor more information, please visit:\n" << CYAN << "https://github.io/night/" << link << '\n';

	output << RESET;
	return output.str();
}

std::string Error::UnexpectedError(const std::exception& e)
{
	return std::string("Uh oh! We've come across an unexpected error:\n\n    ") + e.what() + "\n\n" +
		   "Please submit an issue on the GitHub page:\ngithub.com/dynamicsquid/night\n";
}

CompileError::CompileError(
	const std::string& debug_file, const int debug_line,
	const std::string& _type,
	const std::string& _file, const int _line,
	const std::string& _desc,
	const std::string& _note, const std::string& _link
)
	: Error(debug_file, debug_line, _type, _file, _line, _desc, _note, _link)
{
	error_type = "compile error";
}

const std::string CompileError::invalid_syntax     = "invalid syntax";
const std::string CompileError::invalid_grammar = "invalid grammar";
const std::string CompileError::invalid_definition = "invalid definition";
const std::string CompileError::invalid_type       = "invalid type";

RuntimeError::RuntimeError(
	const std::string& debug_file, const int debug_line,
	const std::string& _type,
	const std::string& _file, const int _line,
	const std::string& _desc,
	const std::string& _note, const std::string& _link
)
	: Error(debug_file, debug_line, _type, _file, _line, _desc, _note, _link)
{
	error_type = "runtime error";
}

const std::string RuntimeError::invalid_expression = "invalid expression";
const std::string RuntimeError::out_of_range       = "out of range";
const std::string RuntimeError::invalid_type       = "type mismatch";

FrontEndError::FrontEndError(const std::string& _msg)
{
	std::cout << "[ error ] - " << _msg << '\n';
}

std::string FrontEndError::what() const
{
	return "";
}