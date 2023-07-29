#pragma once

#include "bytecode.hpp"

#include <iostream>
#include <string>

class Test
{
public:
	Test(Test const&) = delete;
	void operator=(Test const&) = delete;

public:
	static Test& get();

	void test(std::string const& test_name, std::string const& test_code);
	void expect();

private:
	Test();


};