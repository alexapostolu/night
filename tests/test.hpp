#pragma once

#include "bytecode.hpp"

#include <string>

class Test
{
public:
	Test(Test const&) = delete;
	void operator=(Test const&) = delete;

public:
	static Test& get();

	template<typename T> void start(T t = T());

	void expect(BytecodeType t)
	{
		std::clog << " . " << bytecode_to_str(t) << " == " << bytecode_to_str(lexer.curr().type);

		if (!(cond))
			std::clog << " . . assertion failed\n         " << (msg) << "\n";
		else
			std::clog << " . . assertion passed\n";
	}

	template<typename... Args>
	void expect(T t, Args... args) // recursive variadic function
	{
		std::cout << t << std::endl;

		func(args...);
	}


	void test(std::string const& test_name, std::string const& test_code);
	void expect();

private:
	Test();


};