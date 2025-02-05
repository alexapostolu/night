#include "error.hpp"

#include <iostream>
#include <source_location>
#include <fstream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <unistd.h>
#endif

#ifdef _WIN32
int const RESET = 7;
int const RED = FOREGROUND_RED;
int const GREEN = 10;
int const BLUE = 9;
void set_text_colour(int colour)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, colour);
}
#else
int const RESET = 0;
int const RED = 31;
int const GREEN = 32;
int const BLUE = 34;
void set_text_colour(int colour)
{
	std::cout << "\033[" << colour << "m";
}
#endif

night::error::error()
	: debug_flag(false), has_minor_errors_(false) {}

night::error& night::error::get()
{
	static error instance;
	return instance;
}

void night::error::what() const
{
	for (auto const& err : errors)
	{
		std::string s = err.message;

		size_t pos = 0;
		while ((pos = s.find("integer", pos)) != std::string::npos) {
			s.replace(pos, strlen("integer"), "\033[36minteger\033[0m");
			pos += strlen("\033[36minteger\033[0m");
		}
		pos = 0;
		while ((pos = s.find("float", pos)) != std::string::npos) {
			s.replace(pos, strlen("float"), "\033[36mfloat\033[0m");
			pos += strlen("\033[36mfloat\033[0m");
		}
		pos = 0;
		while ((pos = s.find("character", pos)) != std::string::npos) {
			s.replace(pos, strlen("character"), "\033[36mcharacter\033[0m");
			pos += strlen("\033[36mcharacter\033[0m");
		}


		set_text_colour(RED);

		std::cout << "[ ";

		switch (err.type)
		{
		case ErrorType::Warning:
			std::cout << "warning";
			break;
		case ErrorType::Minor:
			std::cout << "minor error";
			break;
		case ErrorType::FatalCompile:
			std::cout << "fatal compile error";
			break;
		case ErrorType::FatalRuntime:
			std::cout << "fatal runtime error";
			break;
		}

		std::cout << " ]\n";

		set_text_colour(RESET);

		std::cout << err.location.file << " (" << std::to_string(err.location.line) << ":" << std::to_string(err.location.col) << ")\n";

		if (error::get().debug_flag)
			std::cout << err.source_location.file_name() << " " << std::to_string(err.source_location.line()) << '\n';

		std::cout << '\n' << s << "\n\n";

		std::string error_line;
		std::ifstream error_file(err.location.file);
		for (int i = 0; i < err.location.line; ++i)
			std::getline(error_file, error_line);

		std::cout << "    " << error_line << "\n";

		for (int i = 0; i < err.location.col; ++i)
			std::cout << ' ';
		set_text_colour(GREEN);
		std::cout << "    ^\n\n";
		set_text_colour(RESET);
	}
}

void night::error::create_warning(
	std::string const& msg,
	Location const& loc,
	std::source_location const& s_loc) noexcept
{
	error::get().errors.emplace_back(ErrorType::Warning, loc, s_loc, msg);
}

void night::error::create_minor_error(
	std::string const& msg,
	Location const& loc,
	std::source_location const& s_loc) noexcept
{
	error::get().errors.emplace_back(ErrorType::Minor, loc, s_loc, msg);
	has_minor_errors_ = true;
}

night::error const& night::error::create_fatal_error(
	std::string const& msg,
	Location const& loc,
	std::source_location const& s_loc) noexcept
{
	error::get().errors.emplace_back(ErrorType::FatalCompile, loc, s_loc, msg);
	return error::get();
}

night::error const& night::error::create_runtime_error(
	std::string const& msg,
	std::source_location const& s_loc) noexcept
{
	error::get().errors.emplace_back(ErrorType::FatalRuntime, Location{}, s_loc, msg);
	return error::get();
}

bool night::error::has_minor_errors() const
{
	return has_minor_errors_;
}