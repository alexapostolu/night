#pragma once

#include <fstream>
#include <iostream>
#include <string>

// code errors
int error(const std::string&& error)
{
	std::cout << "Error - " << error << '\n';
	return 1;
}

// testing purposes
void logEvent(const std::string& event)
{
	std::cout << event;
}

void logEvent(const std::string&& event)
{
	std::cout << event;
}

// stores user output
std::string storeOutput(const std::string& outputStr)
{
	static std::string output = "";
	output += outputStr;

	return output;
}

// prints it out in the end
void printOutput(const std::string& output)
{
	std::cout << output;
}