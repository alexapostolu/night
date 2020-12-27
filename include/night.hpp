#pragma once

#include "token.hpp"

#include <string>
#include <vector>

struct NightData
{
    VariableType type;

    std::string data;
    std::vector<NightData> extras;
};

struct NightVariable
{
    std::string name;
    NightData value;
};

struct NightFunction
{
    std::string name;
    std::vector<std::string> parameters;
    std::vector<Statement> body;
};

struct NightClass
{
    std::string name;
    std::vector<NightVariable> variables;
    std::vector<NightVariable> methods;
};

// displays standard output
void NightPrint(const NightData& value);